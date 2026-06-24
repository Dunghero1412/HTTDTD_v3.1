// ============================================================================
// File: TDOASolver.cpp
// Mô tả: Tính toán vị trí (x,y) từ timestamp của 4 sensor
//        Sử dụng phương pháp Chan + Levenberg-Marquardt
// ============================================================================
#include "TDOASolver.hpp" // Header chứa class TDOASolver và struct SensorPos
#include "Config.hpp" // Header chứa cấu hình cảm biến (SENSORS array) và các hằng số khác
#include <cmath> // Sử dụng math cho sqrt, pow, isnan
#include <Eigen/Dense> // Sử dụng Eigen cho đại số tuyến tính và LM
// Nếu không muốn dùng Eigen, có thể tự viết, nhưng Eigen rất mạnh.
// Cài: sudo apt install libeigen3-dev
#include <unsupported/Eigen/NonLinearOptimization> // Cho Levenberg-Marquardt
#include <unsupported/Eigen/NumericalDiff> // Cho numerical differentiation (nếu cần)
#include <limits> // Cho std::numeric_limits
#include <iostream> // Cho debug (có thể bỏ sau khi hoàn thiện)
// fvec và fjac là vector và ma trận dùng trong LM optimization được định nghĩa trong Eigen::LevenbergMarquardt 

using namespace Eigen;

// ============================================================================
// Tính tốc độ âm thanh dựa trên nhiệt độ
// Công thức: v(T) = 331.5 + 0.607*T (m/s)
// T: nhiệt độ độ C
// ============================================================================
double TDOASolver::speedOfSound(double T) {
    // v = 331.3 + 0.606*T (công thức gần đúng)
    // v = 331.5 + 0.607*T (công thức Cramer - chính xác hơn)
    return 331.5 + 0.607 * T; // m/s
}

int TDOAFunctor::operator()(const VectorXd& xy, VectorXd& fvec) const {
    double x = xy(0), y = xy(1);
    
    double dA = sqrt(pow(x - sensors[0].x, 2) + pow(y - sensors[0].y, 2));
    double dB = sqrt(pow(x - sensors[1].x, 2) + pow(y - sensors[1].y, 2));
    double dC = sqrt(pow(x - sensors[2].x, 2) + pow(y - sensors[2].y, 2));
    double dD = sqrt(pow(x - sensors[3].x, 2) + pow(y - sensors[3].y, 2));
    double dE = sqrt(pow(x - sensors[4].x, 2) + pow(y - sensors[4].y, 2));
    double dF = sqrt(pow(x - sensors[5].x, 2) + pow(y - sensors[5].y, 2));

    fvec(0) = (dB - dA) / v - tdoa[0];
    fvec(1) = (dC - dA) / v - tdoa[1];
    fvec(2) = (dD - dA) / v - tdoa[2];
    fvec(3) = (dE - dA) / v - tdoa[3];
    fvec(4) = (dF - dA) / v - tdoa[4];

    return 0;
}

