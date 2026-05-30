
```markdown
# Hệ Thống HTTDTD v3.1 - Sơ Đồ Kiến Trúc (Mermaid)

## 1. Kiến Trúc Tổng Thể Hệ Thống

```mermaid
graph TB
    subgraph "Thao Trường (Physical)"
        TARGET["🎯 Bia Mục Tiêu"]
        SENSORS["📡 4 Cảm Biến Piezoelectric<br/>Vị trí: A, B, C, D"]
        BULLET["💥 Đạn Thật<br/>(Impact Event)"]
    end

    subgraph "Nút Phân Tán (Distributed Nodes) - Raspberry Pi x 5"
        NODE["🖥️ NODE 1-5<br/>(Row A-D)<br/>Total: 20 Nodes"]
        
        subgraph "Mỗi Node"
            direction LR
            STM32["STM32F407VET6<br/>(Timestamp Capture)"]
            BME280["🌡️ BME280<br/>(Temperature/Pressure)"]
            LORA["📶 LoRa Module<br/>(SX1276)"]
            TDOA_CALC["⚙️ TDOASolver<br/>(Chan + LM)"]
            
            STM32 -->|"UART/SPI"| LORA
            BME280 -->|"I2C/SPI"| STM32
            STM32 -->|"Timestamps (168MHz)"| TDOA_CALC
            TDOA_CALC -->|"(x,y) Result"| LORA
        end
    end

    subgraph "Gateway & Controller (Raspberry Pi - Central)"
        GATEWAY["📡 UDP Gateway<br/>(LoRa → UDP)"]
        CONTROLLER["🎮 HTTDTD Controller<br/>(Qt Application)"]
        
        subgraph "Controller Components"
            direction LR
            NODEMAN["NodeManager<br/>(State & Data)"]
            UDPGW["UdpGateway<br/>(Network I/O)"]
            SCORECALC["ScoreCalculator<br/>(Scoring Logic)"]
            LOGGER["Logger<br/>(Debug Output)"]
            SAFETYDIA["SafetyDialog<br/>(Startup Check)"]
        end
    end

    subgraph "User Interface"
        UI["🖥️ Qt GUI<br/>(MainWindow)"]
        
        subgraph "UI Components"
            SCOREBOARD["📊 Scoreboard<br/>(20 Nodes)"]
            STATUSBAR["📈 Status Bar<br/>(Battery, Temp, Conn)"]
            BUTTONS["🔘 Control Buttons<br/>(Node, Col, Row, Marking, Clear)"]
            DEBUGLOG["📝 Debug Log<br/>(Real-time Output)"]
        end
    end

    BULLET -->|"Impact Sound Wave"| SENSORS
    SENSORS -->|"Analog Signal"| STM32
    NODE -->|"LoRa Packet<br/>(x,y,Battery,Temp)"| GATEWAY
    GATEWAY -->|"UDP Port 1680/1780"| CONTROLLER
    CONTROLLER -->|"Commands"| NODE
    CONTROLLER -->|"Update UI"| UI
    UI -->|"Display Results"| SCOREBOARD
    CONTROLLER -->|"Log Events"| DEBUGLOG
    CONTROLLER -->|"Query Status"| NODEMAN
