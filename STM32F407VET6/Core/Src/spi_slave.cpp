// ============================================================================
// File: spi_slave.cpp
// Mô tả: Triển khai SPISlave cho giao tiếp SPI slave.
// ============================================================================
#include "spi_slave.hpp"
#include "system.h"

extern SPI_HandleTypeDef hspi3;   // Khai báo ở main.cpp

bool SPISlave::transmitting = false;
const uint8_t* SPISlave::txBuf = nullptr;
uint16_t SPISlave::txLen = 0;
uint16_t SPISlave::txIndex = 0;

void SPISlave::init() {
    // Cấu hình chân NSS (PA15) làm ngắt ngoài cả cạnh lên và xuống
    GPIO_InitTypeDef gpio = {0};
    gpio.Pin = SPI3_NSS_PIN;
    gpio.Mode = GPIO_MODE_IT_RISING_FALLING;
    gpio.Pull = GPIO_PULLUP;      // Kéo lên để tránh trôi khi chưa có master
    HAL_GPIO_Init(SPI3_NSS_PORT, &gpio);

    // Cấu hình ngắt EXTI15
    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void SPISlave::setTxData(const uint8_t* data, uint16_t length) {
    txBuf = data;
    txLen = length;
    txIndex = 0;
    // Nếu đang trong phiên truyền (NSS thấp) thì không reset index vì
    // dữ liệu mới sẽ được gửi ở phiên sau, không ảnh hưởng phiên hiện tại.
    // Khi NSS xuống lần tới, onNSSFalling sẽ đặt lại txIndex = 0.
}

// Ngắt SPI3: nạp byte kế tiếp vào TXDR khi TXE được set
void SPISlave::handleSPIInterrupt() {
    if (__HAL_SPI_GET_FLAG(&hspi3, SPI_FLAG_TXE)) {
        if (transmitting && txBuf != nullptr) {
            if (txIndex < txLen) {
                *((__IO uint8_t*)&hspi3.Instance->DR) = txBuf[txIndex];
                txIndex++;
            } else {
                *((__IO uint8_t*)&hspi3.Instance->DR) = 0x00; // padding
            }
        } else {
            // Không truyền, gửi byte 0
            *((__IO uint8_t*)&hspi3.Instance->DR) = 0x00;
        }
    }
}

// Xử lý ngắt NSS: phân biệt cạnh lên/xuống dựa trên mức chân
void SPISlave::handleNSSInterrupt() {
    if (HAL_GPIO_ReadPin(SPI3_NSS_PORT, SPI3_NSS_PIN) == GPIO_PIN_RESET) {
        onNSSFalling();
    } else {
        onNSSRising();
    }
}

void SPISlave::onNSSFalling() {
    transmitting = true;
    txIndex = 0;   // Bắt đầu gửi từ đầu buffer
}

void SPISlave::onNSSRising() {
    transmitting = false;
}

// ============================================================================
// ISR cho SPI3 và EXTI15_10
// ============================================================================
extern "C" void SPI3_IRQHandler(void) {
    SPISlave::handleSPIInterrupt();
}

extern "C" void EXTI15_10_IRQHandler(void) {
    // Kiểm tra xem ngắt đến từ PA15 (line 15)
    if (__HAL_GPIO_EXTI_GET_IT(SPI3_NSS_PIN) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(SPI3_NSS_PIN);
        SPISlave::handleNSSInterrupt();
    }
}
