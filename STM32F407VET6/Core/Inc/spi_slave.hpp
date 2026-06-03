// ============================================================================
// File: spi_slave.hpp
// Mô tả: Lớp điều khiển SPI3 ở chế độ slave để gửi dữ liệu sang Raspberry Pi.
// việc sử dụng SPI3 vì nó có chân NSS riêng biệt (PA15) giúp dễ dàng quản lý tín hiệu chọn chip.
// RPI sẽ là master, STM32F407VET6 sẽ là slave, và chúng ta sẽ sử dụng ngắt để xử lý việc truyền dữ liệu khi RPI yêu cầu.
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
    static void onNSSFalling(); // Bắt đầu phiên truyền, rpi đưa NSS xuống thấp khi nhận lệnh từ DR và gửi dummy byte để kích hoạt truyền dữ liệu từ STM32F407VET6 sang RPI
    // Được gọi khi NSS lên cao (kết thúc phiên truyền)
    static void onNSSRising(); // Kết thúc phiên truyền, rpi đưa NSS lên cao đồng thời kích chân RDC để STM32F407VET6 biết đã hoàn thành truyền dữ liệu

    static bool transmitting;   // true nếu NSS đang thấp (đang trong phiên truyền)
    static const uint8_t* txBuf;
    static uint16_t txLen;
    static uint16_t txIndex;
};