```

---

## 2. Luồng Dữ Liệu Quá Trình Bắn (Firing Process)

```mermaid
sequenceDiagram
    participant User as 👤 Người Vận Hành
    participant UI as 🖥️ Controller UI
    participant NodeMgr as 📦 NodeManager
    participant Gateway as 📡 Gateway (UDP)
    participant Node as 🖥️ Node (RPi)
    participant STM32 as ⚡ STM32F407
    participant Sensor as 📡 Piezo Sensor
    participant TDoA as ⚙️ TDOASolver

    User->>UI: 1. Ấn nút UP cho Node 1A
    UI->>NodeMgr: sendUpDown("1A", true)
    NodeMgr->>Gateway: Gửi lệnh UP (LoRa)
    Gateway->>Node: Nhận lệnh (UDP → LoRa)
    
    Note over Node: Motor bắt đầu quay<br/>(70 giây)
    
    User->>UI: 2. Chờ kích hoạt
    Node->>STM32: Khởi tạo Timer 168MHz
    
    Note over Sensor: 💥 ĐẠN VỀ BIA
    Sensor->>STM32: Tín hiệu Piezo (mV)
    STM32->>STM32: Comparator detect → Trigger
    STM32->>STM32: Capture timestamp[A,B,C,D]<br/>(4 timestamps)
    
    Note over STM32: Tính TDOA:<br/>tB-tA, tC-tA, tD-tA<br/>Đơn vị: 1/168MHz = 5.95ns
    
    STM32->>STM32: Đọc BME280 → Nhiệt độ
    STM32->>TDoA: timestamps[4] + T
    TDoA->>TDoA: 1. Tính vận tốc âm<br/>v = 331.5 + 0.607*T (m/s)
    TDoA->>TDoA: 2. Chan method<br/>(Linear init)
    TDoA->>TDoA: 3. Levenberg-Marquardt<br/>(Non-linear optimize)
    TDoA->>TDoA: 4. Return (x,y) [cm]
    
    STM32->>Node: Gửi dữ liệu (x,y,Battery,Temp)
    Node->>Gateway: LoRa packet
    Gateway->>NodeMgr: UDP packet
    
    NodeMgr->>NodeMgr: Tính điểm<br/>(Score = distance_from_center)
    NodeMgr->>UI: Emit scoreboardUpdated()
    
    UI->>UI: Cập nhật bảng điểm
    UI->>DEBUGLOG: Log kết quả
    
    User->>UI: 3. Xem kết quả<br/>(x,y, điểm)
```

---

## 3. Kiến Trúc Firmware STM32F407

```mermaid
graph TB
    subgraph "STM32F407VET6 Firmware"
        direction TB
        
        MAIN["🔴 main()<br/>Startup"]
        SYSCLOCK["⏰ System Clock<br/>168MHz PLL"]
        GPIO["🔌 GPIO Config<br/>PA0-PA7: Analog IN"]
        I2C["🔗 I2C1<br/>BME280"]
        SPI["📡 SPI1<br/>LoRa Module"]
        TIMER["⏱️ TIM2 Timer<br/>168MHz"]
        ADC["🔌 ADC1<br/>4 Channel"]
        COMP["🔍 Comparator"]
        DMA["📦 DMA"]
        UART["📡 UART1<br/>Debug"]
        
        MAIN -->|"Init"| SYSCLOCK
        SYSCLOCK -->|"168MHz"| TIMER
        SYSCLOCK -->|"Config"| GPIO
        GPIO -->|"Enable"| ADC
        GPIO -->|"Enable"| I2C
        GPIO -->|"Enable"| SPI
        GPIO -->|"Enable"| COMP
        GPIO -->|"Enable"| UART
        
        ADC -->|"Continuous"| COMP
        COMP -->|"Rising Edge"| TIMER
        TIMER -->|"Capture"| DMA
        DMA -->|"Store"| MAIN
        
        I2C -->|"Read Temp"| MAIN
        MAIN -->|"Send"| UART
    end
    
    subgraph "Data Processing"
        RAWDATA["Raw Timestamps<br/>4x 32-bit<br/>Unit: 1/168MHz"]
        TDOA["TDOA Calculation"]
        SPEED["Speed of Sound<br/>v = 331.5 + 0.607*T"]
        POSITION["Position x,y<br/>Chan + LM"]
    end
    
    DMA -->|"4 TS"| RAWDATA
    MAIN -->|"Temp"| SPEED
    RAWDATA -->|"Δt"| TDOA
    TDOA -->|"+ Speed"| POSITION
    POSITION -->|"Send SPI"| MAIN
    
    style MAIN fill:#ff6b6b
    style TIMER fill:#4ecdc4
    style COMP fill:#95e1d3
    style DMA fill:#f38181
