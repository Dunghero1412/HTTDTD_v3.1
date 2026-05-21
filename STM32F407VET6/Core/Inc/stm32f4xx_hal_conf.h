// ============================================================================
// File: Core/Inc/stm32f4xx_hal_conf.h
// Mô tả: Cấu hình các module HAL được sử dụng trong dự án TDOA.
//        Chỉ bật các module cần thiết để tiết kiệm bộ nhớ và tăng tốc build.
// ============================================================================

#ifndef __STM32F4xx_HAL_CONF_H
#define __STM32F4xx_HAL_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

// #include "stm32f4xx_hal.h"
// ---------- Chọn module HAL cần dùng (1 = bật, 0 = tắt) ----------
#define HAL_MODULE_ENABLED          1   // HAL core

// Ngoại vi cơ bản - bắt buộc
#define HAL_RCC_MODULE_ENABLED      1   // Cấu hình clock
#define HAL_GPIO_MODULE_ENABLED     1   // Điều khiển GPIO
#define HAL_DMA_MODULE_ENABLED      0   // Không dùng DMA trong dự án này
#define HAL_CORTEX_MODULE_ENABLED   1   // NVIC, SysTick
#define HAL_PWR_MODULE_ENABLED      1   // Quản lý nguồn (cần cho clock)
#define HAL_FLASH_MODULE_ENABLED    1   // Thao tác Flash (cần cho clock)

// Ngoại vi giao tiếp
#define HAL_TIM_MODULE_ENABLED      1   // Timer (TIM2 capture)
#define HAL_SPI_MODULE_ENABLED      1   // SPI2 (BME280) + SPI3 (RPI)
#define HAL_UART_MODULE_ENABLED     1   // USART1 debug
#define HAL_EXTI_MODULE_ENABLED     1   // Ngắt ngoài (TC, RDC, RS, NSS)

// Ngoại vi không dùng - tắt hết
#define HAL_ADC_MODULE_ENABLED      0
#define HAL_CAN_MODULE_ENABLED      0
#define HAL_CAN_LEGACY_MODULE_ENABLED 0
#define HAL_CRC_MODULE_ENABLED      0
#define HAL_CRYP_MODULE_ENABLED     0
#define HAL_DAC_MODULE_ENABLED      0
#define HAL_DCMI_MODULE_ENABLED     0
#define HAL_DMA2D_MODULE_ENABLED    0
#define HAL_ETH_MODULE_ENABLED      0
#define HAL_NAND_MODULE_ENABLED     0
#define HAL_NOR_MODULE_ENABLED      0
#define HAL_PCCARD_MODULE_ENABLED   0
#define HAL_SRAM_MODULE_ENABLED     0
#define HAL_SDRAM_MODULE_ENABLED    0
#define HAL_HASH_MODULE_ENABLED     0
#define HAL_I2C_MODULE_ENABLED      0
#define HAL_I2S_MODULE_ENABLED      0
#define HAL_IWDG_MODULE_ENABLED     0
#define HAL_LTDC_MODULE_ENABLED     0
#define HAL_DSI_MODULE_ENABLED      0
#define HAL_RTC_MODULE_ENABLED      0
#define HAL_SD_MODULE_ENABLED       0
#define HAL_MMC_MODULE_ENABLED      0
#define HAL_SPDIFRX_MODULE_ENABLED  0
#define HAL_USART_MODULE_ENABLED    0   // Dùng UART thay vì USART
#define HAL_WWDG_MODULE_ENABLED     0
#define HAL_FMPI2C_MODULE_ENABLED   0
#define HAL_FMPI2C_EX_MODULE_ENABLED 0
#define HAL_QSPI_MODULE_ENABLED     0
#define HAL_SAI_MODULE_ENABLED      0
#define HAL_SMBUS_MODULE_ENABLED    0

// ---------- Cấu hình tần số HSE (thạch anh ngoài) ----------
#if !defined(HSE_VALUE)
  #define HSE_VALUE    8000000U   // Thạch anh 8 MHz
#endif

// ---------- Cấu hình tần số HSI (nội) ----------
#if !defined(HSI_VALUE)
  #define HSI_VALUE    16000000U
#endif

// ---------- Cấu hình xung nhịp SysTick ----------
#define TICK_INT_PRIORITY            0x0FU  // Ưu tiên thấp nhất

// ---------- Bật assert_param (nên tắt khi release để tối ưu) ----------
#define USE_FULL_ASSERT    0U

// ---------- Cấu hình ngắt ----------
#ifdef USE_RTOS
  #define USE_RTOS          0U
#endif

#if !defined(HSE_STARTUP_TIMEOUT)
  #define HSE_STARTUP_TIMEOUT   100U   // 100ms timeout HSE ready
#endif

#if !defined(LSE_STARTUP_TIMEOUT)
  #define LSE_STARTUP_TIMEOUT   5000U   // 5s timeout LSE ready
#endif

#if !defined(LSI_VALUE)
  #define LSI_VALUE    32000U
#endif
#if !defined(LSE_VALUE)
  #define LSE_VALUE    32768U
#endif

#define PREFETCH_ENABLE              1U
#define INSTRUCTION_CACHE_ENABLE     1U
#define DATA_CACHE_ENABLE            1U

/* ========== VDD voltage (mV) ========== */
#define VDD_VALUE                    3300U


// ---------- Include các header HAL theo module đã bật ----------
#ifdef HAL_RCC_MODULE_ENABLED
  #include "stm32f4xx_hal_rcc.h"
#endif

#ifdef HAL_GPIO_MODULE_ENABLED
  #include "stm32f4xx_hal_gpio.h"
#endif

#ifdef HAL_DMA_MODULE_ENABLED
  #include "stm32f4xx_hal_dma.h"
#endif

#ifdef HAL_CORTEX_MODULE_ENABLED
  #include "stm32f4xx_hal_cortex.h"
#endif

#ifdef HAL_PWR_MODULE_ENABLED
  #include "stm32f4xx_hal_pwr.h"
#endif

#ifdef HAL_FLASH_MODULE_ENABLED
  #include "stm32f4xx_hal_flash.h"
#endif

#ifdef HAL_TIM_MODULE_ENABLED
  #include "stm32f4xx_hal_tim.h"
#endif

#ifdef HAL_SPI_MODULE_ENABLED
  #include "stm32f4xx_hal_spi.h"
#endif

#ifdef HAL_UART_MODULE_ENABLED
  #include "stm32f4xx_hal_uart.h"
#endif

#ifdef HAL_EXTI_MODULE_ENABLED
  #include "stm32f4xx_hal_exti.h"
#endif

// ---------- Macro kiểm tra tham số (chỉ dùng khi debug) ----------
#ifdef USE_FULL_ASSERT
  #define assert_param(expr) ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
  void assert_failed(uint8_t* file, uint32_t line);
#else
  #define assert_param(expr) ((void)0U)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STM32F4xx_HAL_CONF_H */
