// ============================================================================
// File: Config.hpp
// Mô tả: Các hằng số cấu hình phần cứng và hệ thống NODE.
// ============================================================================
#pragma once

#include <cstdint>
#include <string>

// ---------- GPIO Raspberry Pi (BCM) ----------
constexpr int GPIO_MAIN_MOTOR = 21;          // OUTPUT - Motor quay
constexpr int GPIO_FORCE_TRIGGER = 17;       // OUTPUT - Kết nối PB2 (TC) của STM32
constexpr int GPIO_RECEIVED_COMPLETE = 19;   // OUTPUT - Kết nối PB1 (RDC)
constexpr int GPIO_FORCE_RESET = 20;         // OUTPUT - Kết nối PB4 (RS)
constexpr int GPIO_HARD_RESET = 22;          // OUTPUT - Kết nối NRST của STM32
constexpr int GPIO_LOAD_DATA = 18;           // INPUT  - Kết nối PB0 (DR) của STM32

// ---------- SPI0 (CE0 = GPIO 8) ----------
constexpr int SPI_CHANNEL = 0;               // /dev/spidev0.0
constexpr int SPI_SPEED = 1000000;           // 1 MHz
constexpr uint8_t SPI_CMD_TEMP = 0x01;       // Lệnh yêu cầu nhiệt độ từ STM32

// ---------- UART LoRa (ttyAMA0) ----------
constexpr const char* LORA_UART = "/dev/serial0";  // GPIO 14/15
constexpr int LORA_BAUD = 115200;

// ---------- Tham số thời gian (ms) ----------
constexpr uint32_t MOTOR_ON_TIME = 70000;    // 70 giây motor
constexpr uint32_t TRIGGER_DELAY = 10000;    // 10 giây sau khi motor khởi động
constexpr uint32_t TIMEOUT_CAPTURE = 60000;  // 60 giây để nhận đủ 3 gói
constexpr uint32_t RECEIVED_COMPLETE_PULSE = 1000; // 1 giây xung RC
constexpr uint32_t FORCE_RESET_PULSE = 2000; // 2 giây reset
constexpr uint32_t SPI_TIMEOUT = 100;        // SPI read timeout (ms)

// ---------- Số lần capture thành công tối đa ----------
constexpr int MAX_CAPTURES = 3;

// ---------- Tọa độ sensor (cm) ----------
struct SensorPos {
    double x, y;
};
const SensorPos SENSORS[6] = {
    {-50, -50}, // A (kênh 0)
    {-50,  50}, // B (kênh 1)
    { 50,  50}, // C (kênh 2)
    { 50, -50}, // D (kênh 3)
    {-50,   0}, // E (kênh 4)
    {  0, -50}  // F (kênh 5)
};

// ---------- Thông tin node ----------
struct NodeID {
    int col;   // 1-5
    char row;  // A-D
};
NodeID parseNodeID(const std::string& str);  // Hàm chuyển "1A" thành {1,'A'}
std::string nodeIDString(const NodeID& id);  // "1A"

// ---------- Cấu hình LoRa cho từng cột ----------
constexpr int SF_BY_COL[] = {7, 8, 9, 10, 11}; // SF7 cho col1, SF11 cho col5
constexpr int FREQ_MHZ = 915;                   // Tần số (có thể 868 hoặc 915)