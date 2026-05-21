// ============================================================================
// File: Core/Src/system_stm32f4xx.c
// Mô tả: Cấu hình clock hệ thống cho STM32F407xx (CMSIS)
//        - SystemInit(): khởi tạo clock mặc định (HSI 16MHz)
//        - SystemCoreClockUpdate(): cập nhật biến SystemCoreClock
// ============================================================================

#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_hal_conf.h"

// Biến toàn cục chứa tần số lõi (Hz)
uint32_t SystemCoreClock = 16000000; // Mặc định HSI 16MHz

// Hệ số chia cho APB1/APB2 (dùng để tính clock cho timer)
const uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};
const uint8_t APBPrescTable[8]  = {0, 0, 0, 0, 1, 2, 3, 4};

/**
 * @brief  Khởi tạo hệ thống (được gọi trước main)
 * @note   Cấu hình: bật HSI, tắt HSE/PLL, cấu hình vector table và FPU
 */
void SystemInit(void) {
    // Bật FPU (Floating Point Unit)
    #if (__FPU_PRESENT == 1) && (__FPU_USED == 1)
    SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));  // Full access cho CP10 và CP11
    #endif

    // Reset clock configuration registers
    RCC->CR |= (uint32_t)0x00000001;   // Bật HSI
    RCC->CFGR = 0x00000000;            // Reset CFGR
    RCC->CR &= (uint32_t)0xFEF6FFFF;   // Tắt HSE, CSS, PLL
    RCC->PLLCFGR = 0x24003010;         // Reset PLLCFGR
    RCC->CR &= (uint32_t)0xFFFBFFFF;   // Tắt PLL
    RCC->CIR = 0x00000000;             // Tắt tất cả ngắt clock

    // Đặt vị trí bảng vector (mặc định ở đầu Flash)
    SCB->VTOR = FLASH_BASE;

    // Cấu hình ưu tiên ngắt (4 bit ưu tiên, 0 bit subpriority)
    NVIC_SetPriorityGrouping(0x03);

    // Cập nhật biến SystemCoreClock (HSI 16MHz)
    SystemCoreClock = 16000000;
}

/**
 * @brief  Cập nhật SystemCoreClock dựa trên cấu hình RCC hiện tại
 * @note   Gọi sau khi thay đổi clock để các thư viện (như HAL) có giá trị chính xác
 */
void SystemCoreClockUpdate(void) {
    uint32_t tmp = 0, pllvco = 0, pllp = 2, pllsource = 0, pllm = 2;

    // Lấy nguồn clock hệ thống
    tmp = RCC->CFGR & RCC_CFGR_SWS;
    switch (tmp) {
        case 0x00:  // HSI
            SystemCoreClock = HSI_VALUE;
            break;
        case 0x04:  // HSE
            SystemCoreClock = HSE_VALUE;
            break;
        case 0x08:  // PLL
            // Đọc nguồn PLL (HSE hay HSI)
            pllsource = (RCC->PLLCFGR & RCC_PLLCFGR_PLLSRC) >> 22;
            pllm = (RCC->PLLCFGR & RCC_PLLCFGR_PLLM) >> RCC_PLLCFGR_PLLM_Pos;

            if (pllsource != 0) {
                // HSE làm nguồn
                pllvco = (HSE_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos);
            } else {
                // HSI làm nguồn
                pllvco = (HSI_VALUE / pllm) * ((RCC->PLLCFGR & RCC_PLLCFGR_PLLN) >> RCC_PLLCFGR_PLLN_Pos);
            }

            // Xác định PLLP (hệ số chia đầu ra chính)
            pllp = (((RCC->PLLCFGR & RCC_PLLCFGR_PLLP) >> RCC_PLLCFGR_PLLP_Pos) + 1) * 2;
            SystemCoreClock = pllvco / pllp;
            break;
        default:
            SystemCoreClock = HSI_VALUE;
            break;
    }

    // Tính HCLK, PCLK1, PCLK2
    tmp = AHBPrescTable[((RCC->CFGR & RCC_CFGR_HPRE) >> RCC_CFGR_HPRE_Pos)];
    SystemCoreClock >>= tmp;  // HCLK = SYSCLK / (2^AHBPrescTable)

    // PCLK1 = HCLK / (2^APB1Presc)
    tmp = APBPrescTable[((RCC->CFGR & RCC_CFGR_PPRE1) >> RCC_CFGR_PPRE1_Pos)];
    // Nếu APB1 Prescaler > 1 thì timer clock = 2x PCLK1
    if (tmp > 0) {
        tmp = 1;
    } else {
        tmp = 0;
    }
    // (Biến SystemCoreClock vẫn là HCLK, không lưu PCLK1 riêng)
}
