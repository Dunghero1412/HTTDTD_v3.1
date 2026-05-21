// ============================================================================
// File: temperature_logger.cpp
// Mô tả: Triển khai gửi nhiệt độ định kỳ.
// ============================================================================
#include "system.h"
#include "temperature_logger.hpp"
#include "bme280.hpp"
#include "tdoa_manager.hpp"
#include <stdio.h>
#include <string.h>

extern UART_HandleTypeDef huart1;   // Khai báo ở main.cpp

uint32_t TemperatureLogger::lastTempTime = 0;

void TemperatureLogger::init() {
    lastTempTime = HAL_GetTick();
}

void TemperatureLogger::update() {
    if (HAL_GetTick() - lastTempTime >= TEMP_UPDATE_INTERVAL_MS) {
        lastTempTime = HAL_GetTick();
        float temp;
        if (BME280::readTemperature(temp)) {
            // Lấy timestamp hiện tại (64-bit) để gửi kèm
            uint64_t ts = TDOAManager::getCurrentTimestamp();
            char msg[64];
            int len = snprintf(msg, sizeof(msg), "0x%016llX, %.2f\r\n", ts, (double)temp);
            HAL_UART_Transmit(&huart1, (uint8_t*)msg, len, HAL_MAX_DELAY);
        } else {
            const char* err = "BME280 read error\r\n";
            HAL_UART_Transmit(&huart1, (uint8_t*)err, strlen(err), HAL_MAX_DELAY);
        }
    }
}
