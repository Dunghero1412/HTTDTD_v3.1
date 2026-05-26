// SafetyAcknowledgementDialog.hpp
#pragma once
#include <QDialog>
#include <QPushButton>
#include <QTextEdit>
#include <QLabel>
#include <QTimer>
#include <QScrollArea>

// ============================================================================
// Dialog yêu cầu đọc 3 file DISCLAIMER, SAFETY_WARNING, OPERATION_LIMITATIONS
// Người dùng PHẢI:
// 1. Lướt đến cuối tài liệu
// 2. Chờ ít nhất 30 giây
// 3. Mới được ấn nút "Tôi đã hiểu rủi ro"
// ============================================================================

class SafetyAcknowledgementDialog : public QDialog {
    Q_OBJECT
public:
    explicit SafetyAcknowledgementDialog(QWidget *parent = nullptr);
    ~SafetyAcknowledgementDialog();

private slots:
    void onTextScrolled();          // ← Khi user lướt
    void onTimerTick();             // ← Cập nhật countdown (30s)
    void onAcknowledgeClicked();    // ← Ấn nút xác nhận
    void onRejectClicked();         // ← Ấn nút từ chối

private:
    void setupUI();                 // ← Khởi tạo layout
    void loadSafetyDocuments();     // ← Đọc 3 file từ disk
    void updateButtonState();       // ← Enable/Disable nút "Xác nhận"
    bool isScrolledToBottom() const; // ← Kiểm tra user đã lướt đến cuối?

    // UI Widgets
    QTextEdit *m_textEdit;          // Hiển thị 3 file
    QLabel *m_statusLabel;          // Trạng thái: "Bạn phải lướt hết tài liệu..." hoặc "Chờ X giây"
    QPushButton *m_confirmBtn;      // Nút "Tôi đã hiểu rủi ro" (khóa lúc đầu)
    QPushButton *m_rejectBtn;       // Nút "Thoát chương trình"

    // Timer & State
    QTimer *m_countdownTimer;       // Timer đếm ngược 30 giây
    int m_secondsRemaining = 30;    // Số giây còn lại
    bool m_scrolledToBottom = false; // User đã lướt đến cuối?
    bool m_timerExpired = false;    // 30 giây đã hết?

    // Dữ liệu
    QString m_disclaimerText;       // Nội dung DISCLAIMER.md
    QString m_safetyWarningText;    // Nội dung SAFETY_WARNING.md
    QString m_operationLimitText;   // Nội dung OPERATION_LIMITATIONS.md
};
