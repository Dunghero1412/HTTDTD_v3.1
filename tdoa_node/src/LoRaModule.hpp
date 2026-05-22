// ============================================================================
// File: LoRaModule.hpp
// Mô tả: Giao tiếp UART với module LoRa SX1276 (AT command).
// ============================================================================
#pragma once
#include <string>
#include <cstdint>
#include <functional>

class LoRaModule {
public:
    // Khởi tạo UART và cấu hình LoRa (SF, frequency, ...)
    bool init(const std::string& uartDev, int baud, int sf, int freqMHz);

    // Gửi chuỗi dữ liệu qua LoRa (AT+SEND)
    bool send(const std::string& data);

    // Đặt callback khi nhận dữ liệu (chuỗi hoàn chỉnh)
    void setReceiveCallback(std::function<void(const std::string&)> cb);

    // Vòng lặp xử lý (cần gọi liên tục hoặc trong thread)
    void process();

    // Trả về thông tin kết nối (ví dụ "SF7-915")
    std::string connectionInfo() const;

    // Đọc pin (giả lập hoặc thật nếu có kết nối ADC)
    int batteryPercent() const; // Tạm trả về 85

private:
    int uart_fd;
    std::string rxBuffer;
    std::function<void(const std::string&)> recvCallback;
    int sf;
    int freq;
    void parseLine(const std::string& line);
};