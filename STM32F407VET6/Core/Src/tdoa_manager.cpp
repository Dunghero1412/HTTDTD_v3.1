// ============================================================================
// File: tdoa_manager.cpp
// Mô tả: Triển khai các phương thức của TDOAManager và các trình phục vụ ngắt.
// ============================================================================
#include "tdoa_manager.hpp"
#include "system.h"
#include <stdio.h>
#include <string.h>

// ---------- Biến toàn cục ----------
volatile uint32_t overflow_count = 0;   // Đếm số lần tràn TIM2

// Handle của TIM2 (dùng trong nội bộ file)
static TIM_HandleTypeDef htim2;

// ---------- Các biến trạng thái tĩnh ----------
TDOAManager::State TDOAManager::currentState = TDOAManager::IDLE;
TDOAManager::CaptureData TDOAManager::captures[4];
bool TDOAManager::channelCaptured[4] = {false, false, false, false};
uint32_t TDOAManager::successCount = 0;
uint32_t TDOAManager::delayStartTick = 0;
bool TDOAManager::delayActive = false;
char TDOAManager::spiTxBuffer[256];
uint16_t TDOAManager::spiTxLen = 0;

// Cờ báo ngắt ngoài (được set trong ISR, xử lý trong processEvents)
static volatile bool tcFlag = false;
static volatile bool rdcFlag = false;
static volatile bool rsFlag = false;

// ---------- Các hàm nội bộ ----------

// Bắt đầu capture: bật ngắt capture cho cả 4 kênh
static void enableCaptureChannels() {
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_CC1);
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_CC2);
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_CC3);
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_CC4);
}

// Tắt ngắt capture của cả 4 kênh
static void disableCaptureChannels() {
    __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC1);
    __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC2);
    __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC3);
    __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC4);
}

// ---------- Triển khai các phương thức public ----------

void TDOAManager::init() {
    // 1. Cấu hình TIM2 cho input capture, độ phân giải ~11.9 ns (84 MHz, PSC=0)
    __HAL_RCC_TIM2_CLK_ENABLE();
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;               // Không chia, tần số timer = 84 MHz
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 0xFFFFFFFF;         // 32-bit tự do, chỉ dùng tràn
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_IC_Init(&htim2);

    // Cấu hình kênh capture (xung rising, không lọc)
    TIM_IC_InitTypeDef icConfig = {0};
    icConfig.ICPolarity = TIM_ICPOLARITY_RISING;
    icConfig.ICSelection = TIM_ICSELECTION_DIRECTTI;
    icConfig.ICPrescaler = TIM_ICPSC_DIV1;
    icConfig.ICFilter = 0;
    HAL_TIM_IC_ConfigChannel(&htim2, &icConfig, TIM_CHANNEL_1);
    HAL_TIM_IC_ConfigChannel(&htim2, &icConfig, TIM_CHANNEL_2);
    HAL_TIM_IC_ConfigChannel(&htim2, &icConfig, TIM_CHANNEL_3);
    HAL_TIM_IC_ConfigChannel(&htim2, &icConfig, TIM_CHANNEL_4);

    // Bật ngắt tràn (update) và đặt độ ưu tiên thấp hơn ngắt capture
    HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);

    // Khởi động timer
    HAL_TIM_Base_Start(&htim2);

    // 2. Cấu hình các chân điều khiển
    GPIO_InitTypeDef gpio = {0};

    // DATA_READY output (PB0)
    gpio.Pin = DATA_READY_PIN;
    gpio.Mode = GPIO_MODE_OUTPUT_PP;
    gpio.Pull = GPIO_NOPULL;
    gpio.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(DATA_READY_PORT, &gpio);
    HAL_GPIO_WritePin(DATA_READY_PORT, DATA_READY_PIN, GPIO_PIN_RESET);

    // RDC input (PB1) – sẽ dùng EXTI1
    gpio.Pin = RDC_PIN;
    gpio.Mode = GPIO_MODE_IT_RISING;        // Ngắt cạnh lên
    gpio.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(RDC_PORT, &gpio);

    // TC input (PB2) – dùng EXTI2 (cả cạnh lên để set cờ, và sau đó đọc mức)
    // Khởi tạo với ngắt cạnh lên, sau đó sẽ đọc mức bằng polling
    gpio.Pin = TC_PIN;
    gpio.Mode = GPIO_MODE_IT_RISING;
    gpio.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(TC_PORT, &gpio);

    // RS input (PB4) – EXTI4
    gpio.Pin = RS_PIN;
    gpio.Mode = GPIO_MODE_IT_RISING;
    gpio.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(RS_PORT, &gpio);

    // 3. Cấu hình NVIC cho các ngắt ngoài
    HAL_NVIC_SetPriority(EXTI1_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI1_IRQn);
    HAL_NVIC_SetPriority(EXTI2_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI2_IRQn);
    HAL_NVIC_SetPriority(EXTI4_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(EXTI4_IRQn);

    // Trạng thái ban đầu
    currentState = IDLE;
    successCount = 0;
    overflow_count = 0;
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    for (int i = 0; i < 4; ++i) channelCaptured[i] = false;
}

