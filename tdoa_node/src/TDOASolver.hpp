// ============================================================================
// File: TDOASolver.hpp
// Mô tả: Tính toán vị trí (x,y) từ timestamp của 4 sensor.
// ============================================================================
#pragma once
#include <cstdint>
#include <vector>
#include <utility> // pair

class TDOASolver {
public:
    // Đầu vào: timestamp 64-bit của 4 kênh (A,B,C,D) và nhiệt độ (độ C)
    // Đầu ra: cặp (x,y) tính bằng cm
    static std::pair<double, double> computePosition(
        const uint64_t timestamps[4], double temperature);
private:
    static double speedOfSound(double tempCelsius); // m/s
};