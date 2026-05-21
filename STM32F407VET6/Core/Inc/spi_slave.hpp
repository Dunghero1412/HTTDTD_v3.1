// ============================================================================
// File: spi_slave.hpp
// Mô tả: Lớp điều khiển SPI3 ở chế độ slave để gửi dữ liệu sang Raspberry Pi.
// ============================================================================
#pragma once
#include "system.h"

class SPISlave {
public:
    // Khởi tạo SPI3 slave và cấu hình chân NSS
    static void init();

    // Đặt buffer dữ liệu sẽ gửi khi RPI yêu cầu
    static void setTxData(const uint8_t* data, uint16_t length);

    // Xử lý ngắt SPI3 (điền dữ liệu vào TXDR)
    static void handleSPIInterrupt();

    // Xử lý ngắt EXTI của chân NSS (PA15) – gọi khi có cạnh lên/xuống
    static void handleNSSInterrupt();

    // Được gọi khi NSS xuống thấp (bắt đầu phiên truyền)
    static void onNSSFalling();
    // Được gọi khi NSS lên cao (kết thúc phiên truyền)
    static void onNSSRising();

    static bool transmitting;   // true nếu NSS đang thấp (đang trong phiên truyền)
    static const uint8_t* txBuf;
    static uint16_t txLen;
    static uint16_t txIndex;
};