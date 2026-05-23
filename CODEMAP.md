# HTTDTD_v3.1 - Code Map (Bản đồ mã nguồn)

**Phiên bản:** 3.1  
**Ngày cập nhật:** 05/2026  
**Mục đích:** Hệ thống xác định tọa độ va chạm sử dụng TDOA (Time Difference of Arrival) và tính điểm tự động

---

## 📋 Tổng quan kiến trúc

Dự án bao gồm **3 thành phần chính** chạy trên các nền tảng khác nhau:

```
┌─────────────────────────────────────────────────────────────┐
│  Controller (Qt5 GUI)                                        │
│  - Giao diện người dùng (20 node × 14 cột)                  │
│  - Quản lý trạng thái, điều khiển node                      │
│  - Tính điểm, xếp loại                                       │
└──────────────────────┬──────────────────────────────────────┘
                       │ UDP (LoRa gateway)
                       ↓
┌─────────────────────────────────────────────────────────────┐
│  TDOA Node (Raspberry Pi Zero 2W) ×20                        │
│  - Nhận lệnh từ Controller                                   │
│  - Đọc timestamp qua SPI từ STM32                            │
│  - Tính toán vị trí (TDOA Solver)                            │
│  - Gửi kết quả qua LoRa                                      │
└──────────────────────┬──────────────────────────────────────┘
                       │ SPI (slave)
                       ↓
┌─────────────────────────────────────────────────────────────┐
│  STM32F407VET6 Microcontroller ×20                           │
│  - Thu timestamp từ 4 cảm biến piezo (TIM2)                 │
│  - Đọc cảm biến nhiệt độ/áp suất (BME280)                   │
│  - Gửi dữ liệu qua SPI slave                                 │
└─────────────────────────────────────────────────────────────┘
```

---

## 📁 Cấu trúc thư mục chi tiết

### 1. **STM32F407VET6/** - Firmware cho Vi điều khiển
Chứa toàn bộ mã nguồn chạy trên STM32F407VET6 (viết bằng C/C++, dùng ARM GNU Toolchain).

#### **STM32F407VET6/Core/Inc/** - Header Files
| File | Chức năng |
|------|----------|
| **system.h** | Cấu hình hệ thống, định nghĩa GPIO, định thời |
| **tdoa_manager.hpp** | Quản lý quá trình thu timestamp, xử lý timer |
| **spi_slave.hpp** | Giao tiếp SPI slave với Raspberry Pi |
| **bme280.hpp** | Driver cảm biến BME280 (I2C/SPI) |
| **temperature_logger.hpp** | Ghi log nhiệt độ, áp suất |
| **stm32f4xx_hal_conf.h** | Cấu hình STM32 HAL |

#### **STM32F407VET6/Core/Src/** - Implementation
| File | Chức năng |
|------|----------|
| **main.cpp** | Entry point, khởi tạo hệ thống, vòng lặp chính |
| **tdoa_manager.cpp** | Logic quản lý: bắt đầu timer, lưu timestamp, đóng gói dữ liệu |
| **spi_slave.cpp** | Xử lý SPI slave DMA, gửi buffer dữ liệu |
| **bme280.cpp** | Đọc nhiệt độ/áp suất, hiệu chuẩn |
| **temperature_logger.cpp** | Log dữ liệu nhiệt độ |
| **system_stm32f4xx.c** | System clock configuration |

