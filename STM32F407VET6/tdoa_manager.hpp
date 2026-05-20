// ============================================================================
// File: system.h
// Mô tả: Định nghĩa các chân, hằng số và khai báo biến toàn cục dùng chung.
// ============================================================================
#pragma once
#include "stm32f4xx_hal.h"

// ---------- TIM2 input capture (cảm biến piezoelectric) ----------
#define TIM2_CH1_PIN        GPIO_PIN_0
#define TIM2_CH1_PORT       GPIOA
#define TIM2_CH2_PIN        GPIO_PIN_1
#define TIM2_CH2_PORT       GPIOA
#define TIM2_CH3_PIN        GPIO_PIN_2
#define TIM2_CH3_PORT       GPIOA
#define TIM2_CH4_PIN        GPIO_PIN_3
#define TIM2_CH4_PORT       GPIOA

// ---------- SPI2 – BME280 (master) ----------
#define BME_CS_PIN          GPIO_PIN_12
#define BME_CS_PORT         GPIOB
#define BME_SCK_PIN         GPIO_PIN_13
#define BME_SCK_PORT        GPIOB
#define BME_MISO_PIN        GPIO_PIN_14
#define BME_MISO_PORT       GPIOB
#define BME_MOSI_PIN        GPIO_PIN_15
#define BME_MOSI_PORT       GPIOB

// ---------- SPI3 – Giao tiếp với Raspberry Pi (slave) ----------
#define SPI3_NSS_PIN        GPIO_PIN_15
#define SPI3_NSS_PORT       GPIOA
#define SPI3_SCK_PIN        GPIO_PIN_10
#define SPI3_SCK_PORT       GPIOC
#define SPI3_MISO_PIN       GPIO_PIN_11
#define SPI3_MISO_PORT      GPIOC
#define SPI3_MOSI_PIN       GPIO_PIN_12
#define SPI3_MOSI_PORT      GPIOC

// ---------- Các chân điều khiển ----------
#define DATA_READY_PIN      GPIO_PIN_0   // PB0 - Báo dữ liệu sẵn sàng (output)
#define DATA_READY_PORT     GPIOB
#define RDC_PIN             GPIO_PIN_1   // PB1 - Read Data Complete (input, ngắt)
#define RDC_PORT            GPIOB
#define TC_PIN              GPIO_PIN_2   // PB2 - Trigger Command (input, ngắt)
#define TC_PORT             GPIOB
#define RS_PIN              GPIO_PIN_4   // PB4 - Reset (input, ngắt)
#define RS_PORT             GPIOB

// ---------- USART1 – Debug/UART ----------
#define UART1_TX_PIN        GPIO_PIN_9
#define UART1_TX_PORT       GPIOA
#define UART1_RX_PIN        GPIO_PIN_10
#define UART1_RX_PORT       GPIOA

// ---------- Hằng số hệ thống ----------
#define MAX_SUCCESS_COUNT       3           // Số lần truyền thành công tối đa rồi dừng
#define TEMP_UPDATE_INTERVAL_MS 30000       // Định kỳ gửi nhiệt độ (30 giây)

// Biến đếm tràn TIM2 (toàn cục, sử dụng để mở rộng timestamp lên 64-bit)
extern volatile uint32_t overflow_count;