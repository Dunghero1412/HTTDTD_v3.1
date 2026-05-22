// ============================================================================
// File: TDOASolver.cpp
// ============================================================================
#include "TDOASolver.hpp"
#include "Config.hpp"
#include <cmath>
#include <Eigen/Dense> // Sử dụng Eigen cho đại số tuyến tính và LM
// Nếu không muốn dùng Eigen, bạn có thể tự viết, nhưng Eigen rất mạnh.
// Cài: sudo apt install libeigen3-dev
#include <iostream>

using namespace Eigen;

double TDOASolver::speedOfSound(double T) {
    return 331.3 + 0.606 * T; // m/s
}

// Hàm tính sai số cho LM
struct TDOAFunctor {
    const double* tdoa; // mảng 3 giá trị TDOA (B-A, C-A, D-A)
    double v;           // vận tốc âm thanh (cm/s)
    const SensorPos* sensors;

    // Input: vector 2 chiều (x,y) tính bằng cm
    // Output: vector 3 chiều sai số (expected - measured TDOA)
    int operator()(const VectorXd& xy, VectorXd& fvec) const {
        double x = xy(0), y = xy(1);
        double dA = sqrt(pow(x - sensors[0].x, 2) + pow(y - sensors[0].y, 2));
        double dB = sqrt(pow(x - sensors[1].x, 2) + pow(y - sensors[1].y, 2));
        double dC = sqrt(pow(x - sensors[2].x, 2) + pow(y - sensors[2].y, 2));
        double dD = sqrt(pow(x - sensors[3].x, 2) + pow(y - sensors[3].y, 2));

        // TDOA: (d_i - d_A)/v (đơn vị giây)
        fvec(0) = (dB - dA)/v - tdoa[0];
        fvec(1) = (dC - dA)/v - tdoa[1];
        fvec(2) = (dD - dA)/v - tdoa[2];
        return 0;
    }
};

std::pair<double, double> TDOASolver::computePosition(
    const uint64_t timestamps[4], double temperature) {
    double v = speedOfSound(temperature) * 100.0; // m/s -> cm/s

    // Tính TDOA so với sensor A (kênh 0)
    double tdoa[3];
    for (int i = 1; i < 4; ++i) {
        // timestamp là tick 32-bit + overflow, giả sử đơn vị là ~11.9ns
        // Chuyển thành giây: (tick * (1/84MHz)) = tick / 84e6
        double tA = timestamps[0] / 84e6;
        double t_i = timestamps[i] / 84e6;
        tdoa[i-1] = t_i - tA;
    }

    // Khởi tạo vị trí ban đầu bằng phương pháp Chan (đơn giản)
    // Giải hệ tuyến tính từ 3 TDOA (xấp xỉ)
    Matrix3d A;
    Vector3d b;
    for (int i = 0; i < 3; ++i) {
        double xi = SENSORS[i+1].x, yi = SENSORS[i+1].y;
        double x1 = SENSORS[0].x, y1 = SENSORS[0].y;
        double Ki = xi*xi + yi*yi;
        double K1 = x1*x1 + y1*y1;
        A(i,0) = 2*(xi - x1);
        A(i,1) = 2*(yi - y1);
        A(i,2) = -2*v*v*tdoa[i];
        b(i) = Ki - K1 - v*v*tdoa[i]*tdoa[i];
    }
    Vector3d sol = A.colPivHouseholderQr().solve(b);
    double x0 = sol(0), y0 = sol(1);
    // Nếu nằm ngoài bia, đưa về tâm
    if (std::isnan(x0) || std::abs(x0)>100 || std::abs(y0)>100) {
        x0 = 0; y0 = 0;
    }

    // Tối ưu LM
    VectorXd xy(2);
    xy << x0, y0;
    TDOAFunctor functor = {tdoa, v, SENSORS};
    LevenbergMarquardt<TDOAFunctor> lm(functor);
    lm.minimize(xy);

    return {xy(0), xy(1)};
}