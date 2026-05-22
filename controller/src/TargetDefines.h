#ifndef TARGETDEFINES_H
#define TARGETDEFINES_H

#include <cmath>

// Bán kính các vòng (cm)
constexpr double BULLSEYE_RADIUS = 1.25;
constexpr double RING_STEP = 3.5;

// Điểm tối đa
constexpr int MAX_SCORE = 10;

// Hàm tính điểm từ khoảng cách tâm (cm)
inline int calculateScore(double distance) {
    if (distance <= BULLSEYE_RADIUS) return 10;
    double d = distance - BULLSEYE_RADIUS;
    int ring = static_cast<int>(d / RING_STEP); // 0: vòng 9, 1: vòng 8, ...
    int score = 9 - ring;
    if (score < 0) score = 0;   // ngoài bia
    return score;
}

// Xếp loại dựa trên tổng điểm 3 lần bắn
inline QString classifyTotal(int total) {
    if (total >= 27) return "Giỏi";
    if (total >= 23) return "Khá";
    if (total >= 18) return "Tốt";
    if (total >= 15) return "Đạt";
    return "Trượt";
}

#endif