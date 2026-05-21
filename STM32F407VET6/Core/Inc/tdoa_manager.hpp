// ============================================================================
// File: Core/Inc/tdoa_manager.hpp
// Mô tả: Máy trạng thái capture TDOA.
// ============================================================================
#pragma once

#include "system.h"
#include "spi_slave.hpp"

class TDOAManager {
public:
    enum State {
        IDLE,
        DELAY_2S,
        CAPTURING,
        DATA_READY
    };

    static void init();
    static void processEvents();

    // Callbacks từ ngắt
    static void onTC_IRQ();
    static void onRDC_IRQ();
    static void onRS_IRQ();
    static void onCaptureCH1(uint32_t capture_val);
    static void onCaptureCH2(uint32_t capture_val);
    static void onCaptureCH3(uint32_t capture_val);
    static void onCaptureCH4(uint32_t capture_val);
    static void onOverflow();

    // Timestamp 64-bit hiện tại (dùng cho debug/nhiệt độ)
    static uint64_t getCurrentTimestamp();

private:
    struct CaptureData {
        uint32_t overflow;
        uint32_t tick;
    };

    static void packDataForSPI();
    static void startCaptureSequence();
    static void stopCaptureAndClear();
    static void resetSystem();

    static State currentState;
    static CaptureData captures[4];
    static bool channelCaptured[4];
    static uint32_t successCount;

    static uint32_t delayStartTick;
    static bool delayActive;

    static char spiTxBuffer[256];
    static uint16_t spiTxLen;
};
