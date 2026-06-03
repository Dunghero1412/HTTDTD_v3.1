// ============================================================================
// File: system.h
// Mô tả: Định nghĩa các chân, hằng số và khai báo biến toàn cục dùng chung.
//        Hỗ trợ TIM2 (4 cảm biến A,B,C,D) + TIM5 (2 cảm biến E,F)
//        Clock: SYSCLK=168MHz, APB1=42MHz->TIM=84MHz, APB2=84MHz->TIM=168MHz
//        Lưu ý: TIM2 và TIM5 đều ở APB1 nên có cùng clock 84MHz, resolution ~11.9ns
//        Không chỉnh sửa file này nếu không cần thiết, tránh xung đột chân và cấu hình với các module khác.
//        Tác giả: Chiêm Dũng
//        Ngày tạo: 29/05/2026
// ============================================================================
#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f4xx_hal.h" // Thư viện HAL cho STM32F4

// ---------- TIM2 input capture (cảm biến piezoelectric A,B,C,D) @ 84MHz ----------
#define TIM2_CH1_PIN        GPIO_PIN_0 // Sensor A
#define TIM2_CH1_PORT       GPIOA
#define TIM2_CH2_PIN        GPIO_PIN_1 // Sensor B
#define TIM2_CH2_PORT       GPIOA
#define TIM2_CH3_PIN        GPIO_PIN_2 // Sensor C
#define TIM2_CH3_PORT       GPIOA
#define TIM2_CH4_PIN        GPIO_PIN_3 // Sensor D
#define TIM2_CH4_PORT       GPIOA

// ---------- TIM5 input capture (cảm biến piezoelectric E,F) @ 84MHz ----------
// *** CHÂN CỦA TIM5 TRÊN STM32F407VET6 ***
// Tuỳ chọn 1: Sử dụng Port H (khuyến cáo - tránh xung đột)
#define TIM5_CH1_PIN        GPIO_PIN_10    // Sensor E
#define TIM5_CH1_PORT       GPIOH
#define TIM5_CH2_PIN        GPIO_PIN_11    // Sensor F
#define TIM5_CH2_PORT       GPIOH

// Tuỳ chọn 2: Nếu không có Port H trên PCB, sử dụng Port I (AF_TIM5)
// #define TIM5_CH1_PIN        GPIO_PIN_0     // Sensor E
// #define TIM5_CH1_PORT       GPIOI
// #define TIM5_CH2_PIN        GPIO_PIN_1     // Sensor F
// #define TIM5_CH2_PORT       GPIOI

// Tuỳ chọn 3: Nếu chỉ có Port A,B,C, sử dụng PH10 trên Port H (cần kiểm tra PCB)
// Lưu ý: AF_TIM5_CH1 = PH10, AF_TIM5_CH2 = PH11 (AF2)

// ---------- SPI2 – BME280 (master) ----------
#define BME_CS_PIN          GPIO_PIN_12 // PB12
#define BME_CS_PORT         GPIOB
#define BME_SCK_PIN         GPIO_PIN_13 // PB13
#define BME_SCK_PORT        GPIOB
#define BME_MISO_PIN        GPIO_PIN_14 // PB14
#define BME_MISO_PORT       GPIOB
#define BME_MOSI_PIN        GPIO_PIN_15 // PB15
#define BME_MOSI_PORT       GPIOB

// ---------- SPI3 – Giao tiếp với Raspberry Pi (slave) ----------
#define SPI3_NSS_PIN        GPIO_PIN_15 // PA15
#define SPI3_NSS_PORT       GPIOA
#define SPI3_SCK_PIN        GPIO_PIN_10 // PC10
#define SPI3_SCK_PORT       GPIOC
#define SPI3_MISO_PIN       GPIO_PIN_11 // PC11
#define SPI3_MISO_PORT      GPIOC
#define SPI3_MOSI_PIN       GPIO_PIN_12 // PC12
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
#define UART1_TX_PIN        GPIO_PIN_9   // PA9
#define UART1_TX_PORT       GPIOA
#define UART1_RX_PIN        GPIO_PIN_10  // PA10
#define UART1_RX_PORT       GPIOA

// ---------- Hằng số hệ thống ----------
#define MAX_SUCCESS_COUNT       3           // Số lần truyền thành công tối đa rồi dừng
#define TEMP_UPDATE_INTERVAL_MS 30000       // Định kỳ gửi nhiệt độ (30 giây)

// ---------- Clock configuration & Timer resolution ----------
// SYSCLK = 168MHz (HSE 8MHz -> PLL M=8, N=336, P=2)
// APB1 = 42MHz (DIV4), Timer Clock APB1 = 84MHz (×2 vì DIV > 1)
// APB2 = 84MHz (DIV2), Timer Clock APB2 = 168MHz (×2 vì DIV > 1)
// => TIM2, TIM5 ở APB1 -> 84MHz
// => Resolution: 1 / 84MHz ≈ 11.9 ns
#define TIMER_CLOCK_HZ      84000000U      // TIM2, TIM5 clock frequency (Hz)
#define TIMER_RESOLUTION_NS 12             // ~11.9ns (1/84MHz)

// ---------- Biến toàn cục overflow counters ----------
// Biến đếm tràn TIM2 (toàn cục, sử dụng để mở rộng timestamp lên 64-bit)
// nên khai báo theo kiểu volatile vì nó được cập nhật trong ngắt và đọc ở main loop
extern volatile uint32_t overflow_count;

// Biến đếm tràn TIM5 (toàn cục, sử dụng để mở rộng timestamp lên 64-bit)
// extern volatile uint32_t overflow_count_tim5;
// vì TIM2 và TIM5 cùng source clock là APB1 (84MHz) nên overflow_count_tim5 không cần thiết
// có thể thêm nếu muốn sử dụng TIM5 cho mục đích khác

#ifdef __cplusplus
}
#endif
