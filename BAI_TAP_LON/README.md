# bài tập lớn môn lập trình mạng
## Server có các chức năng sau:
- Nhận yêu cầu tìm kiếm file (theo tên file) của client A nào đó
- Gửi lệnh tìm kiếm tới các client khác đang kết nối
- Gửi lại cho client A danh sách các client có file mà client A tìm kiếm
- Chuyển tiếp file khi client A yêu cầu 
## Client có các chức năng sau:
- Gửi yêu cầu tìm kiếm một file lên server
- Nhận danh sách  các client có file mà client yêu cầu tìm kiếm
- Lựa chọn một client trong kết quả server gửi về để yêu cầu download file
