# TUYÊN BỐ MIỄN TRỪ TRÁCH NHIỆM (DISCLAIMER)

Hệ thống này là nền tảng thử nghiệm phục vụ:
- huấn luyện
- đánh giá kỹ thuật
- và hỗ trợ xác định tọa độ điểm va chạm bằng phương pháp TDOA (Time Difference Of Arrival).

Hệ thống sử dụng:
- cảm biến Piezoelectric,
- mạch xử lý tín hiệu tương tự,
- bộ định thời gian độ chính xác cao,
- và thuật toán xử lý tín hiệu phi tuyến
để ước lượng vị trí va chạm của đầu đạn.

## ⚠️ CẢNH BÁO QUAN TRỌNG - AN TOÀN THAO TRƯỜNG

**HỆ THỐNG CÓ THỂ PHÁT SINH LỖI CÓ THỂ GẬY THIỆT HẠI LỚN HOẶC MẤT AN TOÀN THAO TRƯỜNG**

Các rủi ro tiềm ẩn bao gồm:
- **Sai lệch kết quả định vị:** Có khả năng cung cấp tọa độ không chính xác, dẫn đến:
  - Xác định sai vị trí va chạm đầu đạn
  - Cho phép đạn gây sát thương tính mạng khi không phát hiện đúng vị trí
  - Mất kiểm soát khu vực an toàn thao trường
- **Lỗi cảm biến hoặc xử lý tín hiệu:** Có thể không phát hiện va chạm hoặc phát hiện sai mục tiêu
- **Lỗi do chủ quan:** có thể gây mất an toàn tính mạng do việc tuân thủ kỹ luật thao trường bị vi phạm nghiêm trọng
- **Không đảm bảo độ tin cậy 100%:** KHÔNG được xem như thiết bị chính để xác nhận an toàn

- TÔI - người phát triển dự án xin TUYÊN BỐ MIỄN TRỪ TRÁCH NHIỆM đối với bất kỳ rủi ro, tai nạn, thiệt hại tài sản, nhân mạng, uy tín, kết quả do việc sử dugj mã nguồn gây ra. toàn bộ dự án đều được cấp phép theo giấy phép mã nguồn mở Apache 2.0 và được cung cấp **"NGUYÊN TRẠNG" (AS-IS)**, mọi cá nhân, đơn vị, tổ chức sử dụng mã nguồn đều phải tự kiểm tra, thử nghiệm, lập kế hoạch, quán triệt nội quy, quy định, kỹ luật liên quan đến việc sử dụng thiết bị và mã nguồn.

- TÔI - không chịu bất kỳ trách nhiệm nào dù là diễn đạt hay ngụ ý liên quan đến bất kỳ thiệt hại nào do việc sử dụng mã nguồn và thiết bị gây ra.
- NGƯỜI VẬN HÀNH - phải tự đảm bảo mọi chức năng của mã nguồn, thiết bị đã được hiệu chỉnh, kiểm tra và các vấn đề liên quan dến an toàn thao trường, trường bắn đã nằm trong tầm kiểm soát trước khi kích hoạt thiết bị.

## LƯU Ý QUAN TRỌNG

Kết quả tọa độ do hệ thống cung cấp chỉ mang tính:
- hỗ trợ kỹ thuật,
- tham khảo,
- và ước lượng.

Hệ thống KHÔNG đảm bảo độ chính xác tuyệt đối trong mọi điều kiện vận hành.

Sai số có thể xuất hiện do:
- nhiễu môi trường,
- nhiệt độ,
- độ ẩm,
- rung động cơ học,
- sai số cảm biến,
- sai số đồng bộ thời gian,
- nhiễu điện,
- cấu hình phần cứng,
- đặc tính từng loại đạn,
- lỗi firmware hoặc phần mềm,
- hiệu chuẩn không đúng,
- hoặc lỗi vận hành.

## GIỚI HẠN TRÁCH NHIỆM

