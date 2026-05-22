// ============================================================================
// File: main.cpp
// Mô tả: Entry point, đọc ID từ tham số dòng lệnh và khởi động NodeController.
// ============================================================================
#include "NodeController.hpp"
#include <iostream>
#include <cstring>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <nodeID> (ví dụ 1A)\n";
        return 1;
    }
    NodeID id = parseNodeID(argv[1]);
    std::cout << "Starting NODE_" << nodeIDString(id) << std::endl;

    NodeController node(id);
    node.run();
    return 0;
}