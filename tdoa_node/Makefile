CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2
LDFLAGS = -llgpio -lpthread
INCLUDES = -I/usr/include/eigen3
SRC = src/main.cpp src/NodeController.cpp src/LoRaModule.cpp src/SPIMaster.cpp src/TDOASolver.cpp
TARGET = node

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)