void TDOAManager::processEvents() {
    // Xử lý cờ RS (reset) – ưu tiên cao nhất
    if (rsFlag) {
        rsFlag = false;
        resetSystem();
        return;
    }

    // Xử lý cờ RDC khi đang ở trạng thái DATA_READY
    if (rdcFlag) {
        rdcFlag = false;
        if (currentState == DATA_READY) {
            HAL_GPIO_WritePin(DATA_READY_PORT, DATA_READY_PIN, GPIO_PIN_RESET); // kéo DR xuống
            successCount++;
            if (successCount >= MAX_SUCCESS_COUNT) {
                // Đã đủ 3 lần thành công -> dừng hẳn, về IDLE
                stopCaptureAndClear();
                currentState = IDLE;
            } else {
                // Chưa đủ 3 lần -> tiếp tục capture ngay (vì TC vẫn đang high)
                startCaptureSequence();
                currentState = CAPTURING;
            }
        }
    }

    // Xử lý trạng thái và sự kiện
    switch (currentState) {
    case IDLE:
        // Kiểm tra cờ TC (cạnh lên đã xảy ra)
        if (tcFlag) {
            tcFlag = false;
            // Bắt đầu chờ 2 giây
            delayStartTick = HAL_GetTick();
            delayActive = true;
            currentState = DELAY_2S;
        }
        break;

    case DELAY_2S:
        // Nếu TC xuống thấp trong khi chờ -> hủy
        if (HAL_GPIO_ReadPin(TC_PORT, TC_PIN) == GPIO_PIN_RESET) {
            delayActive = false;
            currentState = IDLE;
        }
        // Nếu đã đủ 2 giây và TC vẫn cao -> bắt đầu capture
        if (delayActive && (HAL_GetTick() - delayStartTick >= 2000)) {
            delayActive = false;
            startCaptureSequence();
            currentState = CAPTURING;
        }
        break;

    case CAPTURING:
        // Kiểm tra TC còn cao không, nếu thấp -> hủy capture
        if (HAL_GPIO_ReadPin(TC_PORT, TC_PIN) == GPIO_PIN_RESET) {
            stopCaptureAndClear();
            currentState = IDLE;
        }
        // Kiểm tra đã capture đủ 4 kênh chưa
        if (channelCaptured[0] && channelCaptured[1] && channelCaptured[2] && channelCaptured[3]) {
            // Đủ 4 timestamp -> đóng gói và báo DATA_READY
            packDataForSPI();
            SPISlave::setTxData((const uint8_t*)spiTxBuffer, spiTxLen);
            HAL_GPIO_WritePin(DATA_READY_PORT, DATA_READY_PIN, GPIO_PIN_SET);
            currentState = DATA_READY;
            // Tắt ngắt capture để tránh capture thêm (đợi RPI đọc xong)
            disableCaptureChannels();
        }
        break;

    case DATA_READY:
        // Ở trạng thái này chỉ chờ RDC hoặc RS (đã xử lý ở trên)
        break;
    }
}

void TDOAManager::onTC_IRQ() {
    tcFlag = true;  // Báo có cạnh lên TC
}

void TDOAManager::onRDC_IRQ() {
    rdcFlag = true; // Báo RPI đã đọc xong
}

void TDOAManager::onRS_IRQ() {
    rsFlag = true;  // Báo yêu cầu reset
}

// Các callback capture: lưu giá trị capture và overflow hiện tại, sau đó tắt ngắt kênh đó
void TDOAManager::onCaptureCH1(uint32_t val) {
    if (currentState == CAPTURING && !channelCaptured[0]) {
        uint32_t ov = overflow_count;
        // Nếu cờ tràn đang set -> tăng thêm 1 vì ngắt tràn chưa chạy
        if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE)) {
            ov++;
        }
        captures[0].overflow = ov;
        captures[0].tick = val;
        channelCaptured[0] = true;
        __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC1);   // Không cho capture tiếp
    }
}

void TDOAManager::onCaptureCH2(uint32_t val) {
    if (currentState == CAPTURING && !channelCaptured[1]) {
        uint32_t ov = overflow_count;
        if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE)) ov++;
        captures[1].overflow = ov;
        captures[1].tick = val;
        channelCaptured[1] = true;
        __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC2);
    }
}

void TDOAManager::onCaptureCH3(uint32_t val) {
    if (currentState == CAPTURING && !channelCaptured[2]) {
        uint32_t ov = overflow_count;
        if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE)) ov++;
        captures[2].overflow = ov;
        captures[2].tick = val;
        channelCaptured[2] = true;
        __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC3);
    }
}

