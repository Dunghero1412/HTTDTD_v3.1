# ============================================================================
# Makefile cho dự án TDOA quân sự - STM32F407VET6
# Biên dịch bằng arm-none-eabi-gcc/g++ (C++11)
# Tương thích: Unix/Linux/macOS
# ============================================================================

# ---------- Shell mặc định ----------
SHELL = /bin/bash

# ---------- Thư mục dự án ----------
PROJECT_DIR := $(shell pwd)
BUILD_DIR   := $(PROJECT_DIR)/build

# ---------- Tên file đầu ra ----------
TARGET := tdoa_f407

# ---------- Công cụ biên dịch (tìm tự động hoặc chỉ định đường dẫn) ----------
# Nếu chưa có trong PATH, hãy sửa thành đường dẫn tuyệt đối:
# CROSS_COMPILE := /usr/bin/arm-none-eabi-
CROSS_COMPILE := arm-none-eabi-

CC       := $(CROSS_COMPILE)gcc
CXX      := $(CROSS_COMPILE)g++
AS       := $(CROSS_COMPILE)gcc -x assembler-with-cpp
OBJCOPY  := $(CROSS_COMPILE)objcopy
OBJDUMP  := $(CROSS_COMPILE)objdump
SIZE     := $(CROSS_COMPILE)size
GDB      := $(CROSS_COMPILE)gdb

# ---------- CPU và FPU ----------
CPU       := -mcpu=cortex-m4
FPU       := -mfpu=fpv4-sp-d16
FLOAT_ABI := -mfloat-abi=hard
MCU       := $(CPU) -mthumb $(FPU) $(FLOAT_ABI)

# Định danh MCU (bắt buộc cho CMSIS)
MCU_DEFINES = -DSTM32F407xx -DSTM32F4XX -DUSE_HAL_DRIVER

# ---------- Các cờ biên dịch chung ----------
# -Og: tối ưu cho debug, -g: thêm debug symbols
OPTIMIZE  := -Og
DEBUG     := -g

COMMON_FLAGS := -Wall -Wextra -Wno-unused-parameter $(OPTIMIZE) $(DEBUG)

# Đường dẫn include - DÙNG := ĐỂ MỞ RỘNG NGAY
INCLUDES := \
    -I$(PROJECT_DIR)/Core/Inc \
    -I$(PROJECT_DIR)/Drivers/STM32F4xx_HAL_Driver/Inc \
    -I$(PROJECT_DIR)/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy \
    -I$(PROJECT_DIR)/Drivers/CMSIS/Device/ST/STM32F4xx/Include \
    -I$(PROJECT_DIR)/Drivers/CMSIS/Include

# ---------- Cờ cho C, C++ và Assembler ----------
CFLAGS   := $(MCU) $(COMMON_FLAGS) $(MCU_DEFINES) $(INCLUDES) -std=gnu11 -fdata-sections -ffunction-sections
CXXFLAGS := $(MCU) $(COMMON_FLAGS) $(MCU_DEFINES) $(INCLUDES) -std=c++11 -fno-rtti -fno-exceptions -fdata-sections -ffunction-sections  -isystem $(PROJECT_DIR)/Drivers/CMSIS/Include
#CXXFLAGS += --specs=nosys.specs
ASFLAGS  := $(MCU) $(MCU_DEFINES) $(INCLUDES) -c

# ---------- Cờ liên kết ----------
LDSCRIPT := $(PROJECT_DIR)/STM32F407VETx_FLASH.ld
LIBS     := -lc -lm -lnosys -lgcc
LDFLAGS  := $(MCU)   -T$(LDSCRIPT) $(LIBS) \
            -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref \
            -Wl,--gc-sections 

# ---------- Các file nguồn ----------
# C++ source (ứng dụng chính)
CPP_SOURCES := \
    Core/Src/main.cpp \
    Core/Src/tdoa_manager.cpp \
    Core/Src/spi_slave.cpp \
    Core/Src/bme280.cpp \
    Core/Src/temperature_logger.cpp \
    Core/Src/DebugUART.cpp \

# C source (HAL, CMSIS)
C_SOURCES := \
    Core/Src/system_stm32f4xx.c \
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c \
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c \
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c \
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c \
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c \
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c \
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c \
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c \
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c \
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c \
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c \
    Drivers/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c \
    Core/Src/system_stm32f4xx.c

# ASM source (startup)
ASM_SOURCES := Core/Startup/startup_stm32f407xx.s

# ---------- Tạo danh sách object từ tên file ----------
# Sử dụng notdir để lấy tên file, rồi đổi đuôi thành .o
CPP_OBJS := $(addprefix $(BUILD_DIR)/,$(notdir $(CPP_SOURCES:.cpp=.o)))
C_OBJS   := $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
ASM_OBJS := $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))

# Tất cả các object
OBJS := $(CPP_OBJS) $(C_OBJS) $(ASM_OBJS)

# File đầu ra
ELF := $(BUILD_DIR)/$(TARGET).elf
HEX := $(BUILD_DIR)/$(TARGET).hex
BIN := $(BUILD_DIR)/$(TARGET).bin