```

---

## 4. Luồng Chi Tiết TDOASolver

```mermaid
graph LR
    INPUT["📥 Input:<br/>timestamps[4]<br/>temperature°C"]
    
    TEMP["🌡️ Temperature<br/>Compensation<br/>v = 331.5 + 0.607*T<br/>m/s → cm/s"]
    
    TDOA_CAL["🔢 TDOA Calc<br/>tA = ts[0]/168e6<br/>tB = ts[1]/168e6<br/>TDOA[i]=t[i+1]-tA"]
    
    CHAN["📐 Chan Method<br/>Initial Guess<br/>2x2 Linear System"]
    
    INIT_POS["✓ Initial Position<br/>x0, y0<br/>from Chan"]
    
    JACOBIAN["🔤 Jacobian Matrix<br/>dF/dx, dF/dy"]
    
    LM["⚙️ Levenberg-Marquardt<br/>Optimize<br/>Minimize residual"]
    
    OUTPUT["📤 Output:<br/>x_final, y_final<br/>cm"]
    
    INPUT -->|"T: 25°C"| TEMP
    INPUT -->|"168MHz"| TDOA_CAL
    
    TEMP -->|"v cm/s"| TDOA_CAL
    TDOA_CAL -->|"Δt sec"| CHAN
    CHAN -->|"Solve 2x2"| INIT_POS
    
    INIT_POS -->|"x0,y0"| LM
    TDOA_CAL -->|"TDOA"| JACOBIAN
    JACOBIAN -->|"J matrix"| LM
    
    LM -->|"Iterate"| OUTPUT
    
    style INPUT fill:#e1f5ff
    style OUTPUT fill:#e1f5ff
    style TEMP fill:#fff9c4
    style CHAN fill:#f3e5f5
    style LM fill:#fce4ec
```

---

## 5. Luồng Khởi Động (Startup Sequence)

```mermaid
sequenceDiagram
    participant System as System
    participant App as QApplication
    participant Main as main()
    participant SafetyDia as SafetyDialog
    participant MainWin as MainWindow
    participant Gateway as UdpGateway
    participant NodeMgr as NodeManager

    System->>App: 🚀 Launch
    App->>Main: Create App
    Main->>SafetyDia: 1. Show Safety Dialog
    
    Note over SafetyDia: ⚠️ STARTUP CHECK
    SafetyDia->>SafetyDia: Load 3 files:<br/>DISCLAIMER.md<br/>SAFETY_WARNING.md<br/>OPERATION_LIMITATIONS.md
    
    SafetyDia->>SafetyDia: Display + Wait
    SafetyDia->>SafetyDia: Check:<br/>1. Scroll to bottom<br/>2. Wait 30s
    
    alt User OK
        SafetyDia->>Main: ✅ Accepted
        Main->>MainWin: 2. Create MainWindow
        MainWin->>Gateway: Init UDP
        Gateway->>Gateway: Bind 1680,1780
        Gateway->>NodeMgr: Connect
        NodeMgr->>NodeMgr: Load 20 nodes
        MainWin->>MainWin: Setup UI
        MainWin->>App: show()
        App->>App: exec()
    else User Reject
        SafetyDia->>Main: ❌ Rejected
        Main->>App: return 0
        App->>System: Close
    end
```

---

## 6. NodeManager - Quản Lý Trạng Thái

```mermaid
graph TB
    subgraph "NodeManager"
        NODELIST["📋 NodeList<br/>20 NodeInfo"]
        STATE["🔄 State<br/>DEACTIVATED<br/>ACTIVATED<br/>MARKING<br/>WARN"]
        SESSION["🔢 Session<br/>Counter"]
        SCOREBOARD["📊 Scoreboard<br/>Data"]
    end

    subgraph "NodeInfo"
        NODEID["Node ID<br/>1A-5D"]
        SHOTS["Shots[3]<br/>x,y,score"]
        STATS["Stats<br/>battery,temp"]
        CLASSIFICATION["Classification"]
    end

    subgraph "UDP Gateway"
        UPLINK["⬆️ Uplink<br/>1680"]
        DOWNLINK["⬇️ Downlink<br/>1780"]
    end

    UPLINK -->|"Parse"| NODELIST
    NODELIST -->|"Update"| STATE
    NODELIST -->|"Add"| SCOREBOARD
    SCOREBOARD -->|"Calculate"| CLASSIFICATION
    STATE -->|"Determine"| NODELIST
    
    NODELIST -.->|"Emit"| SCOREBOARD
    NODELIST -.->|"Emit"| STATS
    
    DOWNLINK -->|"Commands"| NODELIST
    
    style NODELIST fill:#c8e6c9
    style STATE fill:#fff9c4
    style SCOREBOARD fill:#bbdefb
