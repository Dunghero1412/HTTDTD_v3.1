GIỚI HẠN VẬN HÀNH (OPERATION LIMITATIONS)

Version: 3.1.37 (Stable)
Ngày phát hành: 2026-05-17
Cập nhật lần cuối: 2026-05-26

---

1. TỔNG QUAN

Hệ thống được thiết kế nhằm hỗ trợ xác định tọa độ điểm va chạm bằng phương pháp TDOA (Time Difference Of Arrival).

Độ chính xác và độ ổn định của hệ thống phụ thuộc vào:

- Điều kiện môi trường
- Cấu hình phần cứng
- Chất lượng tín hiệu đầu vào
- Điều kiện thao trường
- Phương pháp triển khai
- Và đặc tính vật lý của đầu đạn

Kết quả do hệ thống cung cấp có thể thay đổi tùy theo điều kiện vận hành thực tế.

Hệ thống không bảo đảm độ chính xác tuyệt đối trong mọi tình huống.

---

2. GIỚI HẠN MÔI TRƯỜNG

Hiệu suất và độ chính xác của hệ thống có thể suy giảm do các yếu tố môi trường, bao gồm nhưng không giới hạn:

- Nhiệt độ môi trường
- Độ ẩm không khí
- Gió mạnh
- Mưa hoặc hơi nước
- Bụi và rung động cơ học
- Phản xạ âm thanh
- Địa hình phức tạp
- Vật cản xung quanh
- Nhiễu điện từ

Các yếu tố trên có thể ảnh hưởng trực tiếp đến chất lượng tín hiệu thu nhận và sai số tính toán TDOA.

---

3. GIỚI HẠN CẢM BIẾN

Cảm biến Piezoelectric có thể phát sinh sai số do:

- Lão hóa vật liệu
- Lệch độ nhạy giữa các cảm biến
- Sai số cơ học
- Trôi tín hiệu theo thời gian
- Thời gian đáp ứng không đồng đều
- Lắp đặt không chính xác
- Tác động rung hoặc va đập cơ học

Cảm biến xuống cấp hoặc hoạt động bất thường có thể gây sai lệch đáng kể kết quả định vị.

---

4. GIỚI HẠN ĐỒNG BỘ THỜI GIAN

Độ chính xác timestamp phụ thuộc vào:

- Clock timer hệ thống
- Cấu hình MCU
- DMA latency
- Interrupt latency
- Comparator propagation delay
- Chất lượng tín hiệu analog
- Nhiễu điện và jitter hệ thống

Sai lệch trong cấu hình timer hoặc đồng bộ thời gian có thể dẫn đến sai số lớn trong quá trình tính toán tọa độ.

---

5. GIỚI HẠN THEO LOẠI ĐẠN

Kết quả định vị có thể thay đổi tùy theo:

- Cỡ đạn
- Tốc độ đầu đạn
- Đạn cận âm
- Vũ khí có gắn giảm thanh
- Góc va chạm
- Khoảng cách bắn
- Đặc tính sóng xung kích
- Đặc tính vật liệu bia

Hệ thống không bảo đảm hoạt động đồng nhất với mọi loại vũ khí, loại đạn hoặc điều kiện bắn khác nhau.

---

6. GIỚI HẠN KHI BẮN LIÊN THANH

Trong điều kiện tốc độ bắn cao hoặc nhiều phát bắn liên tiếp:

- Tín hiệu có thể chồng lấp
- Timestamp có thể bị nhầm lẫn
- Bộ đệm DMA có thể quá tải
- Hệ thống có thể phát sinh false trigger
- Thuật toán có thể xử lý sai thứ tự va chạm

Độ tin cậy của kết quả có thể giảm đáng kể khi vượt quá khả năng xử lý thiết kế của hệ thống.

---

7. GIỚI HẠN TRIỂN KHAI THỰC TẾ

Hệ thống giả định rằng:

