// ============================================================================
// File: Core/Src/main.cpp
// Mô tả: Hàm chính, khởi tạo phần cứng và vòng lặp chính.
//        - SYSCLK: 168MHz (PLL từ HSE 8MHz)
//        - APB1: 42MHz -> Timer Clock: 84MHz (TIM2, TIM5)
//        - APB2: 84MHz -> Timer Clock: 168MHz (TIM1, TIM8)
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
#include "DebugUART.hpp"

// ---------- Handle các ngoại vi (khai báo toàn cục) ----------
UART_HandleTypeDef huart1;
SPI_HandleTypeDef hspi2;
SPI_HandleTypeDef hspi3;

// ---------- Khai báo hàm trước ----------
extern "C" {
    void SystemClock_Config(void);    // Thêm SystemClock_Config
    void MX_GPIO_Init(void);          // Thêm GPIO init
    void MX_USART1_UART_Init(void);   // Thêm UART1 init
    void MX_SPI2_Init(void);          // Thêm SPI2 init
    void MX_SPI3_Init(void);          // Thêm SPI3 init
    void MX_TIM2_Init(void);          // Thêm TIM2 init
    void MX_TIM5_Init(void);          // Thêm TIM5 init
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
    DebugUART::init(&huart1);
    DebugUART::setFileBufferEnabled(true);
    MX_SPI2_Init();
    MX_SPI3_Init();
    
    // Khởi tạo timer cho các cảm biến
    MX_TIM2_Init();  // Khởi tạo TIM2 cho cảm biến A,B,C,D
    MX_TIM5_Init();  // Khởi tạo TIM5 cho cảm biến E,F
    
    // Khởi tạo các module ứng dụng
    TDOAManager::init();
    SPISlave::init();
    
    // Khởi tạo BME280
    if (!BME280::init()) {
        DebugUART::log("BME280 init failed!\r\n");
    } else {
        DebugUART::log("BME280 OK\r\n");
    }
    
    // Khởi tạo bộ gửi nhiệt độ
    TemperatureLogger::init();
    
    DebugUART::log("TDOA System ready.\r\n");
    DebugUART::log("MCU: STM32F407VET6 @ 168MHz\r\n");
    DebugUART::log("TIM2 (A,B,C,D): 84MHz, ~11.9ns/tick\r\n");
    DebugUART::log("TIM5 (E,F): 84MHz, ~11.9ns/tick\r\n");
    
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
 * @note   HSE 8MHz -> PLL 168MHz (SYSCLK)
 *         APB1 = 42MHz (Prescaler = 4), Timer Clock = 84MHz
 *         APB2 = 84MHz (Prescaler = 2), Timer Clock = 168MHz
 *         FLASH latency = 5 wait states
 */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct;
    memset(&RCC_OscInitStruct, 0, sizeof(RCC_OscInitStruct));
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    
    // Bật Power Controller và cấu hình voltage scale
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    
    // Cấu hình oscillator: HSE 8MHz
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;   // Sử dụng HSE làm nguồn chính
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;                     // Bật HSE
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;                 // Bật PLL
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;         // Sử dụng HSE làm nguồn PLL
    RCC_OscInitStruct.PLL.PLLM = 8;                              // Chia HSE 8MHz cho 8 = 1MHz
    RCC_OscInitStruct.PLL.PLLN = 336;                            // Nhân lên 336MHz
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;                  // Chia 336 cho 2 = 168MHz (SYSCLK)
    RCC_OscInitStruct.PLL.PLLQ = 7;                              // Chia cho 7 = 48MHz (USB)
    
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
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;      // HCLK = 168MHz
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;       // APB1 = 42MHz -> Timer = 84MHz
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;       // APB2 = 84MHz -> Timer = 168MHz
    
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
    __HAL_RCC_GPIOA_CLK_ENABLE();  // Port A (TIM2, TIM3, TIM4, TIM5)
    __HAL_RCC_GPIOB_CLK_ENABLE();  // Port B (TIM3, TIM4)
    __HAL_RCC_GPIOC_CLK_ENABLE();  // Port C (TIM8)
    __HAL_RCC_GPIOH_CLK_ENABLE();  // Port H (TIM5)
    __HAL_RCC_GPIOI_CLK_ENABLE();  // Port I (nếu cần)
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

/**
 * @brief  Khởi tạo TIM5 (Input Capture - Sensor E,F) @ 84MHz
 * @note   TIM5 clock: APB1 = 42MHz -> Timer clock = 84MHz (với prescaler divisor)
 *         Configuration: 32-bit timer, PSC=0, Period=0xFFFFFFFF
 */
void MX_TIM5_Init(void) {
    __HAL_RCC_TIM5_CLK_ENABLE();
    
    TIM_HandleTypeDef htim5 = {0};
    htim5.Instance = TIM5;
    htim5.Init.Prescaler = 0;               // Không chia, tần số timer = 84MHz
    htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim5.Init.Period = 0xFFFFFFFF;         // 32-bit tự do, chỉ dùng tràn
    htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    if (HAL_TIM_IC_Init(&htim5) != HAL_OK) {
        while (1) {}
    }

    // Cấu hình kênh capture (xung rising, không lọc)
    TIM_IC_InitTypeDef icConfig = {0};
    icConfig.ICPolarity = TIM_ICPOLARITY_RISING;
    icConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
    icConfig.ICPrescaler = TIM_ICPSC_DIV1;
    icConfig.ICFilter = 0;
    
    HAL_TIM_IC_ConfigChannel(&htim5, &icConfig, TIM_CHANNEL_1);  // Sensor E
    HAL_TIM_IC_ConfigChannel(&htim5, &icConfig, TIM_CHANNEL_2);  // Sensor F

    // Bật ngắt tràn (update) và ngắt capture
    HAL_NVIC_SetPriority(TIM5_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM5_IRQn);
    __HAL_TIM_ENABLE_IT(&htim5, TIM_IT_UPDATE);
    __HAL_TIM_ENABLE_IT(&htim5, TIM_IT_CC5); // Sensor E
    __HAL_TIM_ENABLE_IT(&htim5, TIM_IT_CC6); // Sensor F

    // Khởi động timer
    HAL_TIM_Base_Start(&htim5);
}
/**
 * @brief  Khởi tạo TIM2 (Input capture - Sensor A,B,C,D) @ 84MHz
 * @note   TIM2 clock: APB1 = 42MHz -> Timer clock = 84MHz (với prescaler divisor)
 *         Configuration: 32-bit timer, PSC=0, Period=0xFFFFFFFF
 */
void MX_TIM2_Init(void) {
    __HAL_RCC_TIM2_CLK_ENABLE();
    
    TIM_HandleTypeDef htim2 = {0};
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;               // Không chia, tần số timer = 84MHz
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFFFFFF;         // 32-bit tự do, chỉ dùng tràn
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    
    if (HAL_TIM_IC_Init(&htim2) != HAL_OK) {
        while (1) {}
    }

    // Cấu hình kênh capture (xung rising, không lọc)
    TIM_IC_InitTypeDef icConfig = {0};
    icConfig.ICPolarity = TIM_ICPOLARITY_RISING;
    icConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
    icConfig.ICPrescaler = TIM_ICPSC_DIV1;
    icConfig.ICFilter = 0;
    
    HAL_TIM_IC_ConfigChannel(&htim2, &icConfig, TIM_CHANNEL_1);  // Sensor A
    HAL_TIM_IC_ConfigChannel(&htim2, &icConfig, TIM_CHANNEL_2);  // Sensor B
    HAL_TIM_IC_ConfigChannel(&htim2, &icConfig, TIM_CHANNEL_3);  // Sensor C
    HAL_TIM_IC_ConfigChannel(&htim2, &icConfig, TIM_CHANNEL_4);  // Sensor D

    // Bật ngắt tràn (update) và ngắt capture
    HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_CC1);
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_CC2);
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_CC3);
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_CC4);

    // Khởi động timer
    HAL_TIM_Base_Start(&htim2);
}

#ifdef  USE_FULL_ASSERT
extern "C" {
    void assert_failed(uint8_t *file, uint32_t line) {
        /* Bạn có thể thêm code xử lý lỗi hoặc vòng lặp vô hạn ở đây */
        while (1) {}
    }
}
#endif