void TDOAManager::onCaptureCH4(uint32_t val) {
    if (currentState == CAPTURING && !channelCaptured[3]) {
        uint32_t ov = overflow_count;
        if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE)) ov++;
        captures[3].overflow = ov;
        captures[3].tick = val;
        channelCaptured[3] = true;
        __HAL_TIM_DISABLE_IT(&htim2, TIM_IT_CC4);
    }
}

void TDOAManager::onOverflow() {
    overflow_count++;
}

uint64_t TDOAManager::getCurrentTimestamp() {
    uint32_t ov = overflow_count;
    uint32_t cnt = __HAL_TIM_GET_COUNTER(&htim2);
    // Đọc lại overflow_count phòng khi bị ngắt ngay sau khi đọc
    if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE)) {
        ov++;
    }
    return ((uint64_t)ov << 32) | cnt;
}

// ---------- Các hàm private ----------
void TDOAManager::packDataForSPI() {
    // Định dạng: "A , 0x%08X, 0x%08X\r\nB , ..."
    spiTxLen = snprintf(spiTxBuffer, sizeof(spiTxBuffer),
        "A , 0x%08lX, 0x%08lX\r\n"
        "B , 0x%08lX, 0x%08lX\r\n"
        "C , 0x%08lX, 0x%08lX\r\n"
        "D , 0x%08lX, 0x%08lX\r\n",
        captures[0].overflow, captures[0].tick,
        captures[1].overflow, captures[1].tick,
        captures[2].overflow, captures[2].tick,
        captures[3].overflow, captures[3].tick);
}

void TDOAManager::startCaptureSequence() {
    // Xóa cờ đã capture
    for (int i = 0; i < 4; ++i) {
        channelCaptured[i] = false;
        captures[i].overflow = 0;
        captures[i].tick = 0;
    }
    // Bật lại ngắt capture cho tất cả các kênh
    enableCaptureChannels();
}

void TDOAManager::stopCaptureAndClear() {
    disableCaptureChannels();            // Tắt ngắt capture
    for (int i = 0; i < 4; ++i) {
        channelCaptured[i] = false;
    }
}

void TDOAManager::resetSystem() {
    // Tắt mọi capture, xóa dữ liệu, đưa về IDLE
    disableCaptureChannels();
    for (int i = 0; i < 4; ++i) {
        channelCaptured[i] = false;
        captures[i].overflow = 0;
        captures[i].tick = 0;
    }
    successCount = 0;
    overflow_count = 0;
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    HAL_GPIO_WritePin(DATA_READY_PORT, DATA_READY_PIN, GPIO_PIN_RESET);
    delayActive = false;
    currentState = IDLE;
    tcFlag = false;
    rdcFlag = false;
    rsFlag = false;
}

// ============================================================================
// CÁC TRÌNH PHỤC VỤ NGẮT (ISR)
// ============================================================================

// Ngắt TIM2: xử lý capture và overflow
extern "C" void TIM2_IRQHandler(void) {
    // Capture kênh 1
    if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_CC1) != RESET) {
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC1);
        uint32_t cap = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_1);
        TDOAManager::onCaptureCH1(cap);
    }
    // Capture kênh 2
    if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_CC2) != RESET) {
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC2);
        uint32_t cap = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_2);
        TDOAManager::onCaptureCH2(cap);
    }
    // Capture kênh 3
    if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_CC3) != RESET) {
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC3);
        uint32_t cap = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_3);
        TDOAManager::onCaptureCH3(cap);
    }
    // Capture kênh 4
    if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_CC4) != RESET) {
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_CC4);
        uint32_t cap = HAL_TIM_ReadCapturedValue(&htim2, TIM_CHANNEL_4);
        TDOAManager::onCaptureCH4(cap);
    }
    // Tràn update
    if (__HAL_TIM_GET_FLAG(&htim2, TIM_FLAG_UPDATE) != RESET) {
        __HAL_TIM_CLEAR_FLAG(&htim2, TIM_FLAG_UPDATE);
        TDOAManager::onOverflow();
    }
}

// Ngắt ngoài PB1 (RDC)
extern "C" void EXTI1_IRQHandler(void) {
    if (__HAL_GPIO_EXTI_GET_IT(RDC_PIN) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(RDC_PIN);
        TDOAManager::onRDC_IRQ();
    }
}

// Ngắt ngoài PB2 (TC)
extern "C" void EXTI2_IRQHandler(void) {
    if (__HAL_GPIO_EXTI_GET_IT(TC_PIN) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(TC_PIN);
        TDOAManager::onTC_IRQ();
    }
}

// Ngắt ngoài PB4 (RS)
extern "C" void EXTI4_IRQHandler(void) {
    if (__HAL_GPIO_EXTI_GET_IT(RS_PIN) != RESET) {
        __HAL_GPIO_EXTI_CLEAR_IT(RS_PIN);
        TDOAManager::onRS_IRQ();
    }
}