int TDOAFunctor::df(const VectorXd& xy, MatrixXd& fjac) const {
    double x = xy(0), y = xy(1);
    
    double dA = sqrt(pow(x - sensors[0].x, 2) + pow(y - sensors[0].y, 2));
    double dB = sqrt(pow(x - sensors[1].x, 2) + pow(y - sensors[1].y, 2));
    double dC = sqrt(pow(x - sensors[2].x, 2) + pow(y - sensors[2].y, 2));
    double dD = sqrt(pow(x - sensors[3].x, 2) + pow(y - sensors[3].y, 2));
    double dE = sqrt(pow(x - sensors[4].x, 2) + pow(y - sensors[4].y, 2));
    double dF = sqrt(pow(x - sensors[5].x, 2) + pow(y - sensors[5].y, 2));

    if (dA < 1e-6 || dB < 1e-6 || dC < 1e-6 || dD < 1e-6 || dE < 1e-6 || dF < 1e-6) {
        fjac.setZero();
        return 1;
    }

    fjac(0, 0) = ((x - sensors[1].x) / dB - (x - sensors[0].x) / dA) / v;
    fjac(0, 1) = ((y - sensors[1].y) / dB - (y - sensors[0].y) / dA) / v;
    fjac(1, 0) = ((x - sensors[2].x) / dC - (x - sensors[0].x) / dA) / v;
    fjac(1, 1) = ((y - sensors[2].y) / dC - (y - sensors[0].y) / dA) / v;
    fjac(2, 0) = ((x - sensors[3].x) / dD - (x - sensors[0].x) / dA) / v;
    fjac(2, 1) = ((y - sensors[3].y) / dD - (y - sensors[0].y) / dA) / v;
    fjac(3, 0) = ((x - sensors[4].x) / dE - (x - sensors[0].x) / dA) / v;
    fjac(3, 1) = ((y - sensors[4].y) / dE - (y - sensors[0].y) / dA) / v;
    fjac(4, 0) = ((x - sensors[5].x) / dF - (x - sensors[0].x) / dA) / v;
    fjac(4, 1) = ((y - sensors[5].y) / dF - (y - sensors[0].y) / dA) / v;

    return 0;
}

