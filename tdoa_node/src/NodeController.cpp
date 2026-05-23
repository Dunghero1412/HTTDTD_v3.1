// ============================================================================
// File: NodeController.cpp
// ============================================================================
#include "NodeController.hpp"
#include "TDOASolver.hpp"
#include <lgpio.h>
#include <iostream>
#include <sstream>
#include <chrono>
#include <thread>
#include <cstring>
#include <cmath>

// Hàm parse ID từ đối số dòng lệnh
NodeID parseNodeID(const std::string& str) {
    NodeID id;
    id.col = str[0] - '0'; // '1' -> 1 // chuyển ký tự thành số
    id.row = std::toupper(str[1]); // 'A' -> 'A' // chuyển ký tự thành chữ hoa
    return id;
}
std::string nodeIDString(const NodeID& id) {
    return std::to_string(id.col) + id.row;
}

NodeController::NodeController(const NodeID& id)
    : nodeID(id), state(IDLE), running(true), captureCount(0) {
    // Khởi tạo GPIO
    gpioHandle = lgGpiochipOpen(0); // /dev/gpiochip0
    if (gpioHandle < 0) {
        std::cerr << "Failed to open GPIO chip\n";
        exit(1);
    }
    // nhận output, thiết lập mức thấp ban đầu
    lgGpioClaimOutput(gpioHandle, 0, GPIO_MAIN_MOTOR, LG_LOW); 
    lgGpioClaimOutput(gpioHandle, 0, GPIO_FORCE_TRIGGER, LG_LOW);
    lgGpioClaimOutput(gpioHandle, 0, GPIO_RECEIVED_COMPLETE, LG_LOW);
    lgGpioClaimOutput(gpioHandle, 0, GPIO_FORCE_RESET, LG_LOW);
    lgGpioClaimOutput(gpioHandle, 0, GPIO_HARD_RESET, LG_LOW);
    // nhận input, thiết lập mức thấp ban đầu
    lgGpioClaimInput(gpioHandle, 0, GPIO_LOAD_DATA);

    // Khởi tạo SPI
    if (!spi.init(SPI_CHANNEL, SPI_SPEED)) {
        std::cerr << "SPI init failed\n"; // log lỗi , dừng chương trình
        exit(1);
    }

    // Khởi tạo LoRa
    int sf = SF_BY_COL[nodeID.col - 1]; // col index 0..4
    if (!lora.init(LORA_UART, LORA_BAUD, sf, FREQ_MHZ)) {
        std::cerr << "LoRa init failed\n"; // log lỗi , dừng chương trình
        exit(1);
    }
    lora.setReceiveCallback([this](const std::string& msg) { onLoRaReceived(msg); }); // callback khi nhận được message

    // Bắt đầu thread nhận LoRa
    loraThread = std::thread(&NodeController::loraRxTask, this);
}

NodeController::~NodeController() {
    running = false;
    if (loraThread.joinable()) loraThread.join();
    lgGpiochipClose(gpioHandle);
}