#### **STM32F407VET6/Drivers/** - HAL & CMSIS
- **STM32F4xx_HAL_Driver/** - STMicroelectronics HAL Driver
- **CMSIS/** - ARM CMSIS Core, Device, NN

#### **STM32F407VET6/** - Build Configuration
- **Makefile** - Build script cho STM32
- **STM32F407VETx_FLASH.ld** - Linker script (Flash memory layout)

---

### 2. **tdoa_node/** - Phần mềm Raspberry Pi Node
Xử lý logic chính: nhận lệnh, tính toán, gửi kết quả (viết bằng C++).

#### **tdoa_node/src/** - Source Files
| File | Chức năng |
|------|----------|
| **main.cpp** | Entry point: khởi tạo Node ID, vòng lặp chính |
| **Config.hpp** | Các hằng số: GPIO, tần số, vị trí sensor, tham số TDOA |
| **NodeController.hpp/.cpp** | Quản lý trạng thái node, xử lý lệnh từ Controller |
| **SPIMaster.hpp/.cpp** | Giao tiếp SPI master với STM32 (đọc timestamp) |
| **LoRaModule.hpp/.cpp** | Gửi/nhận dữ liệu LoRa (SX1276 via UART) |
| **TDOASolver.hpp/.cpp** | Tính toán vị trí: Chan's method + Levenberg-Marquardt |

#### **tdoa_node/** - Build
- **Makefile** - Build script cho Node
- **CMakeLists.txt** (tùy chọn) - CMake configuration

#### **Logic luồng Node**
```
main()
  ├─ khởi tạo GPIO, SPI, LoRa
  └─ while(true):
      ├─ chờ lệnh từ LoRa (Controller)
      ├─ bật motor, đợi trigger
      ├─ kích hoạt STM32 (GPIO PB2)
      ├─ đọc timestamp qua SPI
      ├─ tính tọa độ (TDOA Solver)
      ├─ gửi kết quả qua LoRa
      └─ reset, dừng motor
```

---

### 3. **controller/** - Giao diện điều khiển (Qt5)
Ứng dụng GUI điều khiển và hiển thị kết quả, chạy trên Linux PC.

#### **controller/** - Source Files
| File | Chức năng |
|------|----------|
| **main.cpp** | Entry point: khởi tạo ứng dụng Qt, MainWindow |
| **MainWindow.hpp/.cpp** | Cửa sổ chính: UI bảng điểm, nút điều khiển, log debug |
| **UdpGateway.hpp/.cpp** | Giao tiếp UDP với LoRa gateway (localhost:1680, :1780) |
| **NodeManager.hpp/.cpp** | Quản lý 20 node: lưu trạng thái, pin%, nhiệt độ |
| **ScoreCalculator.hpp/.cpp** | Tính điểm dựa khoảng cách, xếp loại |
| **Logger.hpp/.cpp** | Log các sự kiện với mã màu (INFO/WARN/SEND/RECV) |

#### **UI Elements**
- **Bảng điểm**: 20 dòng (1A-5D) × 14 cột (Node, Lần 1-3, Tổng, Trung bình, Xếp loại)
- **Bảng trạng thái**: Pin%, trạng thái node, nhiệt độ, kết nối
- **Nút điều khiển**: Col/Row, Node cá nhân, MARKING, STATUS, CLEAR
- **Debug Log**: Cửa sổ text với timestamp

#### **controller/** - Build
- **Makefile** - Build script (sử dụng Qt5)

---

### 4. **Root Level Files**
| File | Chức năng |
|------|----------|
| **Makefile** | Master Makefile gọi make trong từng thư mục |
| **README.md** | Tài liệu dự án (hướng dẫn, cấu hình, giao thức) |
| **LICENSE** | Giấy phép Apache 2.0 |
| **LICENSE_vi.md** | Giấy phép tiếng Việt |

---

## 🔌 Giao thức & Kết nối

### STM32F407VET6 - Kết nối I/O
```
Timer 2 (Timestamp):
  - PA0 (TIM2_CH1) ← Cảm biến A
  - PA1 (TIM2_CH2) ← Cảm biến B
  - PA2 (TIM2_CH3) ← Cảm biến C
  - PA3 (TIM2_CH4) ← Cảm biến D

SPI2 (Master - BME280):
  - PB12 (NSS), PB13 (SCK), PB14 (MISO), PB15 (MOSI)

SPI3 (Slave - Raspberry Pi):
  - PA15 (NSS), PC10 (SCK), PC11 (MISO), PC12 (MOSI)

Điều khiển GPIO từ Raspberry Pi:
  - PB0 (DATA_READY) → output
  - PB1 (RDC) → input (đã đọc)
  - PB2 (TC) → input (trigger)
  - PB4 (RS) → input (reset)
  - NRST → Hard reset

USART1 (Debug):
  - PA9/PA10 (TX/RX)
```

### Raspberry Pi Zero 2W - GPIO Mapping
```
SPI0: CE0=GPIO8, MOSI=GPIO10, MISO=GPIO9, SCLK=GPIO11
UART: GPIO14/15 → SX1276 LoRa
GPIO18 → LOAD_DATA (PB0)
GPIO19 → RECEIVED_COMPLETE (PB1)
GPIO20 → FORCE_RESET (PB4)
GPIO21 → MAIN_MOTOR
GPIO22 → HARD_RESET (NRST)
GPIO17 → FORCE_TRIGGER (PB2)
```

### Controller - Network
```
UDP Client
  - Uplink: localhost:1680 (nhận kết quả từ LoRa gateway)
  - Downlink: localhost:1780 (gửi lệnh đến LoRa gateway)
```

---

## 📊 Lưu luồng dữ liệu

### Downlink (Controller → Node)
```
Controller (Qt5)
  └─ UDP → LoRa Gateway (sx1303)
      └─ LoRa Downlink
          └─ Node (Raspberry Pi)
              └─ Logic xử lý lệnh (NodeController.cpp)
```

### Uplink (Node → Controller)
```
Node (STM32 + Raspberry Pi)
  ├─ SPI: STM32 → RPi (timestamp 4 kênh)
  ├─ TDOASolver: tính (x, y)
  └─ LoRa Uplink
      └─ LoRa Gateway
          └─ UDP → Controller
              └─ MainWindow (hiển thị bảng điểm)
```

---

## 🧮 Thuật toán cốt lõi

### TDOA Solver (tdoa_node/src/TDOASolver.cpp)
```
Input:  4 timestamp từ STM32 (độ phân giải ~11.9 ns)
        Vị trí 4 cảm biến: A(-50,-50), B(-50,50), C(50,50), D(50,-50)
        Vận tốc âm thanh (hiệu chỉnh theo nhiệt độ)

Process:
  1. Tính chênh lệch thời gian: ΔtB = tB - tA, ΔtC, ΔtD
  2. Chuyển sang khoảng cách chênh lệch: ΔrB = ΔtB × c, ...
  3. Dùng phương pháp Chan khởi tạo
  4. Tối ưu bằng Levenberg-Marquardt

Output: (x, y) toạ độ va chạm
```

### Tính điểm (controller/ScoreCalculator.cpp)
```
r = sqrt(x² + y²)

Điểm dựa trên bán kính:
  - r ≤ 1.25 cm   → 10 điểm (bulls-eye)
  - r ≤ 4.75 cm   → 9 điểm
  - r ≤ 8.25 cm   → 8 điểm
  - r ≤ 11.75 cm  → 7 điểm
  - r ≤ 15.25 cm  → 6 điểm
  - r ≤ 18.75 cm  → 5 điểm
  - r ≤ 22.25 cm  → 4 điểm
  - r > 22.25 cm  → 0 điểm

Xếp loại (Tổng 3 lần):
  - ≥ 27: Giỏi
  - 23-26: Khá
  - 18-22: Tốt
  - 15-17: Đạt
  - < 15: Trượt
```

---

## 🔧 Build & Compile

### Build toàn bộ
```bash
cd HTTDTD_v3.1
make clean && make
```

### Build riêng từng phần
```bash
# STM32 (sản phẩm: build/tdoa_f407.bin)
cd STM32F407VET6
make clean && make

# Raspberry Pi Node
cd tdoa_node
make clean && make
./node 1A  # Chạy với ID "1A"

# Controller
cd controller
make clean && make
./controller  # Chạy GUI Qt5
```

---

## 📚 Thư viện & Dependency

| Thành phần | Thư viện yêu cầu |
|-----------|-----------------|
| **STM32** | arm-none-eabi-g++, STM32 HAL, CMSIS, newlib |
| **RPi Node** | g++, liblgpio-dev, libeigen3-dev |
| **Controller** | Qt5 (libqt5widgets5, libqt5network5), libeigen3-dev |

---

## 📌 Các file quan trọng (Entry Points)

| Thành phần | Entry Point | Chức năng |
|-----------|-------------|----------|
| **STM32** | `STM32F407VET6/Core/Src/main.cpp` | Khởi tạo, loop chính |
| **RPi Node** | `tdoa_node/src/main.cpp` | Khởi tạo Node, loop lệnh |
| **Controller** | `controller/main.cpp` | Khởi tạo Qt Application |
| **GUI** | `controller/MainWindow.hpp/.cpp` | Cửa sổ chính Qt5 |

---

## 🔄 Class Diagram (Mối quan hệ)

```
STM32F407VET6:
  ├─ TimerManager (quản lý TIM2)
  ├─ SPI_Slave (giao tiếp SPI slave)
  ├─ BME280_Driver (cảm biến)
  └─ TemperatureLogger

TDOA_Node:
  ├─ NodeController
  │  ├─ SPIMaster
  │  ├─ LoRaModule
  │  └─ TDOASolver
  └─ Config

Controller:
  ├─ MainWindow (QMainWindow)
  │  ├─ UdpGateway
  │  ├─ NodeManager
  │  ├─ ScoreCalculator
  │  └─ Logger
```

---

## 💾 Cấu hình chính (Config Files)

| File | Nội dung |
|------|---------|
| `tdoa_node/src/Config.hpp` | GPIO pins, tần số LoRa, tham số TDOA |
| `STM32F407VET6/Core/Inc/system.h` | Clock, timer frequency, SPI settings |
| `STM32F407VET6/Core/Inc/stm32f4xx_hal_conf.h` | HAL configuration |

---

## 🚀 Workflow chính hệ thống

```
1. [Controller] Gửi lệnh UP (NODE_1A_UP) qua UDP
2. [LoRa] Truyền downlink
3. [Node] Nhận lệnh, bật motor
4. [Node] Sau 10s, kích trigger vào STM32
5. [STM32] Thu 4 timestamp qua TIM2
6. [STM32] Kéo chân DATA_READY (PB0)
7. [Node] Đọc timestamp qua SPI
8. [Node] TDOASolver tính (x, y)
9. [Node] Gửi kết quả qua LoRa Uplink
10. [Controller] Nhận, hiển thị bảng điểm
11. [Node] Reset, dừng motor
```

---

## 📖 Ghi chú phát triển

- ✅ Toàn bộ code được chú thích bằng **tiếng Việt**
- ✅ Sử dụng **C++** cho logic cao cấp, **C** cho HAL
- ✅ Cross-compile từ PC hoặc build trực tiếp trên Raspberry Pi
- ✅ Config tập trung trong `Config.hpp` (tiếp cận dễ dàng)
- ✅ Eigen library dùng cho xử lý ma trận (TDOA Solver)
