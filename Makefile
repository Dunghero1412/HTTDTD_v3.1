# ============================================================================
# Makefile tổng cho toàn bộ hệ thống TDOA
# Gọi make trong từng thư mục con.
# ============================================================================

.PHONY: all clean flash stm32 node controller

# Mặc định build tất cả
all: stm32 node controller

# Build riêng từng thành phần
stm32:
	@echo "===== Building STM32 Firmware ====="
	$(MAKE) -C STM32F407VET6

node:
	@echo "===== Building Raspberry Pi Node ====="
	$(MAKE) -C tdoa_node

controller:
	@echo "===== Building Controller ====="
	$(MAKE) -C controller

# Dọn dẹp tất cả
clean:
	@echo "===== Cleaning all ====="
	$(MAKE) -C STM32F407VET6 clean
	$(MAKE) -C tdoa_node clean
	$(MAKE) -C controller clean

# Nạp firmware STM32 (nếu có target 'flash' trong Makefile của STM32)
flash: stm32
	@echo "===== Flashing STM32 ====="
	$(MAKE) -C STM32F407VET6 flash