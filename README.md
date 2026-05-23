```markdown
# Hệ thống TDOA Quân sự (HTTDTD v3.1)

Dự án xác định tọa độ điểm va chạm trên bia bắn sử dụng nguyên lý TDOA (Time Difference of Arrival) của sóng âm thanh (N-Wave). Hệ thống bao gồm ba thành phần chính:

- **STM32F407VET6** – thu thập timestamp từ 4 cảm biến piezoelectric với độ phân giải ~11.9 ns.
- **Raspberry Pi Zero 2W (Node)** – nhận lệnh, tính toán tọa độ bằng thuật toán Chan + Levenberg‑Marquardt, gửi kết quả qua LoRa.
- **Controller (Linux PC)** – giao diện Qt5 điều khiển, hiển thị bảng điểm, quản lý trạng thái 20 node.

Toàn bộ hệ thống được viết bằng **C++**, sử dụng công cụ ARM GNU Toolchain cho STM32, g++ cho Raspberry Pi và Controller.
```

---

## Cấu trúc thư mục

```

HTTDTD_v3.1/
├── STM32F407VET6/          # Firmware cho MCU
│   ├── Core/
│   │   ├── Inc/            # Header: system.h, tdoa_manager.hpp, ...
│   │   ├── Src/            # Source: main.cpp, tdoa_manager.cpp, ...
│   │   └── Startup/        # startup_stm32f407xx.s
│   ├── Drivers/            # HAL, CMSIS
│   ├── Makefile
│   └── STM32F407VETx_FLASH.ld
├── tdoa_node/              # Phần mềm cho Raspberry Pi Node
│   ├── src/                # main.cpp, NodeController, LoRaModule, SPI, TDOASolver
│   ├── Makefile
│   └── CMakeLists.txt (tùy chọn)
├── controller/             # Giao diện điều khiển
│   ├── main.cpp, MainWindow, UdpGateway, NodeManager, ScoreCalculator, Logger
│   └── Makefile
├── Makefile                # Makefile tổng (gọi make từng thư mục)
└── README.md

```

---

## Yêu cầu phần cứng & kết nối

### STM32F407VET6
- 4 cảm biến piezoelectric → op‑amp + comparator → digital 3.3V.
- TIM2 các kênh: CH1 = PA0 (A), CH2 = PA1 (B), CH3 = PA2 (C), CH4 = PA3 (D).
- BME280 qua SPI2 (NSS PB12, SCK PB13, MISO PB14, MOSI PB15).
- SPI3 (slave) kết nối với Raspberry Pi: NSS PA15, SCK PC10, MISO PC11, MOSI PC12.
- Chân điều khiển từ Raspberry Pi:
  - PB0 (DATA_READY) → output báo sẵn sàng.
  - PB1 (RDC) → input nhận xác nhận đã đọc.
  - PB2 (TC) → input kích hoạt trigger.
  - PB4 (RS) → input reset.
- USART1 (PA9/PA10) dành cho debug/log.

### Raspberry Pi Zero 2W (mỗi Node)
- SPI0: CE0 (GPIO 8), MOSI (GPIO 10), MISO (GPIO 9), SCLK (GPIO 11).
- UART (GPIO 14/15) kết nối module LoRa SX1276.
- GPIO 21 → MAIN_MOTOR (motor quay).
- GPIO 17 → FORCE_TRIGGER (nối PB2 của STM).
- GPIO 19 → RECEIVED_COMPLETE (nối PB1).
- GPIO 20 → FORCE_RESET (nối PB4).
- GPIO 22 → HARD_RESET (nối NRST của STM).
- GPIO 18 → LOAD_DATA (nối PB0).

### Controller
- PC Linux (Parrot OS) có giao diện Qt5.
- Kết nối UDP với LoRa packet forwarder (sx1303) qua localhost:1680 (uplink) và :1780 (downlink).

---

## Phần mềm và thư viện yêu cầu

### Chung
- **STM32**: `arm-none-eabi-g++` (ARM GNU Toolchain ≥ 10.3), newlib.
- **Raspberry Pi Node**: `g++` (≥ 8), `liblgpio-dev`, `libeigen3-dev` (cho TDOASolver).
- **Controller**: Qt5 (`qt5-default`, `libqt5widgets5`, `libqt5network5`), `libeigen3-dev` (nếu dùng TDOASolver).

Cài đặt nhanh trên Ubuntu/Debian:
```bash
# Toolchain ARM (cài thủ công từ ARM hoặc apt)
sudo apt install gcc-arm-none-eabi libnewlib-dev

# Raspberry Pi (trên chính Pi hoặc cross‑compile)
sudo apt install g++ liblgpio-dev libeigen3-dev

# Controller (trên PC Linux)
sudo apt install qt5-default libqt5widgets5 libqt5network5 libeigen3-dev
```