```

---

## 7. Giao Tiếp SPI/UART STM32 ↔ RPi

```mermaid
graph LR
    subgraph "STM32F407"
        STM_SPI["SPI Slave<br/>MOSI,MISO,CLK"]
        STM_UART["UART1<br/>PA9 TX, PA10 RX"]
        STM_GPIO["GPIO PB0-PB4<br/>Control"]
    end

    subgraph "Raspberry Pi"
        RPI_SPI["SPI Master<br/>/dev/spidev0.0"]
        RPI_UART["UART<br/>/dev/serial0"]
        RPI_GPIO["GPIO 17-22<br/>Control"]
    end

    subgraph "LoRa Module"
        LORA["LoRa SX1276<br/>SPI"]
    end

    STM_SPI -->|"Timestamp Data<br/>4x32-bit<br/>1MHz"| RPI_SPI
    RPI_SPI -->|"Temp Req<br/>0x01"| STM_SPI
    
    STM_UART -->|"Debug<br/>115200"| RPI_UART
    
    STM_GPIO -->|"TC,RDC,RS"| RPI_GPIO
    RPI_GPIO -->|"17-20"| STM_GPIO
    
    STM_SPI -->|"SPI 1MHz"| LORA
    LORA -->|"SPI"| RPI_SPI
    
    style STM_SPI fill:#ffccbc
    style RPI_SPI fill:#c5cae9
    style LORA fill:#b2dfdb
```

---

## 8. State Machine Node

```mermaid
stateDiagram-v2
    [*] --> DEACTIVATED: Startup
    
    DEACTIVATED --> ACTIVATED: UP
    ACTIVATED --> DEACTIVATED: DOWN
    ACTIVATED --> ACTIVATED: Timer 70s
    
    ACTIVATED --> MARKING: MARKING ON
    MARKING --> ACTIVATED: MARKING OFF
    
    ACTIVATED --> WARN: Battery Low
    WARN --> DEACTIVATED: Reset
    WARN --> ACTIVATED: Charged
    
    DEACTIVATED --> [*]: Clear
    
    note right of ACTIVATED
        Motor ON
        Capture TDoA
    end note
    
    note right of MARKING
        Reference point
        Locked
    end note
    
    note right of WARN
        Battery < 10%
        Connection lost
    end note
```

---

## 9. Hardware Connections

```mermaid
graph TB
    subgraph "STM32F407VET6"
        PA0["PA0 ADC0"]
        PA1["PA1 ADC1"]
        PA2["PA2 ADC2"]
        PA3["PA3 ADC3"]
        PA4["PA4 SPI CS"]
        PA5["PA5 SPI CLK"]
        PA6["PA6 SPI MISO"]
        PA7["PA7 SPI MOSI"]
        PA9["PA9 UART TX"]
        PA10["PA10 UART RX"]
        PB0["PB0 GPIO IN DR"]
        PB1["PB1 GPIO OUT RDC"]
        PB2["PB2 GPIO OUT TC"]
        PB4["PB4 GPIO OUT RS"]
        I2C["I2C1 PB6/PB7"]
    end
    
    subgraph "Sensors"
        SA["📡 Sensor A"]
        SB["📡 Sensor B"]
        SC["📡 Sensor C"]
        SD["📡 Sensor D"]
        BME["🌡️ BME280"]
        LORA_M["📶 LoRa"]
    end
    
    subgraph "Raspberry Pi"
        GPIO["GPIO 17-22"]
        SPI["SPI0"]
        UART["UART"]
    end
    
    SA -->|"Analog"| PA0
    SB -->|"Analog"| PA1
    SC -->|"Analog"| PA2
    SD -->|"Analog"| PA3
    
    BME -->|"I2C"| I2C
    
    LORA_M -->|"SPI"| PA5
    LORA_M -->|"SPI"| PA6
    LORA_M -->|"SPI"| PA7
    LORA_M -->|"CS"| PA4
    
    PA9 -->|"TX"| UART
    PA10 -->|"RX"| UART
    
    PB0 -->|"DR"| GPIO
    GPIO -->|"TC/RDC/RS"| PB1
    GPIO -->|"TC/RDC/RS"| PB2
    GPIO -->|"TC/RDC/RS"| PB4
    
    SPI -->|"Master"| LORA_M
```

---

## 10. LoRa Packet Format (Node → Controller)

```mermaid
graph LR
    START["🔴"]
    
    HEADER["Header<br/>0xAA<br/>1B"]
    NODEID["Node ID<br/>1B"]
    SESSION["Session<br/>1B"]
    X["X Position<br/>Float<br/>4B"]
    Y["Y Position<br/>Float<br/>4B"]
    SCORE["Score<br/>Int16<br/>2B"]
    BATTERY["Battery %<br/>1B"]
    TEMP["Temperature<br/>Int16 x10<br/>2B"]
    CHECKSUM["Checksum<br/>CRC8<br/>1B"]
    END["🔴"]
    
    START --> HEADER
    HEADER --> NODEID
    NODEID --> SESSION
    SESSION --> X
    X --> Y
    Y --> SCORE
    SCORE --> BATTERY
    BATTERY --> TEMP
    TEMP --> CHECKSUM
    CHECKSUM --> END
    
    style HEADER fill:#ffccbc
    style CHECKSUM fill:#ffccbc
