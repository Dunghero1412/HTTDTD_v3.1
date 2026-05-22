// ============================================================================
// File: LoRaModule.cpp
// ============================================================================
#include "LoRaModule.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>
#include <iostream>
#include <sstream>
#include <thread>
#include <chrono>

bool LoRaModule::init(const std::string& uartDev, int baud, int sf, int freqMHz) {
    this->sf = sf;
    this->freq = freqMHz;
    uart_fd = open(uartDev.c_str(), O_RDWR | O_NOCTTY);
    if (uart_fd < 0) return false;

    struct termios tty;
    memset(&tty, 0, sizeof tty);
    tcgetattr(uart_fd, &tty);
    cfsetospeed(&tty, B115200);
    cfsetispeed(&tty, B115200);
    tty.c_cflag = CS8 | CREAD | CLOCAL;
    tty.c_iflag = IGNPAR;
    tty.c_oflag = 0;
    tty.c_lflag = 0;
    tcflush(uart_fd, TCIFLUSH);
    tcsetattr(uart_fd, TCSANOW, &tty);

    // Cấu hình LoRa AT
    std::string cmd;
    cmd = "AT+RESET\r\n"; write(uart_fd, cmd.c_str(), cmd.size());
    usleep(500000);
    cmd = "AT+MODE=0\r\n"; write(uart_fd, cmd.c_str(), cmd.size());
    usleep(100000);
    cmd = "AT+IPR=115200\r\n"; write(uart_fd, cmd.c_str(), cmd.size());
    usleep(100000);
    cmd = "AT+FREQ=" + std::to_string(freqMHz) + "000000\r\n"; write(uart_fd, cmd.c_str(), cmd.size());
    usleep(100000);
    cmd = "AT+SF=" + std::to_string(sf) + "\r\n"; write(uart_fd, cmd.c_str(), cmd.size());
    usleep(100000);
    // Thêm các cấu hình khác nếu cần
    return true;
}

bool LoRaModule::send(const std::string& data) {
    std::string cmd = "AT+SEND=0," + std::to_string(data.size()) + "," + data + "\r\n";
    write(uart_fd, cmd.c_str(), cmd.size());
    // Chờ phản hồi OK
    usleep(200000);
    return true;
}

void LoRaModule::setReceiveCallback(std::function<void(const std::string&)> cb) {
    recvCallback = cb;
}

void LoRaModule::process() {
    char c;
    while (read(uart_fd, &c, 1) > 0) {
        if (c == '\n') {
            if (!rxBuffer.empty()) {
                parseLine(rxBuffer);
                rxBuffer.clear();
            }
        } else {
            rxBuffer += c;
        }
    }
}

void LoRaModule::parseLine(const std::string& line) {
    // Tìm dữ liệu nhận dạng +RCV=<addr>,<len>,<data>
    size_t pos = line.find("+RCV=");
    if (pos != std::string::npos) {
        // Trích xuất data
        size_t comma1 = line.find(',', pos+5);
        size_t comma2 = line.find(',', comma1+1);
        if (comma1 != std::string::npos && comma2 != std::string::npos) {
            std::string data = line.substr(comma2+1);
            if (recvCallback) recvCallback(data);
        }
    }
}

std::string LoRaModule::connectionInfo() const {
    return "SF" + std::to_string(sf) + "-" + std::to_string(freq);
}

int LoRaModule::batteryPercent() const {
    // TODO: đọc ADC hoặc giả lập
    return 85;
}