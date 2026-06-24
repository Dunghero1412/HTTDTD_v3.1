// ============================================================================
// File: main.cpp
// MÃ´ táº£: Entry point, Ä‘á»c ID tá»« tham sá»‘ dÃ²ng lá»‡nh vÃ  khá»Ÿi Ä‘á»™ng NodeController.
// ============================================================================
#include "NodeController.hpp"
#include "DebugUART.hpp"
#include <cstdio>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::fprintf(stderr, "Usage: %s <nodeID> (vÃ­ dá»¥ 1A)\n", argv[0]);
        return 1;
    }

    DebugUART::init(true, 25000);
    NodeID id = parseNodeID(argv[1]);
    DebugUART::log("Starting NODE_%s\n", nodeIDString(id).c_str());

    NodeController node(id);
    node.run();
    DebugUART::saveToFile("debug.txt");
    return 0;
}
