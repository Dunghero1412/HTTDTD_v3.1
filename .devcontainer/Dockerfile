# Môi trường phát triển HTTDTD v3.1 (Ubuntu 24.04)
FROM mcr.microsoft.com/devcontainers/cpp:1-ubuntu-24.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    # STM32 ARM toolchain
    gcc-arm-none-eabi \
    libnewlib-arm-none-eabi \
    binutils-arm-none-eabi \
    gdb-multiarch \
    # Controller (Qt5 GUI)
    qtbase5-dev \
    libqt5widgets5-dev \
    libqt5network5-dev \
    libeigen3-dev \
    pkg-config \
    # Node (biên dịch trên PC; chạy thực tế trên Raspberry Pi)
    liblgpio-dev \
    # Nạp / debug firmware STM32
    stlink-tools \
    openocd \
    # Công cụ bổ sung
    make \
    git \
    && rm -rf /var/lib/apt/lists/*
