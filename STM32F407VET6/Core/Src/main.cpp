// ============================================================================
// File: Core/Src/main.cpp
// Mô tả: Hàm chính, khởi tạo phần cứng và vòng lặp chính.
// ============================================================================

// Quan trọng: Include theo thứ tự đúng
#include <stdint.h>
#include <string.h>
#include <stdio.h>

// Include HAL headers với extern "C"
extern "C" {
//    #include "stm32f4xx_hal.h"
}

// Include các module của dự án
#include "system.h"
#include "tdoa_manager.hpp"
#include "bme280.hpp"
#include "spi_slave.hpp"
#include "temperature_logger.hpp"

// ---------- Handle các ngoại vi (khai báo toàn cục) ----------
UART_HandleTypeDef huart1;
SPI_HandleTypeDef hspi2;
SPI_HandleTypeDef hspi3;

// ---------- Khai báo hàm trước ----------
extern "C" {
    void SystemClock_Config(void);
    void MX_GPIO_Init(void);
    void MX_USART1_UART_Init(void);
    void MX_SPI2_Init(void);
    void MX_SPI3_Init(void);
}

// ---------- Hỗ trợ printf qua UART1 ----------
extern "C" {
    int _write(int file, char *ptr, int len) {
        HAL_UART_Transmit(&huart1, (uint8_t*)ptr, len, HAL_MAX_DELAY);
        return len;
    }
}

/**
 * @brief  Hàm chính
 * @note   Khởi tạo phần cứng, các module ứng dụng và chạy vòng lặp chính
 */
int main(void) {
    // Khởi tạo HAL
    HAL_Init();
    
    // Cấu hình clock hệ thống
    SystemClock_Config();
    
    // Khởi tạo các ngoại vi
    MX_GPIO_Init();
    MX_USART1_UART_Init();
    MX_SPI2_Init();
    MX_SPI3_Init();
    
    // Khởi tạo các module ứng dụng
    TDOAManager::init();
    SPISlave::init();
    
    // Khởi tạo BME280
    if (!BME280::init()) {
        printf("BME280 init failed!\r\n");
    } else {
        printf("BME280 OK\r\n");
    }
    
    // Khởi tạo bộ gửi nhiệt độ
    TemperatureLogger::init();
    
    printf("TDOA System ready.\r\n");
    printf("MCU: STM32F407VET6 @ 168MHz\r\n");
    printf("Timer resolution: ~11.9ns\r\n");
    
    // Vòng lặp chính
    while (1) {
        // Xử lý máy trạng thái TDOA
        TDOAManager::processEvents();
        
        // Gửi nhiệt độ định kỳ
        TemperatureLogger::update();
    }
}

/**
 * @brief  Cấu hình clock hệ thống
 * @note   HSE 8MHz -> PLL 168MHz, APB1 = 42MHz (Timer = 84MHz)
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct;memset(&RCC_OscInitStruct, 0, sizeof(RCC_OscInitStruct));
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    // Bật Power Controller và cấu hình voltage scale
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    
    // Cấu hình oscillator: HSE
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        // Lỗi cấu hình oscillator - xử lý tùy ứng dụng
        while (1) {
            // Blink LED báo lỗi hoặc log
        }
    }
    
    // Cấu hình clock phân phối
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK 
                                 | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;  // APB1 = 42MHz -> Timer = 84MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;  // APB2 = 84MHz
    
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK) {
        // Lỗi cấu hình clock
        while (1) {}
    }
}

/**
 * @brief  Khởi tạo GPIO cơ bản (clock enable)
 */
void MX_GPIO_Init(void) {
    // Bật clock cho các GPIO port
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();
}

/**
 * @brief  Khởi tạo USART1 (Debug/UART)
 */
void MX_USART1_UART_Init(void) {
    __HAL_RCC_USART1_CLK_ENABLE();
    
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        while (1) {}
    }
}

/**
 * @brief  Khởi tạo SPI2 (Master - BME280)
 */
void MX_SPI2_Init(void) {
    __HAL_RCC_SPI2_CLK_ENABLE();
    
    hspi2.Instance = SPI2;
    hspi2.Init.Mode = SPI_MODE_MASTER;
    hspi2.Init.Direction = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi2.Init.NSS = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi2.Init.CRCPolynomial = 10;
    
    if (HAL_SPI_Init(&hspi2) != HAL_OK) {
        while (1) {}
    }
}

/**
 * @brief  Khởi tạo SPI3 (Slave - Giao tiếp với RPI)
 */
void MX_SPI3_Init(void) {
    __HAL_RCC_SPI3_CLK_ENABLE();
    
    hspi3.Instance = SPI3;
    hspi3.Init.Mode = SPI_MODE_SLAVE;
    hspi3.Init.Direction = SPI_DIRECTION_2LINES;
    hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi3.Init.NSS = SPI_NSS_HARD_INPUT;
    hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi3.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi3.Init.CRCPolynomial = 10;
    
    if (HAL_SPI_Init(&hspi3) != HAL_OK) {
        while (1) {}
    }
    
    // Cấu hình ngắt SPI3
    HAL_NVIC_SetPriority(SPI3_IRQn, 3, 0);
    HAL_NVIC_EnableIRQ(SPI3_IRQn);
}
