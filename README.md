# IT3323 - Xây dựng chương trình dịch

## Sinh viên
Trần Ngọc Hưng - 20225635

# Bài thực hành 1: Xây dựng bảng chỉ dẫn từ văn bản

## Mô tả
Chương trình đọc một tệp văn bản và một tệp stop word, sau đó xây dựng bảng chỉ dẫn gồm:
- Từ (đã chuẩn hoá về chữ thường)
- Số lần xuất hiện của từ
- Danh sách các dòng mà từ xuất hiện (không trùng lặp)

Kết quả được ghi ra file `result.txt`.

## Cách sử dụng

### 1. Biên dịch
```powershell
make
```
Hoặc:
```powershell
gcc -Wall -O2 -o index.exe index.c
```

### 2. Chạy chương trình
```powershell
./index.exe stopw.txt alice30.txt
```
- `stopw.txt`: Tên file chứa stop words (mỗi dòng một từ)
- `alice30.txt`: Tên file văn bản cần lập chỉ mục

Kết quả sẽ được ghi ra file `result.txt`.

### 3. Dọn dẹp
```powershell
make clean
```

## Định dạng kết quả
Mỗi dòng trong `result.txt`:
```
từ số_lần_xuất_hiện, dòng1, dòng2, ...
```
Ví dụ:
```
were 6, 1, 3, 5, 7, 8
```

## Lưu ý
- Nếu không nhập đủ tham số, chương trình sẽ báo lỗi cú pháp.
- Nếu file không tồn tại, chương trình sẽ báo lỗi.
- Chỉ nhận đúng thứ tự: stopFile trước, textFile sau.