Đơn vị phát triển, vận hành hoặc bàn giao hệ thống KHÔNG chịu trách nhiệm đối với:
- **Thiệt hại tính mạng:** Thương tích nhân viên, tai nạn thao trường chết người do vận hành sai quy trình đã được ban hành, vi phạm kỹ luật liên quan đến vấn đề đảm bảo an toàn nhân mạng trên thao trường.
- **Thiệt hại tài sản:** Phá hủy cơ sở vật chất, thiết bị, hoặc hạ tầng do xạ thủ bắn lệch hoặc việc lắp đặt sai quy trình lắp đặt chuẩn đã được ban hành.
- **Tai nạn thao trường:** Cháy nổ, mất kiểm soát thao trường, hoặc mất an toàn do không tuân thủ nội quy, quy định và kỹ luật.
- **Sai lệch kết quả:** Mất kiểm soát quỹ đạo đạn, không xác định được vị trí va chạm thực tế do hiệu chỉnh , chỉnh sửa mã nguồn, lắp đặt, bố trí node hoặc sensor không tuân thủ theo quy trình và phương thức lắp đặt được ban hành.
- **Mất an toàn:** Lỗi hệ thống dẫn đến không duy trì được vùng an toàn thao trường hoặc tác nhân do con người gây ra.
- Hoặc các hậu quả phát sinh khác
do:
- vận hành sai quy trình,
- tự ý chỉnh sửa hệ thống mà không hiệu chuẩn lại,
- sử dụng sai mục đích,
- vi phạm quy tắc an toàn thao trường,
- triển khai ngoài điều kiện cho phép,
- hoặc phần cứng/firmware lỗi từ nhà sản xuất.

## KHÔNG ĐƯỢC SỬ DỤNG HỆ THỐNG CHO CÁC MỤC ĐÍCH SAU

- **Hệ thống điều khiển hỏa lực tự động.** Hệ thống KHÔNG có khả năng điều khiển hoặc dừng bắn tự động.
- **Hệ thống xác nhận mục tiêu sống/chết.** Không được sử dụng để quyết định bắn hoặc dừng bắn.
- **Thiết bị thay thế sĩ quan thao trường.** Không được xem như sĩ quan kiểm tra an toàn thay thế.
- **Thiết bị thay thế quy trình kiểm tra an toàn bắn.** Quy trình kiểm tra an toàn PHẢI được thực hiện bằng con người.
- **Thiết bị xác nhận tuyệt đối vị trí đầu đạn.** Chỉ mang tính tham khảo, KHÔNG phải xác định cuối cùng.
- **Quyết định duy nhất để cho phép thao trường.** PHẢI luôn có kiểm tra thủ công thêm.

## YÊU CẦU BẮT BUỘC

Mọi sửa đổi liên quan đến:
- phần cứng,
- firmware,
- phần mềm,
- thuật toán,
- timer,
- prescaler,
- comparator threshold,
- vị trí cảm biến,
- hiệu chuẩn,
- sơ đồ kết nối,
- hoặc cấu hình bất kỳ

ĐỀU PHẢI:
- được kiểm thử lại đầy đủ,
- hiệu chuẩn lại tại điều kiện thực tế,
- đánh giá an toàn đầy đủ,
- lập biên bản xác nhận,
- và được sự phê duyệt từ chỉ huy thao trường
trước khi đưa vào vận hành thực tế.

## KẾT LUẬN

**HỆ THỐNG ĐIỆN TỬ LUÔN CÓ KHỨC NĂNG XẢY RA LỖI.**

**An toàn thao trường PHẢI luôn được đảm bảo bởi:**
- **Con người:** Sĩ quan thao trường, nhân viên kỹ thuật, giám sát viên
- **Quy trình quân sự:** Kiểm tra an toàn, quy định thao trường, thủ tục bắn
- **Giám sát trực tiếp:** Không được để hệ thống làm việc tự động mà không giám sát

**TUYỆT ĐỐI KHÔNG phụ thuộc hoàn toàn vào hệ thống điện tử.**

---

**Ngày cập nhật:** 2026-05-27

**Trách nhiệm:** Bất kỳ ai sử dụng hệ thống này phải chịu trách nhiệm pháp lý về an toàn thao trường và bất kỳ thiệt hại nào phát sinh.
