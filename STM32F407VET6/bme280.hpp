// ============================================================================
// File: bme280.hpp
// Mô tả: Lớp driver cho cảm biến BME280 qua SPI2.
// ============================================================================
#pragma once
#include "system.h"

class BME280 {
public:
    // Khởi tạo cảm biến, đọc calibration
    static bool init();

    // Đọc nhiệt độ (độ C). Trả về false nếu có lỗi.
    static bool readTemperature(float& temp);

private:
    // Các hàm giao tiếp SPI cơ bản
    static void csLow();
    static void csHigh();
    static void writeRegister(uint8_t reg, uint8_t value);
    static uint8_t readRegister(uint8_t reg);
    static void readRegisters(uint8_t reg, uint8_t* data, uint8_t len);

    // Các hàm hỗ trợ cảm biến
    static void resetSensor();
    static bool isMeasuring();
    static bool readCalibrationData();

    // Hệ số bù nhiệt (đọc từ sensor)
    static uint16_t dig_T1;
    static int16_t  dig_T2;
    static int16_t  dig_T3;
};