```

---

## 11. Command Message (Controller → Node)

```mermaid
graph LR
    START["🟢"]
    
    HEADER["Header<br/>0xBB<br/>1B"]
    CMD["Command<br/>0x01: UP<br/>0x02: DOWN<br/>0x03: MARKING<br/>0x04: CLEAR<br/>1B"]
    NODEID["Node ID<br/>or 0xFF<br/>Broadcast<br/>1B"]
    PARAM["Parameter<br/>Optional<br/>1B"]
    CHECKSUM["Checksum<br/>CRC8<br/>1B"]
    END["🟢"]
    
    START --> HEADER
    HEADER --> CMD
    CMD --> NODEID
    NODEID --> PARAM
    PARAM --> CHECKSUM
    CHECKSUM --> END
    
    style HEADER fill:#c8e6c9
    style CHECKSUM fill:#c8e6c9
```

---

## 12. UI Update Flow

```mermaid
sequenceDiagram
    participant Node as 📡 Node
    participant Gateway as 🌐 Gateway
    participant NodeMgr as 📦 NodeManager
    participant Signal as 📢 Qt Signals
    participant MainWin as 🖥️ MainWindow
    participant UI as 🎨 UI

    Node->>Gateway: LoRa Packet
    Gateway->>Gateway: Parse UDP
    Gateway->>NodeMgr: updateNode()
    
    NodeMgr->>NodeMgr: Update Info
    NodeMgr->>NodeMgr: Recalc avg
    
    NodeMgr->>Signal: scoreboardUpdated()
    Signal->>MainWin: onScoreboardUpdated()
    MainWin->>UI: updateScoreboard()
    UI->>UI: setItem()
    UI->>UI: Repaint
    
    NodeMgr->>Signal: nodeStatusChanged()
    Signal->>MainWin: onNodeStatusChanged()
    MainWin->>UI: updateNodeStatus()
    UI->>UI: setItem()
    
    NodeMgr->>Signal: debugLog()
    Signal->>MainWin: appendDebugLog()
    MainWin->>UI: logger->log()
    UI->>UI: append()
```

---

## 13. Sơ Đồ Thư Mục Dự Án

```
HTTDTD_v3.1/
├── 📄 README.md
├── 📄 DISCLAIMER.md
├── 📄 SAFETY_WARNING.md
├── 📄 OPERATION_LIMITATIONS.md
├── 📄 CODEMAP.md
├── docs/
│   └── 📄 MERMAID.md ⭐ (File này)
│
├── STM32F407VET6/ (Firmware STM32)
│   ├── Core/
│   │   ├── Inc/ → system.h
│   │   └── Src/ → main.c, tdoa_manager.cpp
│   ├── Drivers/
│   ├── STM32F407VETx_FLASH.ld
│   └── Makefile
│
├── tdoa_node/ (Firmware RPi Node)
│   ├── src/
│   │   ├── main.cpp
│   │   ├── Config.hpp
│   │   ├── TDOASolver.hpp/cpp ⭐
│   │   ├── NodeController.hpp/cpp
│   │   ├── SPIMaster.hpp/cpp
│   │   └── LoRaModule.hpp/cpp
│   ├── Makefile
│   └── CMakeLists.txt
│
└── controller/ (Controller Qt)
    ├── main.cpp ⭐ (SafetyDialog)
    ├── MainWindow.hpp/cpp
    ├── SafetyAcknowledgementDialog.hpp/cpp ⭐
    ├── NodeManager.hpp/cpp
    ├── UdpGateway.hpp/cpp
    ├── ScoreCalculator.hpp/cpp
    ├── Logger.hpp/cpp
    ├── Makefile
    └── resources/
```

---

## 14. Luồng Dữ Liệu Toàn Hệ Thống

```
🎯 Target
    ↓
💥 Impact → Piezo
    ↓
📡 STM32 Capture 4x timestamps @ 168MHz
    ↓
