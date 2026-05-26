```markdown
# GIỚI HẠN VẬN HÀNH (OPERATION LIMITATION)

Hệ thống này chịu ảnh hưởng bởi:
- môi trường,
- cấu hình phần cứng,
- điều kiện thao trường,
- và đặc tính vật lý của đầu đạn.

Độ chính xác và độ ổn định có thể thay đổi tùy điều kiện sử dụng thực tế.

---

# 1. GIỚI HẠN MÔI TRƯỜNG

Độ chính xác có thể giảm do:
- nhiệt độ,
- độ ẩm,
- gió,
- mưa,
- bụi,
- rung động,
- phản xạ âm,
- địa hình,
- hoặc vật cản xung quanh.

---

# 2. GIỚI HẠN CẢM BIẾN

Cảm biến Piezoelectric có thể xuất hiện:
- lão hóa,
- lệch độ nhạy,
- sai số cơ học,
- trôi tín hiệu,
- hoặc thời gian đáp ứng không đồng đều.

Sensor hỏng hoặc xuống cấp có thể gây sai lệch kết quả TDOA.

---

# 3. GIỚI HẠN ĐỒNG BỘ THỜI GIAN

Độ chính xác timestamp phụ thuộc vào:
- clock timer,
- cấu hình MCU,
- DMA,
- interrupt latency,
- comparator propagation delay,
- và chất lượng tín hiệu analog.

Sai cấu hình timer có thể gây sai lệch nghiêm trọng vị trí tính toán.

---

# 4. GIỚI HẠN LOẠI ĐẠN

Hệ thống có thể cho kết quả khác nhau tùy:
- cỡ đạn,
- tốc độ đầu đạn,
- đạn cận âm,
- súng giảm thanh,
- góc va chạm,
- và đặc tính sóng xung kích.

KHÔNG đảm bảo hoạt động giống nhau với mọi loại vũ khí và đạn.

---

# 5. GIỚI HẠN BẮN LIÊN THANH

Nhiều phát bắn liên tiếp có thể:
- chồng tín hiệu,
- gây nhầm timestamp,
- hoặc vượt khả năng xử lý của hệ thống.

Độ tin cậy có thể giảm khi tốc độ bắn quá cao.

---

# 6. GIỚI HẠN TRIỂN KHAI

Hệ thống giả định:
- cảm biến được lắp đúng vị trí,
- khoảng cách chuẩn,
- cố định chắc chắn,
- và đã hiệu chuẩn đầy đủ.

Mọi thay đổi vị trí sensor đều yêu cầu hiệu chuẩn lại.

---

# 7. GIỚI HẠN THUẬT TOÁN

Kết quả tọa độ được tính bằng:
- thuật toán TDOA,
- mô hình toán học,
- và tối ưu phi tuyến.

Trong điều kiện dữ liệu nhiễu hoặc bất thường:
- thuật toán có thể không hội tụ,
- cho ra tọa độ sai,
- hoặc xuất hiện nghiệm không hợp lệ.

---

# 8. GIỚI HẠN NGUỒN ĐIỆN

Nguồn cấp không ổn định có thể gây:
- reset MCU,
- sai timestamp,
- false trigger,
- lỗi DMA,
- hoặc hỏng phần cứng.

---

# 9. YÊU CẦU BẢO TRÌ

Hệ thống PHẢI được:
- kiểm tra định kỳ,
- hiệu chuẩn định kỳ,
- kiểm tra sensor,
- kiểm tra dây tín hiệu,
- và xác minh firmware
trước khi sử dụng thực tế.

---

# 10. TRÁCH NHIỆM VẬN HÀNH

Trách nhiệm cuối cùng về an toàn luôn thuộc về:
- người vận hành,
- cán bộ phụ trách,
- và sĩ quan thao trường.

Hệ thống này chỉ là công cụ hỗ trợ kỹ thuật.

KHÔNG phải hệ thống tự động tuyệt đối.
```