// ============================================================================
// File: temperature_logger.hpp
// Mô tả: Định kỳ đọc nhiệt độ và gửi qua UART1.
// ============================================================================
#pragma once
#include "system.h"

class TemperatureLogger {
public:
    static void init();
    static void update();

private:
    static uint32_t lastTempTime;   // Tick Systick lần cuối gửi
};