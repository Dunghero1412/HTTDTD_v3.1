// ============================================================================
// File: NodeController.hpp
// Mô tả: Quản lý toàn bộ hoạt động của Node: trạng thái, GPIO, SPI, LoRa, tính toán.
// ============================================================================
#pragma once
#include "Config.hpp"
#include "LoRaModule.hpp"
#include "SPIMaster.hpp"
#include <thread>
#include <atomic>
#include <mutex>
#include <string>

class NodeController {
public:
    NodeController(const NodeID& id);
    ~NodeController();
    void run();                     // Vòng lặp chính

private:
    enum State {
        IDLE,
        MOTOR_ON_DELAY,            // Đang chờ 10s đầu
        ACTIVE_CAPTURE,            // Đã kích trigger, đang chờ timestamp
        MARKING                    // Chế độ đánh dấu
    };

    NodeID nodeID;
    State state;

    // GPIO handle (dùng lgpio)
    int gpioHandle;

    LoRaModule lora;
    SPIMaster spi;
    std::thread loraThread;
    std::atomic<bool> running;
    std::mutex cmdMutex;
    std::string pendingCommand;    // Lệnh đang xử lý

    // Thời điểm bắt đầu motor
    std::chrono::steady_clock::time_point motorStartTime;

    // Số gói timestamp đã nhận và gửi thành công
    int captureCount;
    // Thời điểm bắt đầu trigger (sau 10s)
    std::chrono::steady_clock::time_point triggerStartTime;

    // Hàm callback khi nhận lệnh LoRa
    void onLoRaReceived(const std::string& msg);

    // Xử lý lệnh (parse và thực thi)
    void processCommand(const std::string& cmd);

    // Các hàm điều khiển GPIO
    void setMotor(bool on);
    void setTrigger(bool on);
    void pulseReceivedComplete();
    void pulseForceReset();
    void hardResetSTM32();

    // Đọc dữ liệu timestamp từ SPI
    bool readTimestamps(uint64_t* timestamps, float& temperature);

    // Gửi kết quả về Controller
    void sendPosition(double x, double y);

    // Gửi STATUS
    void sendStatus();

    // Reset trạng thái sau khi hoàn thành hoặc timeout
    void finishCycle();

    // Thread nhận LoRa
    void loraRxTask();
};