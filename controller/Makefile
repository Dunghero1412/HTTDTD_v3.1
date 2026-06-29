CXX = g++
CXXFLAGS = -std=c++17 -Wall -O2 -fPIC -DQT_WIDGETS_LIB -DQT_GUI_LIB -DQT_NETWORK_LIB -DQT_CORE_LIB
INCLUDES = -I/usr/include/x86_64-linux-gnu/qt5 \
           -I/usr/include/x86_64-linux-gnu/qt5/QtWidgets \
           -I/usr/include/x86_64-linux-gnu/qt5/QtGui \
           -I/usr/include/x86_64-linux-gnu/qt5/QtNetwork \
           -I/usr/include/x86_64-linux-gnu/qt5/QtCore \
           -I/usr/include/eigen3
LIBS = -lQt5Widgets -lQt5Gui -lQt5Network -lQt5Core

MOC = /usr/lib/qt5/bin/moc

# Danh sách các file có chứa Q_OBJECT cần chạy MOC
MOC_HEADERS = MainWindow.hpp UdpGateway.hpp NodeManager.hpp Logger.hpp
MOC_SRCS = $(patsubst %.hpp, moc_%.cpp, $(MOC_HEADERS))
MOC_OBJS = $(MOC_SRCS:.cpp=.o)

# Danh sách file object gốc
OBJS = main.o MainWindow.o UdpGateway.o NodeManager.o ScoreCalculator.o Logger.o

TARGET = controller

all: $(TARGET)

$(TARGET): $(OBJS) $(MOC_OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $(TARGET) $(OBJS) $(MOC_OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

# Quy tắc tự động tạo file moc_*.cpp từ *.hpp
moc_%.cpp: %.hpp
	$(MOC) $< -o $@

clean:
	rm -f *.o moc_*.cpp $(TARGET)

