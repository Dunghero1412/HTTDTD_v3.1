// ============================================================================
// File: TDOASolver.cpp
// Mô tả: Tính toán vị trí (x,y) từ timestamp của 4 sensor
//        Sử dụng phương pháp Chan + Levenberg-Marquardt
// ============================================================================
#include "TDOASolver.hpp"
#include "Config.hpp"
#include <cmath>
#include <Eigen/Dense> // Sử dụng Eigen cho đại số tuyến tính và LM
// Nếu không muốn dùng Eigen, bạn có thể tự viết, nhưng Eigen rất mạnh.
// Cài: sudo apt install libeigen3-dev
#include <iostream>

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

// ============================================================================
// Functor cho Levenberg-Marquardt Optimization
// Dùng để tối ưu hóa vị trí (x, y) bằng cách minimize sai số TDOA
// ============================================================================
struct TDOAFunctor {
    // Đầu vào
    const double* tdoa;           // Mảng 3 giá trị TDOA: (tB-tA), (tC-tA), (tD-tA) [giây]
    double v;                     // Vận tốc âm thanh [cm/s]
    const SensorPos* sensors;     // Vị trí 4 cảm biến [cm]

    // ========================================================================
    // Hàm compute residual (sai số)
    // Input:  xy = vector 2 chiều [x, y] tính bằng cm
    // Output: fvec = vector 3 chiều sai số [(expected - measured) TDOA]
    // 
    // Sai số = (d_i - d_A)/v - tdoa[i]
    // ========================================================================
    int operator()(const VectorXd& xy, VectorXd& fvec) const {
        double x = xy(0), y = xy(1);
        
        // Tính khoảng cách từ (x,y) đến mỗi cảm biến
        double dA = sqrt(pow(x - sensors[0].x, 2) + pow(y - sensors[0].y, 2));
        double dB = sqrt(pow(x - sensors[1].x, 2) + pow(y - sensors[1].y, 2));
        double dC = sqrt(pow(x - sensors[2].x, 2) + pow(y - sensors[2].y, 2));
        double dD = sqrt(pow(x - sensors[3].x, 2) + pow(y - sensors[3].y, 2));
        double dE = sqrt(pow(x - sensors[4].x, 2) + pow(y - sensors[4].y, 2));
        double dF = sqrt(pow(x - sensors[5].x, 2) + pow(y - sensors[5].y, 2));

        // DEBUG: In ra khoảng cách (có thể bỏ comment để xem)
        // std::cout << "dA=" << dA << " dB=" << dB << " dC=" << dC << " dD=" << dD << std::endl;

        // TDOA = (d_i - d_A)/v - measured_tdoa[i]
        // Nếu = 0 thì hoàn hảo
        fvec(0) = (dB - dA) / v - tdoa[0];  // TDOA B vs A
        fvec(1) = (dC - dA) / v - tdoa[1];  // TDOA C vs A
        fvec(2) = (dD - dA) / v - tdoa[2];  // TDOA D vs A
        fvec(3) = (dE - dA) / v - tdoa[3];  // TDOA E vs A
        fvec(4) = (dF - dA) / v - tdoa[4];  // TDOA F vs A

        // DEBUG: In ra sai số
        // std::cout << "residual: " << fvec.transpose() << std::endl;

        return 0;
    }