void NodeController::run() {
    while (running) {
        // Kiểm tra timeout trong ACTIVE_CAPTURE
        if (state == ACTIVE_CAPTURE) {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                               now - triggerStartTime).count();
            if (elapsed >= TIMEOUT_CAPTURE) {
                std::cout << "Timeout capture\n";
                finishCycle();
                continue;
            }

            // Đọc chân LOAD_DATA (GPIO 18)
            int ld = lgGpioRead(gpioHandle, GPIO_LOAD_DATA);
            if (ld == 1) {
                // STM32 báo sẵn sàng
                uint64_t timestamps[4];
                float temp;
                if (readTimestamps(timestamps, temp)) {
                    // Tính toán vị trí
                    auto [x, y] = TDOASolver::computePosition(timestamps, temp);
                    sendPosition(x, y);
                    pulseReceivedComplete();
                    captureCount++;
                    if (captureCount >= MAX_CAPTURES) {
                        std::cout << "Đủ 3 gói, kết thúc\n";
                        finishCycle();
                    }
                }
            }
        }

        // Xử lý lệnh trong hàng đợi
        std::string cmd;
        {
            std::lock_guard<std::mutex> lock(cmdMutex);
            if (!pendingCommand.empty()) {
                cmd = pendingCommand;
                pendingCommand.clear();
            }
        }
        if (!cmd.empty()) {
            processCommand(cmd);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void NodeController::onLoRaReceived(const std::string& msg) {
    std::lock_guard<std::mutex> lock(cmdMutex);
    pendingCommand = msg; // Chỉ xử lý lệnh mới nhất
}

void NodeController::processCommand(const std::string& cmd) {
    // Parse cú pháp: [<type>, <action>] hoặc [STATUS]
    // Ví dụ: "NODE_1A , UP", "MARKING , UP", "STATUS"
    std::istringstream iss(cmd);
    std::string type, action;
    std::getline(iss, type, ',');
    // Xóa dấu cách thừa
    type.erase(0, type.find_first_not_of(" "));
    type.erase(type.find_last_not_of(" ") + 1);

    std::getline(iss, action);
    action.erase(0, action.find_first_not_of(" "));
    action.erase(action.find_last_not_of(" ") + 1);

    if (type == "STATUS") {
        sendStatus();
        return;
    }

    bool isUp = (action == "UP");      // Kiểm tra hành động UP
    bool isDown = (action == "DOWN");  // Kiểm tra hành động DOWN
    bool isHR = (action == "HR");      // Kiểm tra hành động HR

    // Kiểm tra MARKING
    if (type == "MARKING") {
        if (isUp) {
            setMotor(true);
            state = MARKING; // chuyển trạng thái sang MARKING
            std::cout << "MARKING ON\n";
        } else if (isDown) {
            setMotor(false);
            state = IDLE; // chuyển trạng thái sang IDLE
            std::cout << "MARKING OFF\n";
        }
        return;
    }

    // Nếu đang MARKING, bỏ qua các lệnh khác
    if (state == MARKING) return;

    // Phân tích lệnh NODE_...
    if (type.substr(0,5) != "NODE_") return;
    std::string target = type.substr(5); // "1A", "1", "A"

    // Kiểm tra xem lệnh có áp dụng cho node này không
    bool apply = false;
    if (target.size() == 2 && std::isdigit(target[0]) && std::isalpha(target[1])) {
        // Cụ thể: NODE_1A
        NodeID targetID = parseNodeID(target);
        apply = (targetID.col == nodeID.col && targetID.row == nodeID.row);
    } else if (target.size() == 1 && std::isdigit(target[0])) {
        // Cả cột: NODE_1
        int col = target[0] - '0';
        apply = (col == nodeID.col);
    } else if (target.size() == 1 && std::isalpha(target[0])) {
        // Cả hàng: NODE_A
        char row = std::toupper(target[0]);
        apply = (row == nodeID.row);
    }

    if (!apply) return;

    if (isHR) {
        hardResetSTM32();
        return;
    }

    if (isUp) {
        if (state == IDLE) {
            setMotor(true);
            motorStartTime = std::chrono::steady_clock::now();
            state = MOTOR_ON_DELAY;
            std::cout << "Motor ON, chờ 10s\n";
        }
    } else if (isDown) {
        if (state == ACTIVE_CAPTURE || state == MOTOR_ON_DELAY) {
            std::cout << "Nhận lệnh DOWN, hủy\n";
            finishCycle();
        }
    }
}

void NodeController::setMotor(bool on) {
    lgGpioWrite(gpioHandle, GPIO_MAIN_MOTOR, on ? 1 : 0); // Bật/tắt motor
}

void NodeController::setTrigger(bool on) {
    lgGpioWrite(gpioHandle, GPIO_FORCE_TRIGGER, on ? 1 : 0); // Bật/tắt trigger
}

void NodeController::pulseReceivedComplete() {
    lgGpioWrite(gpioHandle, GPIO_RECEIVED_COMPLETE, 1); // Bật pulse Received Complete
    std::this_thread::sleep_for(std::chrono::milliseconds(RECEIVED_COMPLETE_PULSE));
    lgGpioWrite(gpioHandle, GPIO_RECEIVED_COMPLETE, 0); // Tắt pulse Received Complete
}

void NodeController::pulseForceReset() {
    lgGpioWrite(gpioHandle, GPIO_FORCE_RESET, 1); // Bật pulse Force Reset
    std::this_thread::sleep_for(std::chrono::milliseconds(FORCE_RESET_PULSE));
    lgGpioWrite(gpioHandle, GPIO_FORCE_RESET, 0); // Tắt pulse Force Reset
}

void NodeController::hardResetSTM32() {
    lgGpioWrite(gpioHandle, GPIO_HARD_RESET, 0); // Kéo NRST xuống , thucej hiện việc hard reset trên stm32
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    lgGpioWrite(gpioHandle, GPIO_HARD_RESET, 1); // Thả
}

bool NodeController::readTimestamps(uint64_t* timestamps, float& temperature) {
    // Đọc dữ liệu SPI, bao gồm 4 dòng timestamp và 1 dòng nhiệt độ
    auto lines = spi.readData(SPI_TIMEOUT);
    if (lines.size() < 4) return false;
    // Parse từng dòng
    for (int i = 0; i < 4; ++i) {
        // Định dạng: "A , 0xOVERFLOW, 0xTICK"
        std::istringstream iss(lines[i]);
        std::string ch, comma, ovStr, tickStr;
        iss >> ch >> comma >> ovStr >> comma >> tickStr;
        uint32_t ov = std::stoul(ovStr, nullptr, 16);
        uint32_t tick = std::stoul(tickStr, nullptr, 16);
        timestamps[i] = ((uint64_t)ov << 32) | tick;
    }
    // Nếu có dòng thứ 5 chứa nhiệt độ (tùy chọn)
    if (lines.size() >= 5 && lines[4].size() > 2 && lines[4][0] == 'T') {
        temperature = std::stof(lines[4].substr(2));
    } else {
        // Nếu không, yêu cầu thêm qua SPI (lệnh 0x01) – cần STM32 hỗ trợ
        temperature = spi.requestTemperature(); // mặc định 25 nếu không có
    }
    return true;
}

void NodeController::sendPosition(double x, double y) {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    std::string msg = std::to_string(now_c) + ", NODE_" + nodeIDString(nodeID)
                      + ", " + std::to_string(x) + ", " + std::to_string(y);
    lora.send(msg);
}

void NodeController::sendStatus() {
    int battery = lora.batteryPercent();
    std::string statusStr;
    switch (state) {
        case IDLE: statusStr = "DEACTIVATED"; break;
        case MOTOR_ON_DELAY: case ACTIVE_CAPTURE: statusStr = "ACTIVATED"; break;
        case MARKING: statusStr = "MARKING"; break;
    }
    float temp = 25.0f; // có thể đọc từ BME280 nếu có, tạm để 25
    std::string conn = lora.connectionInfo();
    std::string msg = "NODE" + nodeIDString(nodeID) + " , STATUS , "
                      + std::to_string(battery) + " , " + statusStr + " , "
                      + std::to_string(temp) + " , " + conn;
    lora.send(msg);
}

void NodeController::finishCycle() {
    setTrigger(false);
    pulseForceReset();
    setMotor(false);
    state = IDLE;
    captureCount = 0;
}

void NodeController::loraRxTask() {
    while (running) {
        lora.process();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}