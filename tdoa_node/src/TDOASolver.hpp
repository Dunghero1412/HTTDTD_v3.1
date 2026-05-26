// ============================================================================
// File: TDOASolver.hpp
// Mô tả: Tính toán vị trí (x,y) từ timestamp của 4 sensor.
//        Sử dụng phương pháp Chan + Levenberg-Marquardt optimization.
// ============================================================================
#pragma once
#include <cstdint>
#include <vector>
#include <utility> // pair
#include <Eigen/Dense>

class TDOASolver {
public:
    // Đầu vào: timestamp 64-bit của 4 kênh (A,B,C,D) và nhiệt độ (độ C)
    // Đầu ra: cặp (x,y) tính bằng cm
    static std::pair<double, double> computePosition(
        const uint64_t timestamps[4], double temperature);

private:
    // Tính vận tốc âm thanh từ nhiệt độ (độ C)
    // Công thức: v = 331.5 + 0.607*T (m/s)
    static double speedOfSound(double tempCelsius);
};

// ============================================================================
// Struct: TDOAFunctor
// Mô tả: Functor để tính sai số trong Levenberg-Marquardt optimization
//        Residual: fvec = (computed_TDOA - measured_TDOA)
// ============================================================================
struct TDOAFunctor {
    // Input data
    const double* tdoa;           // mảng 3 giá trị TDOA (B-A, C-A, D-A) [s]
    double v;                     // vận tốc âm thanh (cm/s)
    const void* sensorsPtr;       // con trỏ đến SENSORS array
    
    // Operator(): compute residual vector
    // Input: xy = [x, y] tính bằng cm
    // Output: fvec = residual (expected_TDOA - measured_TDOA)
    int operator()(const Eigen::VectorXd& xy, Eigen::VectorXd& fvec) const;
    
    // Operator(): compute Jacobian matrix (analytical, tối ưu LM)
    // Output: fjac = dF/d(x,y) ma trận 3x2
    int df(const Eigen::VectorXd& xy, Eigen::MatrixXd& fjac) const;
};