    // ========================================================================
    // Hàm compute Jacobian matrix (đạo hàm riêng)
    // Input:  xy = vector 2 chiều [x, y]
    // Output: fjac = ma trận 3x2 (3 phương trình, 2 ẩn)
    //         fjac(i,j) = dF_i / d(xy_j)
    // 
    // J = [dF0/dx  dF0/dy]
    //     [dF1/dx  dF1/dy]
    //     [dF2/dx  dF2/dy]
    // ========================================================================
    int df(const VectorXd& xy, MatrixXd& fjac) const {
        double x = xy(0), y = xy(1);
        
        // Tính khoảng cách từ (x,y) đến mỗi cảm biến
        double dA = sqrt(pow(x - sensors[0].x, 2) + pow(y - sensors[0].y, 2));
        double dB = sqrt(pow(x - sensors[1].x, 2) + pow(y - sensors[1].y, 2));
        double dC = sqrt(pow(x - sensors[2].x, 2) + pow(y - sensors[2].y, 2));
        double dD = sqrt(pow(x - sensors[3].x, 2) + pow(y - sensors[3].y, 2));
        double dE = sqrt(pow(x - sensors[4].x, 2) + pow(y - sensors[4].y, 2));
        double dF = sqrt(pow(x - sensors[5].x, 2) + pow(y - sensors[5].y, 2));

        // Tránh chia cho 0
        if (dA < 1e-6 || dB < 1e-6 || dC < 1e-6 || dD < 1e-6 || dE < 1e-6 || dF < 1e-6) {
            fjac.setZero();
            return 1; // Error flag
        }

        // Đạo hàm của residual TDOA theo x,y
        // dF_i/dx = (1/v) * d(d_i - d_A)/dx
        //         = (1/v) * ((x - x_i)/d_i - (x - x_A)/d_A)
        
        // Row 0: TDOA B vs A
        fjac(0, 0) = ((x - sensors[1].x) / dB - (x - sensors[0].x) / dA) / v;
        fjac(0, 1) = ((y - sensors[1].y) / dB - (y - sensors[0].y) / dA) / v;
        
        // Row 1: TDOA C vs A
        fjac(1, 0) = ((x - sensors[2].x) / dC - (x - sensors[0].x) / dA) / v;
        fjac(1, 1) = ((y - sensors[2].y) / dC - (y - sensors[0].y) / dA) / v;
        
        // Row 2: TDOA D vs A
        fjac(2, 0) = ((x - sensors[3].x) / dD - (x - sensors[0].x) / dA) / v;
        fjac(2, 1) = ((y - sensors[3].y) / dD - (y - sensors[0].y) / dA) / v;
        
        // Row 3: TDOA E vs A
        fjac(3, 0) = ((x - sensors[4].x) / dE - (x - sensors[0].x) / dA) / v;
        fjac(3, 1) = ((y - sensors[4].y) / dE - (y - sensors[0].y) / dA) / v;
        
        // Row 4: TDOA F vs A
        fjac(4, 0) = ((x - sensors[5].x) / dF - (x - sensors[0].x) / dA) / v;
        fjac(4, 1) = ((y - sensors[5].y) / dF - (y - sensors[0].y) / dA) / v;

        // DEBUG: In ra Jacobian
        // std::cout << "Jacobian:\n" << fjac << std::endl;

        return 0;
    }
};

