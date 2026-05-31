// ============================================================================
// File: SPIMaster.cpp
// ============================================================================
#include "SPIMaster.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <cstring>
#include <iostream>
#include <chrono>
#include <thread>
#include <sstream>

bool SPIMaster::init(int channel, int speed) {
    std::string dev = "/dev/spidev0." + std::to_string(channel);
    spi_fd = open(dev.c_str(), O_RDWR);
    if (spi_fd < 0) return false;
    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    return true;
}

uint8_t SPIMaster::transfer(uint8_t tx) {
    uint8_t rx;
    struct spi_ioc_transfer tr = {0};
    tr.tx_buf = (unsigned long)&tx;
    tr.rx_buf = (unsigned long)&rx;
    tr.len = 1;
    ioctl(spi_fd, SPI_IOC_MESSAGE(1), &tr);
    return rx;
}

void SPIMaster::chipSelect(bool active) {
    // GPIO 8 (CE0) được quản lý tự động bởi driver, nhưng ta có thể điều khiển thêm nếu cần.
    // Ở đây ta giả định driver tự động, không cần code.
}

std::vector<std::string> SPIMaster::readData(int timeoutMs) {
    std::vector<std::string> lines;
    std::string currentLine;
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::steady_clock::now() - start).count() < timeoutMs) {
        uint8_t rx = transfer(0x00); // Gửi byte rỗng để nhận
        if (rx == '\n') {
            if (!currentLine.empty()) {
                lines.push_back(currentLine);
                currentLine.clear();
                if (lines.size() >= 7) break; // A,B,C,D,E,F,Temp (7 dòng)
            }
        } else if (rx != '\r' && rx != 0xFF) {
            currentLine += static_cast<char>(rx);
        }
    }
    return lines;
}

float SPIMaster::requestTemperature() {
    transfer(SPI_CMD_TEMP); // Gửi lệnh
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::string tempLine;
    for (int i = 0; i < 20; ++i) {
        uint8_t rx = transfer(0x00);
        if (rx == '\n') break;
        if (rx != '\r' && rx != 0xFF) tempLine += rx;
    }
    // Giả sử dạng "T, 25.3"
    if (tempLine.size() > 2 && tempLine[0] == 'T' && tempLine[1] == ',') {
        return std::stof(tempLine.substr(2));
    }
    return 25.0f; // mặc định
}