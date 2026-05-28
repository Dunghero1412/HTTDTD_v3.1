# Hệ thống TDOA Quân sự (HTTDTD v3.1)

<div align="center">

[![License: APACHE2.0](https://img.shields.io/badge/License-APACHE2.0-yellow.svg)](https://opensource.org/licenses/Apache2.0)
[![Python](https://img.shields.io/badge/C++-pass-green.svg)](https://cpp.org)
[![Platform](https://img.shields.io/badge/Platform-RPi5%20%7C%20RPi%20Zero%202W%20%7C%20STM32F407VET6-red.svg)]()

**Phát triển bởi [Chiêm Dũng](https://github.com/Dunghero1412)**

</div>

**Hệ thống xác định tọa độ điểm va chạm trên bia bắn sử dụng nguyên lý TDOA (Time Difference of Arrival)**

> ⚠️ **LƯU Ý QUAN TRỌNG:** Xem [DISCLAIMER.md](DISCLAIMER.md) và [SAFETY_WARNING.md](SAFETY_WARNING.md) trước khi sử dụng hệ thống.

---

## 📋 Mục lục

- [Tổng quan](#tổng-quan)
- [Kiến trúc hệ thống](#kiến-trúc-hệ-thống)
- [Cấu trúc thư mục](#cấu-trúc-thư-mục)
- [Yêu cầu phần cứng & kết nối](#yêu-cầu-phần-cứng--kết-nối)
- [Phần mềm và thư viện](#phần-mềm-và-thư-viện)
- [Build & Triển khai](#build--triển-khai)
- [Giao thức & Lệnh điều khiển](#giao-thức--lệnh-điều-khiển)
- [Thuật toán TDOA & Tính điểm](#thuật-toán-tdoa--tính-điểm)
- [Giao diện Controller (GUI Qt5)](#giao-diện-controller-gui-qt5)
- [Ghi chú phát triển](#ghi-chú-phát-triển)
- [Tài liệu bổ sung](#tài-liệu-bổ-sung)
- [Liên hệ](#liên-hệ)

---

## 🎯 Tổng quan

Hệ thống HTTDTD v3.1 là một giải pháp công nghệ cao dành cho huấn luyện quân sự, sử dụng phương pháp **TDOA (Time Difference of Arrival)** để xác định toạ độ điểm va chạm trên bia bắn một cách tự động và chính xác.

### Đặc điểm chính:
- ✅ **Độ phân giải thời gian cao:** ~11.9 ns (STM32F407VET6)
- ✅ **Hệ thống phân tán 20 node:** Điều khiển qua giao tiếp LoRa tần số 915 MHz
- ✅ **Thuật toán TDOA nâng cao:** Phương pháp Chan + Levenberg-Marquardt
- ✅ **Tính điểm tự động:** Xếp loại theo chuẩn quân sự (Giỏi, Khá, Tốt, Đạt, Trượt)
- ✅ **Giao diện GUI Qt5:** Hiển thị bảng điểm, trạng thái 20 node, log debug real-time
- ✅ **Viết toàn bộ bằng C++:** Hiệu suất cao, dễ bảo trì

### Ứng dụng:
- Huấn luyện xạ thủ (bắn quân sự)
- Đánh giá kỹ thuật bắn
- Quản lý và chấm điểm tự động
- Hỗ trợ huấn luyện tác chiến

---

## 🏗️ Kiến trúc hệ thống

```
┌─────────────────────────────────────────────────────────────────────┐
│  Controller (Qt5 GUI) - Linux PC (Parrot OS)                        │
│  • Giao diện người dùng: 20 node × 14 cột                          │
│  • Quản lý trạng thái, điều khiển node, tính điểm, xếp loại       │
│  • Kết nối UDP với LoRa gateway (localhost:1680/1780)              │
└────────────────────────────┬────────────────────────────────────────┘
                             │ UDP (LoRa gateway)
                             ▼
┌─────────────────────────────────────────────────────────────────────┐
│  Mạng LoRa (SX1276) - Tần số 915 MHz                                │
│  • Downlink: Controller → 20 Node                                   │
│  • Uplink: 20 Node → Controller                                     │
└────────────────────────────┬────────────────────────────────────────┘
                             │ LoRa Uplink/Downlink
                             ▼
┌─────────────────────────────────────────────────────────────────────┐
│  TDOA Node (Raspberry Pi Zero 2W) ×20 - ID: 1A~5D                  │
│  • Nhận lệnh từ Controller qua LoRa                                 │
│  • Đọc 4 timestamp từ STM32 qua SPI master                          │
│  • Tính toán tọa độ (x, y) bằng TDOA Solver                         │
│  • Gửi kết quả về Controller qua LoRa uplink                        │
└────────────────────────────┬────────────────────────────────────────┘
                             │ SPI (slave)
                             ▼
┌─────────────────────────────────────────────────────────────────────┐
│  STM32F407VET6 Microcontroller ×20                                  │
│  • Thu 4 timestamp từ cảm biến piezoelectric qua Timer2             │
│  • Đọc cảm biến BME280 (nhiệt độ, áp suất) qua SPI master         │
│  • Gửi dữ liệu qua SPI slave để Node đọc                           │
│  • Độ phân giải: ~11.9 ns (Timer 2 chạy ở 84 MHz)                  │
└─────────────────────────────────────────────────────────────────────┘
```

---

## 📁 Cấu trúc thư mục

```
HTTDTD_v3.1/
├── STM32F407VET6/                 # Firmware cho MCU
│   ├── Core/
│   │   ├── Inc/                   # Header files
│   │   │   ├── system.h           # Cấu hình GPIO, timer
│   │   │   ├── tdoa_manager.hpp   # Quản lý TDOA, thu timestamp
│   │   │   ├── spi_slave.hpp      # Giao tiếp SPI slave
│   │   │   ├── bme280.hpp         # Driver cảm biến BME280
│   │   │   └── temperature_logger.hpp  # Ghi log nhiệt độ
│   │   ├── Src/                   # Source files
│   │   │   ├── main.cpp           # Entry point, vòng lặp chính
│   │   │   ├── tdoa_manager.cpp   # Logic TDOA
│   │   │   ├── spi_slave.cpp      # Xử lý SPI slave DMA
│   │   │   ├── bme280.cpp         # Đọc cảm biến
│   │   │   └── temperature_logger.cpp
│   │   └── Startup/
│   │       └── startup_stm32f407xx.s
│   ├── Drivers/                   # HAL, CMSIS
│   │   ├── STM32F4xx_HAL_Driver/
│   │   └── CMSIS/
│   ├── Makefile                   # Build script
│   └── STM32F407VETx_FLASH.ld     # Linker script
│
├── tdoa_node/                     # Phần mềm Raspberry Pi Node
│   ├── src/
│   │   ├── main.cpp               # Entry point Node
│   │   ├── Config.hpp             # Các hằng số GPIO, SPI, timing
│   │   ├── NodeController.hpp/.cpp # Quản lý trạng thái node
│   │   ├── SPIMaster.hpp/.cpp     # Giao tiếp SPI master STM32
│   │   ├── LoRaModule.hpp/.cpp    # Gửi/nhận LoRa SX1276
│   │   └── TDOASolver.hpp/.cpp    # Tính toán TDOA (Chan + LM)
│   ├── Makefile
│   └── CMakeLists.txt (tùy chọn)
│
├── controller/                    # Giao diện điều khiển Qt5
│   ├── main.cpp
│   ├── MainWindow.hpp/.cpp        # Cửa sổ chính, bảng điểm
│   ├── UdpGateway.hpp/.cpp        # Giao tiếp UDP LoRa gateway
│   ├── NodeManager.hpp/.cpp       # Quản lý 20 node
│   ├── ScoreCalculator.hpp/.cpp   # Tính điểm, xếp loại
│   ├── Logger.hpp/.cpp            # Log debug với màu sắc
│   └── Makefile
│
├── Makefile                       # Makefile tổng (gọi make từng thư mục)
├── README.md                      # Tài liệu này
├── CODEMAP.md                     # Bản đồ mã nguồn chi tiết
├── DISCLAIMER.md                  # Tuyên bố miễn trừ trách nhiệm
├── SAFETY_WARNING.md              # Cảnh báo an toàn
├── OPENRATION_LIMITATIONS.md      # Giới hạn vận hành
├── LICENSE                        # Giấy phép Apache 2.0
└── LICENSE_vi.md                  # Giấy phép tiếng Việt

```

---

## ⚙️ Yêu cầu phần cứng & kết nối

### STM32F407VET6

#### Pin Timer 2 (Cảm biến Piezoelectric)
```
TIM2_CH1 (PA0)  ← Cảm biến A
TIM2_CH2 (PA1)  ← Cảm biến B
TIM2_CH3 (PA2)  ← Cảm biến C
TIM2_CH4 (PA3)  ← Cảm biến D
```

Tín hiệu từ cảm biến: Op-amp → Comparator → Digital 3.3V

#### SPI2 (Master) - Cảm biến BME280
```
NSS (PB12)   → Chip Select
SCK (PB13)   → Clock
MISO (PB14)  → Data In
MOSI (PB15)  → Data Out
```

#### SPI3 (Slave) - Kết nối Raspberry Pi
```
NSS (PA15)   → Chip Select
SCK (PC10)   → Clock
MISO (PC11)  → Data In (from STM32)
MOSI (PC12)  → Data Out (to STM32)
```

#### Chân điều khiển từ Raspberry Pi
```
PB0 (DATA_READY)  → Output báo dữ liệu sẵn sàng
PB1 (RDC)         → Input nhận xác nhận đã đọc dữ liệu
PB2 (TC)          → Input kích hoạt trigger (điều chỉnh từ GPIO 17 RPi)
PB4 (RS)          → Input reset (điều chỉnh từ GPIO 20 RPi)
NRST              → Hard reset (điều chỉnh từ GPIO 22 RPi)
```

#### USART1 (Debug)
```
PA9  → TX (truyền)
PA10 → RX (nhận)
Baud rate: 115200
```

### Raspberry Pi Zero 2W (Mỗi Node)

#### SPI0 (Master với STM32)
```
GPIO 8  → CE0  (Chip Enable)
GPIO 10 → MOSI (Data to STM32)
GPIO 9  → MISO (Data from STM32)
GPIO 11 → SCLK (Clock)
Speed: 1 MHz
```

#### UART LoRa (GPIO 14/15 hoặc /dev/serial0)
```
GPIO 14 → TX → SX1276 RX
GPIO 15 → RX ← SX1276 TX
Baud rate: 115200
```

#### GPIO Điều khiển
```
GPIO 21 → MAIN_MOTOR         (Motor quay bia)
GPIO 17 → FORCE_TRIGGER      (Kích hoạt trigger, nối PB2 STM)
GPIO 19 → RECEIVED_COMPLETE  (Xác nhận đã đọc, nối PB1 STM)
GPIO 20 → FORCE_RESET        (Reset STM32, nối PB4)
GPIO 22 → HARD_RESET         (Hard reset, nối NRST STM)
GPIO 18 → LOAD_DATA          (Input báo dữ liệu sẵn sàng, từ PB0 STM)
```

### Controller (PC Linux)

- **OS:** Linux (Ubuntu, Debian, Parrot OS)
- **Giao tiếp UDP:** 
  - **Uplink:** `localhost:1680` (nhận kết quả từ LoRa gateway)
  - **Downlink:** `localhost:1780` (gửi lệnh đến LoRa gateway)
- **LoRa Gateway:** sx1303 (hoặc tương thích)
- **Tần số:** 915 MHz
- **Spreading Factor:** SF7 (cột 1) ~ SF11 (cột 5)

---

## 💻 Phần mềm và thư viện

### Công cụ biên dịch

| Thành phần | Công cụ | Phiên bản |
|-----------|---------|----------|
| STM32 | arm-none-eabi-g++ | ≥ 10.3 |
| Raspberry Pi | g++ | ≥ 8 |
| Controller | g++ | ≥ 8 |

### Thư viện yêu cầu

#### STM32
```bash
arm-none-eabi-g++    # ARM GNU Toolchain
libnewlib-dev        # C Standard Library
STM32 HAL            # (Có sẵn trong repo)
CMSIS                # (Có sẵn trong repo)
```

#### Raspberry Pi Node
```bash
g++                  # GCC C++ Compiler
liblgpio-dev         # GPIO library (thay thế pigpio)
libeigen3-dev        # Linear algebra (TDOA Solver)
```

#### Controller (Qt5 GUI)
```bash
g++                  # GCC C++ Compiler
qt5-default          # Qt5 Framework
libqt5widgets5       # Qt5 Widgets
libqt5network5       # Qt5 Network (UDP)
libeigen3-dev        # Linear algebra
pkg-config           # Package configuration
```

### Cài đặt nhanh (Ubuntu/Debian)

```bash
# STM32 Toolchain
sudo apt update
sudo apt install gcc-arm-none-eabi libnewlib-dev

# Raspberry Pi (trên chính Pi)
sudo apt install g++ liblgpio-dev libeigen3-dev

# Controller (trên Linux PC)
sudo apt install g++ qt5-default libqt5widgets5 libqt5network5 libeigen3-dev pkg-config

# Công cụ phát triển bổ sung (tùy chọn)
sudo apt install git cmake build-essential
```

---

## 🔨 Build & Triển khai

### Build toàn bộ hệ thống

```bash
cd HTTDTD_v3.1
make clean    # Xóa các file build cũ
make          # Build tất cả (STM32, Node, Controller)
```

### Build riêng từng phần

#### 1. STM32F407VET6 Firmware

```bash
cd STM32F407VET6

# Build
make clean && make

# Kết quả:
# - build/tdoa_f407.elf  (ELF executable)
# - build/tdoa_f407.hex  (Intel HEX format)
# - build/tdoa_f407.bin  (Binary format)

# Nạp firmware qua ST-Link
make flash

# Debug với GDB
make debug

# Hiển thị thông tin build
make info

# Kiểm tra toolchain
make check-tools
```

**Lưu ý:** Cần cài đặt `st-flash` hoặc sử dụng OpenOCD/STM32CubeProgrammer để nạp firmware.

#### 2. Raspberry Pi Node

```bash
cd tdoa_node

# Cross-compile từ PC Linux
make clean && make

# Hoặc build trực tiếp trên Raspberry Pi
cd tdoa_node && make clean && make

# Chạy node (thay đổi ID tùy vị trí)
sudo ./node 1A    # Node cột 1, hàng A
sudo ./node 2B    # Node cột 2, hàng B
sudo ./node 5D    # Node cột 5, hàng D
```

**Lưu ý:** Cần quyền `root` (sudo) để truy cập GPIO và SPI.

#### 3. Controller (GUI Qt5)

```bash
cd controller

# Build
make clean && make

# Chạy
./controller
```

**Lưu ý:** Cần có kết nối UDP đến LoRa gateway (mặc định `localhost:1680` và `localhost:1780`).

---

## 📡 Giao thức & Lệnh điều khiển

### Lệnh từ Controller → Node

Định dạng: `NODE_<col><row>,<CMD>` (ví dụ: `NODE_1A,UP`)

| Lệnh | Ý nghĩa |
|------|---------|
| `NODE_<col><row>,UP` | Kích hoạt một node cụ thể (bật motor, chuẩn bị bắn) |
| `NODE_<col>,UP` | Kích hoạt tất cả node trong một cột |
| `NODE_<row>,UP` | Kích hoạt tất cả node trong một hàng |
| `NODE_<col><row>,DOWN` | Hủy kích hoạt (tắt motor) |
| `NODE_<col><row>,HR` | Hard reset (khởi động lại node) |
| `MARKING,UP` | Bắt đầu chế độ đánh dấu (khóa các lệnh khác) |
| `MARKING,DOWN` | Kết thúc chế độ đánh dấu |
| `STATUS` | Yêu cầu tất cả node gửi trạng thái hiện tại |

### Dữ liệu Uplink từ Node → Controller

#### 1. Kết quả tọa độ
```
NODE_<col><row>,<x>,<y>[,<temp>]
```
- `<col><row>`: Node ID (ví dụ: 1A, 2C)
- `<x>`: Toạ độ X (cm)
- `<y>`: Toạ độ Y (cm)
- `<temp>`: Nhiệt độ (°C, tùy chọn)

#### 2. Trạng thái Node
```
NODE_<col><row>,STATUS,<pin%>,<state>,<temp>,<rssi>
```
- `<pin%>`: Mức pin (0-100%)
- `<state>`: ACTIVATED | DEACTIVATED | MARKING | WARN
- `<temp>`: Nhiệt độ (°C)
- `<rssi>`: Cường độ tín hiệu LoRa (dBm)

### Luồng hoạt động chính

```
1. [Controller] Gửi lệnh UP đến NODE_1A qua UDP
                   ↓
2. [LoRa Gateway] Chuyển từ UDP → LoRa downlink
                   ↓
3. [Node 1A] Nhận lệnh, bật motor quay
                   ↓
4. [Node 1A] Đợi 10 giây, sau đó kích trigger vào STM32 (GPIO 17)
                   ↓
5. [STM32] Nhận trigger (PB2), bắt đầu capture timestamp từ Timer 2
           Ghi lại 4 timestamp từ cảm biến A, B, C, D
                   ↓
6. [STM32] Kéo chân DATA_READY (PB0) high → báo Node dữ liệu sẵn sàng
                   ↓
7. [Node] Nhận cảnh báo (GPIO 18), đọc timestamp qua SPI master
          Ghi lại RSSI LoRa, pin%
                   ↓
8. [Node] TDOASolver: tính chênh lệch thời gian (ΔtB, ΔtC, ΔtD)
          → TDOA Localization (Chan + Levenberg-Marquardt)
          → Kết quả: (x, y) toạ độ điểm va chạm
                   ↓
9. [Node] Gửi xung RECEIVED_COMPLETE (GPIO 19) → PB1 STM32
          Gửi kết quả "(x, y, nhiệt độ)" qua LoRa uplink
                   ↓
10. [LoRa Gateway] Chuyển từ LoRa → UDP (localhost:1680)
                   ↓
11. [Controller] Nhận kết quả, tính điểm, hiển thị bảng điểm
                   ↓
12. [Node] Lặp lại (tối đa 3 lần hoặc 60 giây)
           Sau đó: reset STM32, tắt motor, chờ lệnh mới
```

---

## 📊 Thuật toán TDOA & Tính điểm

### TDOA Solver (tdoa_node/src/TDOASolver.cpp)

#### Input
- **4 timestamp** từ STM32 (độ phân giải ~11.9 ns)
- **Vị trí 4 cảm biến** (cm):
  - A: (-50, -50)
  - B: (-50,  50)
  - C: ( 50,  50)
  - D: ( 50, -50)
- **Vận tốc âm thanh** (hiệu chỉnh theo nhiệt độ)

#### Quá trình tính toán
```
1. Tính chênh lệch thời gian (TDOA):
   ΔtB = tB - tA
   ΔtC = tC - tA
   ΔtD = tD - tA

2. Chuyển sang chênh lệch khoảng cách:
   ΔrB = ΔtB × c  (c = vận tốc âm thanh)
   ΔrC = ΔtC × c
   ΔrD = ΔtD × c

3. Phương pháp Chan (khởi tạo):
   Giải hệ phương trình tuyến tính ước lượng vị trí gần đúng

4. Tối ưu bằng Levenberg-Marquardt:
   Minimize: Σ(ri - r_measured)²
   Kết quả cuối cùng: (x, y) toạ độ va chạm
```

#### Output
- **(x, y)** - Toạ độ điểm va chạm (đơn vị: cm)

### Tính điểm (controller/ScoreCalculator.cpp)

#### Công thức
```
r = sqrt(x² + y²)   // Khoảng cách từ tâm bia
```

#### Bảng điểm theo bán kính

| Bán kính (cm) | Điểm | Vòng |
|---------------|------|------|
| r ≤ 1.25 | **10** | Bulls-eye |
| 1.25 < r ≤ 4.75 | **9** | Vòng 1 |
| 4.75 < r ≤ 8.25 | **8** | Vòng 2 |
| 8.25 < r ≤ 11.75 | **7** | Vòng 3 |
| 11.75 < r ≤ 15.25 | **6** | Vòng 4 |
| 15.25 < r ≤ 18.75 | **5** | Vòng 5 |
| 18.75 < r ≤ 22.25 | **4** | Vòng 6 |
| r > 22.25 | **0** | Ngoài bia |

#### Xếp loại (Tổng 3 lần bắn)

| Tổng điểm | Xếp loại |
|----------|---------|
| ≥ 27 | 🥇 **Giỏi** |
| 23-26 | 🥈 **Khá** |
| 18-22 | 🥉 **Tốt** |
| 15-17 | ✅ **Đạt** |
| < 15 | ❌ **Trượt** |

---

## 🖥️ Giao diện Controller (GUI Qt5)

### Bảng điểm (Score Table)

**Layout:** 20 dòng (Node 1A~5D) × 14 cột

| Cột | Nội dung |
|-----|---------|
| 1 | Node ID (1A, 1B, ..., 5D) |
| 2 | Điểm lần 1 |
| 3 | Toạ độ lần 1 (x, y) |
| 4 | Điểm lần 2 |
| 5 | Toạ độ lần 2 |
| 6 | Điểm lần 3 |
| 7 | Toạ độ lần 3 |
| 8 | Tổng điểm |
| 9 | Trung bình |
| 10 | Xếp loại |
| 11-14 | Thông tin bổ sung |

**Cập nhật:** Tự động khi nhận dữ liệu từ Node.

### Bảng trạng thái (Status Panel)

Hiển thị cho mỗi node:
- **Pin%** - Mức pin (0-100%)
- **Trạng thái** - ACTIVATED | DEACTIVATED | MARKING | WARN
- **Nhiệt độ** - °C (từ cảm biến BME280)
- **Kết nối** - RSSI LoRa (dBm)

### Nút điều khiển (Control Buttons)

| Nút | Chức năng |
|-----|----------|
| **COL** (1-5) | Kích hoạt toàn bộ cột |
| **ROW** (A-D) | Kích hoạt toàn bộ hàng |
| **NODE** (cá nhân) | Kích hoạt node cụ thể |
| **MARKING** | Bắt đầu/kết thúc chế độ đánh dấu |
| **STATUS** | Yêu cầu trạng thái tất cả node |
| **CLEAR** | Xóa bảng điểm (kèm auto-clear) |
| **REFRESH** | Cập nhật giao diện |

### Debug Log

- **Cửa sổ text** hiển thị tất cả sự kiện
- **Mã màu:**
  - 🟦 **[INFO]** - Thông tin thường
  - 🟨 **[WARN]** - Cảnh báo
  - 🟩 **[SEND]** - Gửi dữ liệu
  - 🟧 **[RECV]** - Nhận dữ liệu
  - 🟥 **[ERROR]** - Lỗi
- **Timestamp** tự động ghi lại thời gian sự kiện

---

## 📝 Ghi chú phát triển

### Cấu hình chính (Dễ thay đổi)

1. **Config Node** (`tdoa_node/src/Config.hpp`)
   ```cpp
   // GPIO pins, SPI speed, LoRa baudrate, timing
   constexpr int GPIO_MAIN_MOTOR = 21;
   constexpr int SPI_SPEED = 1000000;  // 1 MHz
   constexpr int LORA_BAUD = 115200;
   constexpr uint32_t TRIGGER_DELAY = 10000;  // 10 giây
   constexpr int SF_BY_COL[] = {7, 8, 9, 10, 11};  // SF7-SF11
   ```

2. **Config STM32** (`STM32F407VET6/Core/Inc/system.h`)
   ```c
   #define TIM2_CH1_PIN GPIO_PIN_0   // PA0
   #define SPI3_MOSI_PIN GPIO_PIN_12 // PC12
   #define TEMP_UPDATE_INTERVAL_MS 30000  // 30 giây
   ```

3. **Vị trí sensor** (Config.hpp)
   ```cpp
   const SensorPos SENSORS[4] = {
       {-50, -50}, // A
       {-50,  50}, // B
       { 50,  50}, // C
       { 50, -50}  // D
   };
   ```

### Chú thích mã nguồn

- ✅ Toàn bộ mã được chú thích **bằng tiếng Việt**
- ✅ Sử dụng **C++17** cho Node và Controller
- ✅ Sử dụng **C++11** cho STM32 (tránh rtti, exceptions)
- ✅ **Header comments** mô tả rõ chức năng từng file

### Quy trình sửa đổi hệ thống

**⚠️ QUAN TRỌNG:** Mọi thay đổi phần cứng, firmware, timer, prescaler, comparator threshold, hoặc vị trí cảm biến **PHẢI**:
1. Kiểm thử lại toàn bộ hệ thống
2. Hiệu chuẩn lại độ chính xác
3. Đánh giá an toàn đầy đủ
4. Ghi lại các thay đổi

Xem chi tiết tại [DISCLAIMER.md](DISCLAIMER.md) và [SAFETY_WARNING.md](SAFETY_WARNING.md).

### Cross-compile

- **Từ Linux PC:** `arm-none-eabi-g++` cho STM32
- **Từ Linux PC:** `g++` (native) cho Node (cross-compile tùy chọn)
- **Trên Raspberry Pi:** `g++` (native)

---

## 📚 Tài liệu bổ sung

| Tài liệu | Nội dung |
|----------|---------|
| [CODEMAP.md](CODEMAP.md) | Bản đồ mã nguồn chi tiết, kiến trúc từng module |
| [DISCLAIMER.md](DISCLAIMER.md) | Tuyên bố miễn trừ trách nhiệm, giới hạn hệ thống |
| [SAFETY_WARNING.md](SAFETY_WARNING.md) | Cảnh báo an toàn thao trường, quy tắc vận hành |
| [OPENRATION_LIMITATIONS.md](OPENRATION_LIMITATIONS.md) | Giới hạn vận hành, điều kiện môi trường |
| [LICENSE](LICENSE) | Giấy phép Apache 2.0 (tiếng Anh) |
| [LICENSE_vi.md](LICENSE_vi.md) | Giấy phép Apache 2.0 (tiếng Việt) |

---

## 📞 Liên hệ

### Thông tin dự án

- **Tên dự án:** Hệ thống TDOA Quân sự (HTTDTD)
- **Phiên bản:** 3.1
- **Ngày hoàn thiện:** 05/2026
- **Mục đích:** Xác định tọa độ điểm va chạm tự động dành cho huấn luyện quân sự
- **Địa chỉ kho:** https://github.com/Dunghero1412/HTTDTD_v3.1

### Thông tin liên hệ

| Mục đích | Liên hệ |
|---------|--------|
| **Báo cáo lỗi (Bug Report)** | [Issues](https://github.com/Dunghero1412/HTTDTD_v3.1/issues) |
| **Đề xuất cải tiến** | [Discussions](https://github.com/Dunghero1412/HTTDTD_v3.1/discussions) |
| **Pull Request** | [Create PR](https://github.com/Dunghero1412/HTTDTD_v3.1/pulls) |
| **GitHub Account** | [@Dunghero1412](https://github.com/Dunghero1412) |

### Nhóm phát triển

- **Tác giả chính:** Tổ phát triển HTTDTD
- **Mục đích:** Phát triển cho nhu cầu quân sự (Bộ Quốc phòng Việt Nam)
- **Chế độ:** Dự án mã nguồn mở (Apache 2.0)

### Hỗ trợ kỹ thuật

**Liên hệ qua:**
1. **GitHub Issues** - Báo cáo lỗi, hỏi đáp kỹ thuật
2. **GitHub Discussions** - Thảo luận về tính năng, kiến trúc
3. **Email** - (Yêu cầu thêm từ tác giả)

### Cảnh báo bảo mật

⚠️ **Hệ thống này là nền tảng thử nghiệm phục vụ mục đích quân sự.** 

- **Không sử dụng** cho các mục đích ngoài quân sự không được phép
- **Không phép phân phối** cho các tổ chức, cá nhân không được ủy quyền
- **Không được sửa đổi** firmware mà không thông báo đơn vị quản lý
- Xem [DISCLAIMER.md](DISCLAIMER.md) để biết thêm chi tiết

---

## 🔒 Giấy phép

Dự án này được phát hành dưới **Giấy phép Apache 2.0**.

- Xem [LICENSE](LICENSE) (tiếng Anh)
- Xem [LICENSE_vi.md](LICENSE_vi.md) (tiếng Việt)

---

**Phiên bản README:** 3.1  
**Cập nhật lần cuối:** 05/2026

✅ **Dự án hoàn tất và sẵn sàng triển khai quân sự.**