// ============================================================================
// Hàm chính: Tính toán vị trí (x,y) từ timestamp 6 cảm biến
// ============================================================================
std::pair<double, double> TDOASolver::computePosition(
    const uint64_t timestamps[6], double temperature) {
    
    // ========================================================================
    // BƯỚC 1: Tính tốc độ âm thanh dựa trên nhiệt độ
    // ========================================================================
    double v_ms = speedOfSound(temperature);           // m/s
    double v = v_ms * 100.0;                           // Chuyển sang cm/s
    
    // DEBUG: In ra vận tốc
    // std::cout << "Temperature: " << temperature << "C, Speed of sound: " << v << " cm/s" << std::endl;

    // ========================================================================
    // BƯỚC 2: Tính TDOA (Time Difference of Arrival) so với cảm biến A
    // ========================================================================
    // STM32F407: Timer chạy ở 168MHz (full system clock)
    // 1 tick = 1/168MHz = 5.95 nanoseconds
    // Chuyển tick thành giây: timestamp_sec = timestamp_tick / 168e6
    
    double tdoa[5];
    double tA = timestamps[0] / 168e6;    // ← FIX: Đổi từ 84e6 → 168e6 (đúng frequency)
    
    for (int i = 1; i < 6; ++i) {
        double t_i = timestamps[i] / 168e6;
        tdoa[i - 1] = t_i - tA;
    }
    
    // DEBUG: In ra TDOA
    // std::cout << "TDOA: " << tdoa[0] << "s, " << tdoa[1] << "s, " << tdoa[2] << "s" << std::endl;
    // std::cout << "Timestamps (s): " << tA << ", " << (timestamps[1]/168e6) 
    //           << ", " << (timestamps[2]/168e6) << ", " << (timestamps[3]/168e6) << std::endl;

    // ========================================================================
    // BƯỚC 3: Khởi tạo vị trí ban đầu bằng phương pháp Chan (Linear approximation)
    // ========================================================================
    // Chan method: Giải hệ phương trình tuyến tính 2x2 từ 2 TDOA đầu tiên
    // 
    // Từ TDOA: (d_i - d_A)/v = tdoa[i]
    //          d_i - d_A = v * tdoa[i]
    //
    // Phương trình tuyến tính:
    // 2*(x_i - x_A)*x + 2*(y_i - y_A)*y = x_i^2 + y_i^2 - x_A^2 - y_A^2 - v^2*tdoa[i]^2
    
    MatrixXd A_chan(4, 2);
    VectorXd b_chan(4);
    
    for (int i = 0; i < 4; ++i) {  // ← Chỉ dùng 4 cảm biến (B, C, D, E) để tạo 4x2 system
        double xi = SENSORS[i + 1].x, yi = SENSORS[i + 1].y;
        double x1 = SENSORS[0].x, y1 = SENSORS[0].y;
        double Ki = xi * xi + yi * yi;
        double K1 = x1 * x1 + y1 * y1;
        
        // Matrix A: [2(x_i - x_A)   2(y_i - y_A)]
        A_chan(i, 0) = 2 * (xi - x1);
        A_chan(i, 1) = 2 * (yi - y1);
        
        
        // Vector b: x_i^2 + y_i^2 - x_A^2 - y_A^2 - v^2*tdoa[i]^2
        // ← FIX: Công thức Chan đúng (không có thừa số v^2*tdoa^2 ở đây)
        b_chan(i) = Ki - K1 - v * v * tdoa[i] * tdoa[i];
    }
    
    // DEBUG: In ra ma trận Chan
    // std::cout << "Chan matrix A:\n" << A_chan << "\nChan vector b:\n" << b_chan.transpose() << std::endl;
    
    // Giải hệ tuyến tính A * x = b
    Vector2d sol_chan = A_chan.colPivHouseholderQr().solve(b_chan);
    double x0 = sol_chan(0);
    double y0 = sol_chan(1);
    
    // DEBUG: In ra kết quả Chan
    // std::cout << "Chan initial guess: x0=" << x0 << ", y0=" << y0 << std::endl;
    
    // Kiểm tra và điều chỉnh nếu vị trí ban đầu bất hợp lý
    // Nếu NaN hoặc ngoài bia (~100cm), dùng tâm bia (0,0)
    if (std::isnan(x0) || std::isnan(y0) || std::abs(x0) > 150 || std::abs(y0) > 150) {
        // std::cout << "Invalid initial position, using (0,0)" << std::endl;
        x0 = 0.0;
        y0 = 0.0;
    }

    // ========================================================================
    // BƯỚC 4: Tối ưu hóa bằng Levenberg-Marquardt (LM)
    // ========================================================================
    VectorXd xy(2);
    xy << x0, y0;
    
    // Tạo functor với dữ liệu TDOA
    TDOAFunctor functor = {tdoa, v, SENSORS};
    
    // Tạo đối tượng LM solver
    LevenbergMarquardt<TDOAFunctor> lm(functor);
    
    // DEBUG: Có thể set các tham số LM (nếu cần)
    // lm.parameters.ftol = 1e-6;
    // lm.parameters.xtol = 1e-6;
    // lm.parameters.gtol = 1e-6;
    // lm.parameters.maxfev = 1000;
    
    // Chạy tối ưu hóa
    int ret = lm.minimize(xy);
    
    // DEBUG: In ra kết quả LM
    // std::cout << "LM optimization result code: " << ret << std::endl;
    // std::cout << "LM iterations: " << lm.nfev << std::endl;
    // std::cout << "LM final residual norm: " << lm.fvec.norm() << std::endl;
    
    // ========================================================================
    // BƯỚC 5: Return kết quả tối ưu
    // ========================================================================
    // std::cout << "Final position: x=" << xy(0) << " cm, y=" << xy(1) << " cm" << std::endl;
    
    return {xy(0), xy(1)};
}

// ============================================================================
// DEBUG & NOTES:
// ============================================================================
// 1. Timer frequency: STM32F407 chạy ở 168MHz (NOT 84MHz)
//    → 1 tick = 5.95 ns → timestamp / 168e6 = time in seconds
//
// 2. Chan method: Linear approximation để tìm initial guess
//    → Dùng 2x2 system từ 2 TDOA đầu
//    → Không có hệ số 2 ở v^2*tdoa[i] trong công thức
//
// 3. Levenberg-Marquardt: Non-linear optimization
//    → Cần Jacobian analytical để tối ưu (không dùng numerical diff)
//    → Hàm df() cung cấp Jacobian chính xác
//
// 4. TDOA functor:
//    → operator(): compute residual
//    → df(): compute Jacobian matrix (3x2)
//
// 5. Temperature compensation:
//    → Công thức Cramer: v = 331.5 + 0.607*T (m/s)
//    → Dữ liệu từ BME280 sensor
//
// 6. Sensor positions:
//    → Được define trong Config.hpp (SENSORS array)
//    → Đơn vị: cm
//    → Reference: Sensor A (index 0)
// ============================================================================
