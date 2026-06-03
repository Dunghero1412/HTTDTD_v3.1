// ScoreCalculator.cpp
#include "ScoreCalculator.hpp"
#include <cmath>

int ScoreCalculator::calculateScore(double x, double y) {
    double r = std::sqrt(x*x + y*y); // công thức tính khoảng cách từ tâm (0,0) đến điểm (x,y) = sqrt(x^2 + y^2)
    // Bán kính các vòng (cm): bulls-eye 1.25, sau đó cộng dần 3.5 cm
    const double radii[] = {1.25, 4.75, 8.25, 11.75, 15.25, 18.75, 22.25};
    const int scores[]   = {10, 9, 8, 7, 6, 5, 4};
    for (int i = 0; i < 7; ++i) {
        if (r <= radii[i]) return scores[i];
    }
    return 0; // ngoài vòng 4 điểm (trượt)
}

std::string ScoreCalculator::classify(int totalScore) {
    if (totalScore >= 27) return "Giỏi";
    if (totalScore >= 23) return "Khá";
    if (totalScore >= 18) return "Tốt";
    if (totalScore >= 15) return "Đạt";
    return "Trượt";
}