🔢 TDOASolver: Chan + LM
    ├─ TDOA: tB-tA, tC-tA, tD-tA
    ├─ Speed: 331.5 + 0.607*T
    └─ Result: (x, y) cm
    ↓
🌡️ BME280: Read Temperature
    ↓
📶 LoRa: Send Packet
    ↓
📡 Gateway: UDP Receive 1680
    ↓
📦 NodeManager: Parse & Store
    ├─ Update NodeInfo
    ├─ Calc avg (x, y)
    └─ Determine classification
    ↓
📊 Qt Signals: scoreboardUpdated()
    ↓
🖥️ MainWindow: updateScoreboard()
    ↓
🎨 UI: Display Table
    ├─ Node, Score, X, Y, Avg, Classification
    └─ Status: Battery, Temp, Connection
```

---

## 15. Error Handling & Recovery

```mermaid
graph TD
    ERROR["⚠️ Error Detected"]
    
    ERROR --> CHECK{Error Type?}
    
    CHECK -->|"Sensor Fail"| SENSOR["❌ No Signal"]
    CHECK -->|"TDOA Invalid"| TDOA["❌ Out of Range"]
    CHECK -->|"LM Fail"| LM["❌ Not Converged"]
    CHECK -->|"Network"| NET["❌ UDP Lost"]
    CHECK -->|"Battery"| BAT["⚠️ Low < 10%"]
    
    SENSOR --> LOG1["🔴 Log Error<br/>Invalid Shot"]
    TDOA --> LOG1
    LM --> LOG1
    NET --> LOG2["📴 Disconnect"]
    BAT --> LOG3["🟡 WARN State"]
    
    LOG1 --> RECOVERY1["Try Next Shot"]
    LOG2 --> RECOVERY2["Retry Connection"]
    LOG3 --> RECOVERY3["Alert User"]
    
    RECOVERY1 --> CONTINUE["Continue"]
    RECOVERY2 --> CONTINUE
    RECOVERY3 --> CONTINUE
    
    style ERROR fill:#ff6b6b
    style SENSOR fill:#ffccbc
    style TDOA fill:#ffccbc
    style LM fill:#ffccbc
```

---

## 16. Lifecycle Buổi Tập Luyện

```mermaid
timeline
    title Lifecycle Buổi Tập Luyện
    
    section Startup
        00:00 : 🚀 Start Controller
        00:05 : ⚠️ SafetyDialog Read
        00:35 : ✅ Dialog OK
        01:00 : 🔌 UDP Listen 1680
        02:00 : 📡 20 Nodes Connected
    
    section Preparation
        05:00 : 🔘 Prepare to Fire
        05:30 : UP Node 1A
        05:35 : ACTIVATED
        06:00 : Motor On 70s
        06:30 : Ready
    
    section Firing
        10:00 : 💥 BULLET HIT
        10:00 : Capture 4x TS
        10:10 : TDOASolver Calc
        10:20 : LoRa Send
        10:30 : Update Scoreboard
        10:35 : Display x=25.3 y=15.2 Score=95
    
    section Adjustment
        15:00 : Motor Auto OFF
        15:05 : DEACTIVATED
        15:30 : View Result
        20:00 : MARKING ON
        25:00 : CLEAR for Next Round
    
    section Finish
        60:00 : Session End
        60:30 : Generate Report
        61:00 : Export Data
        61:30 : Close App
```

---

## 📌 Chú Thích Ký Hiệu

| Ký Hiệu | Ý Nghĩa |
|---------|---------|
| 🎯 | Target |
| 💥 | Impact |
| 📡 | Sensor/Module |
| 🖥️ | Computer |
| ⚡ | Processing |
| 📦 | Package/Data |
| 🔢 | Calculation |
| ⚙️ | Algorithm |
| 📊 | Data/Table |
| 🎨 | UI/Interface |
| ⏰ | Timer |
| 🌡️ | Temperature |
| 📶 | LoRa/Wireless |
| 🔌 | Connection/GPIO |
| ✅ | Success |
| ❌ | Error/Failure |
| ⚠️ | Warning |
| 🚀 | Startup |
| 📝 | Log/Output |
| 💾 | Storage |

---

**Cập nhật:** 26 Tháng 5 Năm 2026  
**Phiên bản:** 1.0  
**Tác giả:** Chiêm Dũng (Dunghero1412)
```
```