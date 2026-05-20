// ============================================================================
// File: bme280.cpp
// Mô tả: Triển khai driver BME280.
// ============================================================================
#include "bme280.hpp"

extern SPI_HandleTypeDef hspi2;   // Khai báo ở main.cpp

uint16_t BME280::dig_T1 = 0;
int16_t  BME280::dig_T2 = 0;
int16_t  BME280::dig_T3 = 0;

// Chân CS (PB12)
void BME280::csLow()  { HAL_GPIO_WritePin(BME_CS_PORT, BME_CS_PIN, GPIO_PIN_RESET); }
void BME280::csHigh() { HAL_GPIO_WritePin(BME_CS_PORT, BME_CS_PIN, GPIO_PIN_SET); }

// Ghi 1 byte vào thanh ghi (địa chỉ 7 bit, bit RW=0)
void BME280::writeRegister(uint8_t reg, uint8_t value) {
    uint8_t txData[2] = { reg & 0x7F, value };   // RW=0
    csLow();
    HAL_SPI_Transmit(&hspi2, txData, 2, HAL_MAX_DELAY);
    csHigh();
}

// Đọc 1 byte từ thanh ghi
uint8_t BME280::readRegister(uint8_t reg) {
    uint8_t tx = reg | 0x80; // RW=1
    uint8_t rx = 0;
    csLow();
    HAL_SPI_Transmit(&hspi2, &tx, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi2, &rx, 1, HAL_MAX_DELAY);
    csHigh();
    return rx;
}

// Đọc nhiều byte liên tiếp
void BME280::readRegisters(uint8_t reg, uint8_t* data, uint8_t len) {
    uint8_t tx = reg | 0x80;
    csLow();
    HAL_SPI_Transmit(&hspi2, &tx, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi2, data, len, HAL_MAX_DELAY);
    csHigh();
}

// Reset sensor (ghi 0xB6 vào thanh ghi reset)
void BME280::resetSensor() {
    writeRegister(0xE0, 0xB6);
    HAL_Delay(10);
}

// Kiểm tra bit đo (status 0xF3 bit 3)
bool BME280::isMeasuring() {
    uint8_t status = readRegister(0xF3);
    return (status & 0x08) != 0;
}

// Đọc hệ số bù nhiệt
bool BME280::readCalibrationData() {
    uint8_t calib[6];
    readRegisters(0x88, calib, 6);
    dig_T1 = (uint16_t)(calib[1] << 8 | calib[0]);
    dig_T2 = (int16_t)(calib[3] << 8 | calib[2]);
    dig_T3 = (int16_t)(calib[5] << 8 | calib[4]);
    return true;
}

bool BME280::init() {
    // CS mặc định high
    HAL_GPIO_WritePin(BME_CS_PORT, BME_CS_PIN, GPIO_PIN_SET);

    // Reset
    resetSensor();

    // Kiểm tra chip ID (0x60)
    if (readRegister(0xD0) != 0x60) {
        return false; // Không nhận đúng sensor
    }

    // Cấu hình oversampling và chế độ
    // ctrl_hum (0xF2): osrs_h = 1 (x1)
    writeRegister(0xF2, 0x01);
    // ctrl_meas (0xF4): osrs_t = 1, osrs_p = 1, mode = forced
    writeRegister(0xF4, 0x25); // 001 001 01
    // config (0xF5): standby 0.5ms, filter off, spi 3-wire tắt
    writeRegister(0xF5, 0x00);

    // Đọc calibration
    return readCalibrationData();
}

bool BME280::readTemperature(float& temp) {
    // Bắt đầu phép đo forced
    uint8_t ctrl = readRegister(0xF4);
    ctrl &= 0xFC;   // Xóa mode[1:0]
    ctrl |= 0x01;   // forced mode
    writeRegister(0xF4, ctrl);

    // Chờ đo xong (polling status hoặc delay)
    while (isMeasuring()) { HAL_Delay(1); }

    // Đọc 3 byte nhiệt độ (0xFA, 0xFB, 0xFC)
    uint8_t raw[3];
    readRegisters(0xFA, raw, 3);
    int32_t adc_T = ((uint32_t)raw[0] << 12) | ((uint32_t)raw[1] << 4) | (raw[2] >> 4);

    // Tính nhiệt độ theo công thức BME280 (bỏ qua phần bù bậc cao)
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
    int32_t t_fine = var1 + var2;
    temp = (t_fine * 5 + 128) >> 8;   // Đơn vị 0.01 °C
    temp /= 100.0f;
    return true;
}