---

Build & Triển khai

Build toàn bộ

```bash
cd HTTDTD_v3.1
make          # Build tất cả (STM32, Node, Controller)
make clean    # Dọn sạch
```

Build riêng từng phần

· STM32:
  ```bash
  cd STM32F407VET6
  make clean && make
  ```
  File build/tdoa_f407.bin sẽ được tạo, nạp bằng ST‑Link: make flash (nếu có st-flash) hoặc dùng OpenOCD.
· Raspberry Pi Node:
  ```bash
  cd tdoa_node
  make clean && make
  ```
  Chạy: ./node <ID> (ví dụ: ./node 1A).
· Controller:
  ```bash
  cd controller
  make clean && make
  ./controller
  ```

---

Giao thức & Lệnh

Lệnh điều khiển (Controller → Node)

Lệnh Ý nghĩa
NODE_<col><row> , UP Kích hoạt một node cụ thể
NODE_<col> , UP UP tất cả node trong một cột
NODE_<row> , UP UP tất cả node trong một hàng
NODE_<col><row> , DOWN Hủy kích hoạt
NODE_<col><row> , HR Hard reset node đó
MARKING , UP Đánh dấu (khóa các lệnh khác)
MARKING , DOWN Kết thúc đánh dấu
STATUS Yêu cầu tất cả node gửi trạng thái

Dữ liệu uplink (Node → Controller)

· Toạ độ: NODE_<col><row>, <x>, <y> (x, y đơn vị cm).
· Trạng thái: NODE_<col><row>, STATUS, <pin%>, <trạng thái>, <nhiệt độ>, <kết nối>.

Luồng hoạt động chính

1. Controller gửi lệnh UP đến node.
2. Node bật motor, sau 10s kích trigger vào STM32.
3. STM32 thu 4 timestamp, đóng gói và kéo chân DATA_READY.
4. Node đọc timestamp qua SPI, tính toán vị trí bằng TDOA (Chan + LM).
5. Node gửi kết quả về Controller qua LoRa, đồng thời gửi xung Received Complete.
6. Sau 3 lần hoặc 60s, node dừng, reset STM32 và motor.

---

Thuật toán TDOA & Tính điểm

· TDOA solver: nhận 4 timestamp (đếm timer 32-bit + overflow, độ phân giải ~11.9 ns), tính chênh lệch thời gian so với kênh A. Dùng vận tốc âm thanh hiệu chỉnh theo nhiệt độ BME280. Giải hệ Chan để khởi tạo, sau đó tối ưu bằng Levenberg‑Marquardt (thư viện Eigen).
· Vị trí sensor (tính bằng cm, gốc O tại tâm bia):
  · A: (-50, -50)
  · B: (-50,  50)
  · C: ( 50,  50)
  · D: ( 50, -50)
· Tính điểm: khoảng cách r = sqrt(x²+y²). Các vòng tròn bán kính:
  · Bulls‑eye: 1.25 cm (10 điểm), sau đó mỗi vòng cộng 3.5 cm (9,8,…,4 điểm). Ngoài vòng 4 điểm (r > 22.25 cm) = 0 điểm.
· Xếp loại: Tổng điểm 3 lần bắn:
  · ≥ 27: Giỏi
  · 23‑26: Khá
  · 18‑22: Tốt
  · 15‑17: Đạt
  · < 15: Trượt

---

Giao diện Controller (GUI Qt5)

· Bảng điểm: 20 dòng (1A→5D) × 14 cột (Node, Lần 1‑3: điểm và toạ độ, Tổng điểm, Trung bình, Xếp loại). Tự động điền khi nhận dữ liệu.
· Trạng thái node: hiển thị pin%, trạng thái (ACTIVATED/DEACTIVATED/MARKING/WARN), nhiệt độ, kết nối.
· Nút điều khiển: các nút cột/hàng, nút riêng từng node, MARKING, STATUS, CLEAR (kèm auto‑clear).
· Debug log: cửa sổ text hiển thị các sự kiện với màu sắc (INFO/WARN/SEND/RECV).

---

Ghi chú khi phát triển

· Toàn bộ mã nguồn được chú thích bằng tiếng Việt, mô tả rõ chức năng.
· Khi thay đổi cấu hình (tần số STM32, SF LoRa, vị trí sensor), chỉ cần sửa file Config.hpp (Node) hoặc system.h (STM32).
· Node sử dụng lgpio cho GPIO, không dùng pigpio daemon. Có thể cross‑compile từ PC hoặc build trực tiếp trên Raspberry Pi.

---

Liên hệ

Dự án được phát triển cho mục đích quân sự. Mọi thông tin chi tiết xin liên hệ đơn vị quản lý.

Phiên bản: 3.1
Tác giả: Tổ phát triển HTTDTD
Ngày hoàn thiện: 05/2026

```