// ============================================================================
// Hàm chính: Tính toán vị trí (x,y) từ timestamp 6 cảm biến
// ============================================================================
std::pair<double, double> TDOASolver::computePosition(
    const uint64_t timestamps[6], double temperature) {
    
    // ========================================================================
    // BƯỚC 1: Tính tốc độ âm thanh dựa trên nhiệt độ
    // công thức: v(T) = 331.5 + 0.607*T (m/s)
    // ========================================================================
    double v_ms = speedOfSound(temperature);           // m/s
    double v = v_ms * 100.0;                           // Chuyển sang cm/s
    
    // DEBUG: In ra vận tốc
    // std::cout << "Temperature: " << temperature << "C, Speed of sound: " << v << " cm/s" << std::endl;

    // ========================================================================
    // BƯỚC 2: Tính TDOA (Time Difference of Arrival) so với cảm biến A
    // công thức: tdoa[i] = (timestamp[i] - timestamp[0]) / timer_frequency
    // ========================================================================
    // STM32F407VET6 clock configuration:
    // - HCLK = 168MHz (SYSCLK)
    // - APB1 = HCLK/4 = 42MHz, Timer Clock = 42MHz × 2 = 84MHz (when prescaler > 1)
    // - TIM2, TIM5 ở APB1 với Prescaler=0 → không chia thêm → chạy ở 84MHz
    // 
    // Vậy: Timer Frequency = 84MHz (KHÔNG phải 168MHz)
    // 1 tick = 1/84MHz ≈ 11.9 nanoseconds
    // Chuyển tick thành giây: timestamp_sec = timestamp_tick / 84e6
    
    double tdoa[5];
    double tA = timestamps[0] / 84e6;     // ← Timer chạy 84MHz, chia cho 84e6 (CHÍNH XÁC)
    
    for (int i = 1; i < 6; ++i) {
        double t_i = timestamps[i] / 84e6;
        tdoa[i - 1] = t_i - tA;
    }
    
    // DEBUG: In ra TDOA
    // std::cout << "TDOA: " << tdoa[0] << "s, " << tdoa[1] << "s, " << tdoa[2] << "s" << std::endl;
    // std::cout << "Timestamps (s): " << tA << ", " << (timestamps[1]/84e6) 
    //           << ", " << (timestamps[2]/84e6) << ", " << (timestamps[3]/84e6) << std::endl;

    // ========================================================================
    // BƯỚC 3: Khởi tạo vị trí ban đầu bằng phương pháp Chan (Linear approximation)
    // công thức: A * x = b với A là ma trận 5x2 (từ 5 TDOA) và b là vector 5x1
    // Sử dụng tất cả 5 TDOA (B, C, D, E, F) để tạo hệ phương trình tuyến tính
    // ========================================================================
    // Chan method: Giải hệ phương trình tuyến tính từ 5 TDOA (6 sensors)
    // Sử dụng least squares: A * x = b với A là 5x2 matrix (overdetermined)
    // 
    // Từ TDOA: (d_i - d_A)/v = tdoa[i]
    //          d_i - d_A = v * tdoa[i]
    //
    // Phương trình tuyến tính:
    // 2*(x_i - x_A)*x + 2*(y_i - y_A)*y = x_i^2 + y_i^2 - x_A^2 - y_A^2 - v^2*tdoa[i]^2
    // Với i = 1..5 (B, C, D, E, F), A là ma trận 5x2, b là vector 5x1.
    // Giải bằng least squares để tìm x0, y0 ban đầu cho LM optimization.
    
    MatrixXd A_chan(5, 2); // Ma trận 5x2
    VectorXd b_chan(5);    // Vector 5x1
    
    for (int i = 0; i < 5; ++i) {  // ← Dùng hết 5 TDOA (B, C, D, E, F) để tạo 5x2 system
        double xi = SENSORS[i + 1].x, yi = SENSORS[i + 1].y; // Sensor B, C, D, E, F (indexes 1-5)
        double x1 = SENSORS[0].x, y1 = SENSORS[0].y;         // Sensor A (index 0)
        double Ki = xi * xi + yi * yi;                       // Ki = x_i^2 + y_i^2
        double K1 = x1 * x1 + y1 * y1;                       // K1 = x_A^2 + y_A^2 (A là sensor reference)
        
        // Matrix A: [2(x_i - x_A)   2(y_i - y_A)]
        A_chan(i, 0) = 2 * (xi - x1); // dF_i/dx
        A_chan(i, 1) = 2 * (yi - y1); // dF_i/dy
        
        
        // Vector b: x_i^2 + y_i^2 - x_A^2 - y_A^2 - v^2*tdoa[i]^2
        b_chan(i) = Ki - K1 - v * v * tdoa[i] * tdoa[i]; // ← Dùng tdoa[i] từ 0..4 (B, C, D, E, F) để tạo 5 equations
    }
    
    // DEBUG: In ra ma trận Chan
    // std::cout << "Chan matrix A (5x2):\n" << A_chan << "\nChan vector b:\n" << b_chan.transpose() << std::endl;
    
    // Giải hệ tuyến tính A * x = b bằng least squares
    // công thức đầy đủ: x = (A^T * A)^(-1) * A^T * b
    Vector2d sol_chan = A_chan.colPivHouseholderQr().solve(b_chan); // Giải bằng QR decomposition (robust cho hệ overdetermined)    
    double x0 = sol_chan(0); // Vị trí x ban đầu từ Chan
    double y0 = sol_chan(1); // Vị trí y ban đầu từ Chan
    
    // DEBUG: In ra kết quả Chan
    // std::cout << "Chan initial guess: x0=" << x0 << ", y0=" << y0 << std::endl;
    
    // Kiểm tra và điều chỉnh nếu vị trí ban đầu bất hợp lý
    // Nếu NaN hoặc ngoài bia (~100cm), dùng tâm bia (0,0)
    if (std::isnan(x0) || std::isnan(y0) || std::abs(x0) > 150 || std::abs(y0) > 150) {
        // std::cout << "Invalid initial position, using (0,0)" << std::endl;
        x0 = 0.0; // Đặt lại vị trí ban đầu về tâm bia nếu kết quả Chan không hợp lệ
        y0 = 0.0; // (0,0) là vị trí trung tâm của bia, an toàn để bắt đầu tối ưu hóa
    }

    // ========================================================================
    // BƯỚC 4: Tối ưu hóa bằng Levenberg-Marquardt (LM)
    // Sử dụng vị trí ban đầu từ Chan để bắt đầu tối ưu hóa phi tuyến
    // công thức: minimize ||F(xy)||^2 với F(xy) là vector residual từ TDOA equations
    // ========================================================================
    VectorXd xy(2);
    xy << x0, y0;
    
    // Tạo functor với dữ liệu TDOA
    TDOAFunctor functor = {tdoa, v, SENSORS}; // Cung cấp TDOA, vận tốc âm thanh, và vị trí cảm biến cho functor
    
    // Tạo đối tượng LM solver
    LevenbergMarquardt<TDOAFunctor> lm(functor); // Sử dụng functor để tối ưu hóa vị trí (x,y)
    
    // Cấu hình LM cho tối ưu hóa đầy đủ (không cần tốc độ nhanh)
    // Tăng độ chính xác convergence
    lm.parameters.ftol = 1e-8;    // Function value tolerance
    lm.parameters.xtol = 1e-8;    // Parameter change tolerance
    lm.parameters.gtol = 1e-8;    // Gradient tolerance
    lm.parameters.maxfev = 5000;  // Max function evaluations (default ~200)
    
    // DEBUG: Có thể bật chi tiết tối ưu hóa
    // std::cout << "LM optimization starting with initial guess: ("
    //           << xy(0) << ", " << xy(1) << ")" << std::endl;
    
    // Chạy tối ưu hóa
    int ret = lm.minimize(xy); // Kết quả tối ưu hóa, xy sẽ chứa vị trí tối ưu sau khi chạy
    
    // DEBUG: In ra kết quả LM
    // std::cout << "LM optimization result code: " << ret << std::endl;
    // std::cout << "LM iterations: " << lm.nfev << " (max: " << lm.parameters.maxfev << ")" << std::endl;
    // std::cout << "LM final residual norm: " << lm.fvec.norm() << std::endl;
    // std::cout << "LM final position: (" << xy(0) << ", " << xy(1) << ")" << std::endl;
    
    // ========================================================================
    // BƯỚC 5: Return kết quả tối ưu
    // ========================================================================
    // std::cout << "Final position: x=" << xy(0) << " cm, y=" << xy(1) << " cm" << std::endl;
    
    return {xy(0), xy(1)}; // Trả về vị trí (x,y) tính bằng cm
}

