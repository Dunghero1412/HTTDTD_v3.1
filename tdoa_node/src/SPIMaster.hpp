// ============================================================================
// File: SPIMaster.hpp
// Mô tả: Giao tiếp SPI với STM32 (master), dùng /dev/spidev0.0
// ============================================================================
#pragma once
#include <cstdint>
#include <string>
#include <vector>

class SPIMaster {
public:
    bool init(int channel, int speed);
    // Đọc dữ liệu từ STM32 (sau khi CE0 kéo thấp). Trả về vector các dòng.
    std::vector<std::string> readData(int timeoutMs);
    // Yêu cầu nhiệt độ (gửi lệnh 0x01 rồi đọc phản hồi)
    float requestTemperature();
private:
    int spi_fd = -1;
    uint8_t transfer(uint8_t tx);
    void chipSelect(bool active);
};