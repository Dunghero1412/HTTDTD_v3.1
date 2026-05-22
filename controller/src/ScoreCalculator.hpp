// ScoreCalculator.hpp
#pragma once
#include <utility>
#include <string>

class ScoreCalculator {
public:
    // Trả về điểm (0-10) dựa trên khoảng cách sqrt(x^2+y^2) tính bằng cm
    static int calculateScore(double x, double y);

    // Xếp loại dựa trên tổng điểm tối đa 3 lần (max 30)
    // Trả về chuỗi: "Giỏi", "Khá", "Tốt", "Đạt", "Trượt"
    static std::string classify(int totalScore);
};