# ---------- VPATH để tìm file nguồn trong các thư mục ----------
VPATH := $(PROJECT_DIR)/Core/Src
VPATH += $(PROJECT_DIR)/Drivers/STM32F4xx_HAL_Driver/Src
VPATH += $(PROJECT_DIR)/Core/Startup

# ---------- Mục tiêu mặc định ----------
.PHONY: all clean flash debug

all: $(ELF) $(HEX) $(BIN)
	@echo "========================================="
	@echo "Build hoàn tất: $(ELF)"
	@echo "========================================="
	@$(SIZE) $(ELF)

# ---------- Liên kết ----------
$(ELF): $(OBJS) | $(BUILD_DIR)
	@echo "🔗 Liên kết $@..."
	$(CXX) $(LDFLAGS) -o $@ $^
	@echo "✅ Liên kết thành công"

# ---------- Tạo file HEX ----------
$(HEX): $(ELF) | $(BUILD_DIR)
	@echo "📝 Tạo file HEX..."
	$(OBJCOPY) -O ihex $< $@
	@echo "✅ HEX: $@"

# ---------- Tạo file BIN ----------
$(BIN): $(ELF) | $(BUILD_DIR)
	@echo "📦 Tạo file BIN..."
	$(OBJCOPY) -O binary $< $@
	@echo "✅ BIN: $@"

# ---------- Biên dịch C++ ----------
$(BUILD_DIR)/%.o: %.cpp
	@echo "🔨 [C++] $(notdir $<)"
	@mkdir -p $(BUILD_DIR)
	$(CXX) -c $(CXXFLAGS) $< -o $@

# ---------- Biên dịch C ----------
$(BUILD_DIR)/%.o: %.c
	@echo "🔨 [C]   $(notdir $<)"
	@mkdir -p $(BUILD_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

# ---------- Biên dịch Assembly ----------
$(BUILD_DIR)/%.o: %.s
	@echo "🔨 [ASM] $(notdir $<)"
	@mkdir -p $(BUILD_DIR)
	$(AS) -c $(ASFLAGS) $< -o $@

# ---------- Tạo thư mục build nếu chưa tồn tại ----------
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# ---------- Dọn dẹp ----------
clean:
	@echo "🧹 Dọn dẹp thư mục build..."
	@rm -rf $(BUILD_DIR)
	@echo "✅ Đã xóa $(BUILD_DIR)"

# ---------- Nạp chương trình qua ST-Link ----------
flash: $(BIN)
	@echo "⚡ Đang nạp firmware qua ST-Link..."
	st-flash write $< 0x08000000
	@echo "✅ Nạp thành công"

# ---------- Debug với GDB ----------
debug: $(ELF)
	@echo "🐛 Khởi động GDB server..."
	$(GDB) -ex "target extended-remote :4242" \
	       -ex "monitor reset halt" \
	       -ex "load" \
	       -ex "monitor reset init" \
	       $(ELF)

# ---------- Hiển thị thông tin build ----------
info:
	@echo "========================================="
	@echo "Thông tin dự án TDOA"
	@echo "========================================="
	@echo "MCU:           STM32F407VET6"
	@echo "Cross compile: $(CROSS_COMPILE)"
	@echo "Project dir:   $(PROJECT_DIR)"
	@echo "Build dir:     $(BUILD_DIR)"
	@echo "Tối ưu hóa:    $(OPTIMIZE)"
	@echo "-----------------------------------------"
	@echo "Số file CPP:   $(words $(CPP_SOURCES))"
	@echo "Số file C:     $(words $(C_SOURCES))"
	@echo "Số file ASM:   $(words $(ASM_SOURCES))"
	@echo "-----------------------------------------"
	@echo "Tổng object:   $(words $(OBJS))"
	@echo "========================================="

# ---------- Kiểm tra công cụ biên dịch ----------
check-tools:
	@echo "🔍 Kiểm tra công cụ biên dịch..."
	@which $(CC) || echo "❌ Không tìm thấy $(CC). Hãy cài đặt: sudo apt install gcc-arm-none-eabi"
	@which $(CXX) || echo "❌ Không tìm thấy $(CXX)"
	@which $(OBJCOPY) || echo "❌ Không tìm thấy $(OBJCOPY)"
	@echo "✅ Kiểm tra hoàn tất"

# ---------- In danh sách file nguồn ----------
list-sources:
	@echo "=== C++ Sources ==="
	@for f in $(CPP_SOURCES); do echo "  $$f"; done
	@echo "=== C Sources ==="
	@for f in $(C_SOURCES); do echo "  $$f"; done
	@echo "=== ASM Sources ==="
	@for f in $(ASM_SOURCES); do echo "  $$f"; done

# ---------- Help ----------
help:
	@echo "========================================="
	@echo "Makefile cho dự án TDOA STM32F407"
	@echo "========================================="
	@echo "Các lệnh khả dụng:"
	@echo "  make           - Build firmware (ELF, HEX, BIN)"
	@echo "  make clean     - Xóa thư mục build"
	@echo "  make flash     - Nạp firmware qua ST-Link"
	@echo "  make debug     - Debug với GDB"
	@echo "  make info      - Hiển thị thông tin build"
	@echo "  make check-tools - Kiểm tra toolchain"
	@echo "  make list-sources - In danh sách file nguồn"
	@echo "  make help      - Hiển thị trợ giúp này"
	@echo "========================================="