// ============================================================================
// DEBUG & NOTES:
// ============================================================================
// 1. Timer frequency: STM32F407VET6 TIM2, TIM5 chạy ở 84MHz (NOT 168MHz)
//    Clock path: HCLK(168MHz) → APB1(42MHz) → Timer(84MHz when PSC>1, PSC=0)
//    → 1 tick = 11.9 ns → timestamp / 84e6 = time in seconds
//    ⚠️ COMMON MISTAKE: Using 168e6 instead of 84e6 causes 2x error in TDOA!
//
// 2. Chan method: Linear approximation để tìm initial guess (phương pháp đầy đủ)
//    → Dùng 5x2 least squares system từ HẾT 5 TDOA (6 sensors)
//    → Giải bằng ColPivHouseholderQR (most robust)
//    → Kết quả là tối ưu nhất cho bài toán tuyến tính
//
// 3. Levenberg-Marquardt: Non-linear optimization (tối ưu 2nd stage)
//    → Sử dụng tất cả 5 TDOA equations (6 sensors)
//    → Cần Jacobian analytical để tối ưu (không dùng numerical diff)
//    → Hàm df() cung cấp Jacobian chính xác 5x2 matrix
//    → Tăng maxfev lên 5000 & tolerances xuống 1e-8 (độ chính xác cao)
//
// 4. TDOA functor:
//    → operator(): compute 5-element residual vector
//    → df(): compute 5x2 Jacobian matrix (5 equations, 2 unknowns)
//
// 5. Temperature compensation:
//    → Công thức Cramer: v = 331.5 + 0.607*T (m/s)
//    → Dữ liệu từ BME280 sensor
//
// 6. Sensor positions (6 sensors):
//    → Được define trong Config.hpp (SENSORS array)
//    → Đơn vị: cm
//    → Reference: Sensor A (index 0)
//    → B, C, D, E, F: indexes 1-5
//
// 7. Phương pháp này "đầy đủ nhất" vì:
//    → Chan: 5x2 least squares (not 4x2 or 2x2)
//    → LM: uses all 6 sensors, not subsets
//    → LM: tight tolerances & high max iterations (not speed-optimized)
// ============================================================================