- Cảm biến được lắp đúng vị trí
- Khoảng cách giữa các sensor đúng cấu hình
- Cảm biến được cố định chắc chắn
- Hệ thống đã được hiệu chuẩn đầy đủ
- Dây tín hiệu đạt yêu cầu kỹ thuật

Mọi thay đổi liên quan đến:

- Vị trí sensor
- Cấu trúc bia
- Sơ đồ kết nối
- Kết cấu cơ khí
- Hoặc cấu hình phần cứng

đều yêu cầu hiệu chuẩn và kiểm thử lại trước khi vận hành thực tế.

---

8. GIỚI HẠN THUẬT TOÁN

Hệ thống sử dụng:

- Thuật toán TDOA
- Mô hình Chan Method
- Tối ưu phi tuyến Levenberg-Marquardt

Trong điều kiện dữ liệu nhiễu, thiếu dữ liệu hoặc tín hiệu bất thường:

- Thuật toán có thể không hội tụ
- Có thể xuất hiện nghiệm sai
- Có thể phát sinh nghiệm không hợp lệ
- Hoặc hệ thống không thể xác định chính xác vị trí va chạm

Kết quả tính toán phụ thuộc trực tiếp vào chất lượng dữ liệu đầu vào.

---

9. GIỚI HẠN NGUỒN ĐIỆN

Nguồn cấp không ổn định có thể gây ra:

- Reset MCU ngoài ý muốn
- Sai lệch timestamp
- False trigger
- Lỗi DMA
- Mất dữ liệu
- Treo hệ thống
- Hoặc hư hỏng phần cứng

Hệ thống yêu cầu nguồn cấp ổn định và đạt tiêu chuẩn kỹ thuật trong suốt quá trình vận hành.

---

10. YÊU CẦU BẢO TRÌ

Trước khi sử dụng thực tế, hệ thống phải được:

- Kiểm tra định kỳ
- Hiệu chuẩn định kỳ
- Kiểm tra cảm biến
- Kiểm tra dây tín hiệu
- Kiểm tra nguồn cấp
- Xác minh firmware
- Kiểm tra đồng bộ timestamp
- Và chạy thử nghiệm chức năng

Mọi dấu hiệu bất thường phải được xử lý hoàn toàn trước khi tiếp tục vận hành.

---

11. GIỚI HẠN AN TOÀN VẬN HÀNH

Hệ thống này chỉ là công cụ hỗ trợ kỹ thuật.

Hệ thống không phải:

- Thiết bị xác nhận an toàn tuyệt đối
- Hệ thống điều khiển hỏa lực
- Thiết bị thay thế sĩ quan thao trường
- Hoặc hệ thống ra quyết định tự động

Trách nhiệm cuối cùng về an toàn luôn thuộc về:

- Người vận hành
- Cán bộ kỹ thuật
- Đơn vị triển khai
- Và chỉ huy thao trường

---

12. BÁO CÁO SỰ CỐ VÀ HỖ TRỢ

Mọi lỗi, sai lệch hoặc sự cố phát hiện trong quá trình vận hành phải được báo cáo ngay lập tức trước khi tiếp tục sử dụng hệ thống.

Thông tin liên hệ

- Tác giả: Chiêm Dũng (Dunghero1412)
- Repository: https://github.com/Dunghero1412/HTTDTD_v3.1
- Issues: https://github.com/Dunghero1412/HTTDTD_v3.1/issues

---

GHI NHỚ QUAN TRỌNG

Mọi hệ thống điện tử đều tồn tại khả năng phát sinh lỗi.

Độ chính xác của hệ thống phụ thuộc trực tiếp vào:

- Chất lượng triển khai
- Điều kiện vận hành
- Chất lượng hiệu chuẩn
- Và sự tuân thủ quy trình kỹ thuật

TUYỆT ĐỐI KHÔNG sử dụng hệ thống như nguồn xác nhận an toàn duy nhất trong thao trường bắn đạn thật.

---

Copyright © 2026 CHIÊM DŨNG