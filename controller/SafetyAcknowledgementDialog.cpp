// SafetyAcknowledgementDialog.cpp
#include "SafetyAcknowledgementDialog.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QApplication>
#include <QScrollBar>
#include <QDebug>
#include <QScreen>

// ============================================================================
// Constructor: Khởi tạo dialog
// ============================================================================
SafetyAcknowledgementDialog::SafetyAcknowledgementDialog(QWidget *parent)
    : QDialog(parent), m_countdownTimer(nullptr) {
    
    // Dialog không thể đóng bằng nút X
    setWindowFlags(windowFlags() & ~Qt::WindowCloseButtonHint);
    setModal(true);
    
    // Đọc 3 tài liệu từ disk
    loadSafetyDocuments();
    
    // Khởi tạo UI
    setupUI();
    
    // Thiết lập kích thước dialog (80% màn hình)
    QScreen *screen = QApplication::primaryScreen();
    int width = screen->geometry().width() * 0.8;
    int height = screen->geometry().height() * 0.85;
    resize(width, height);
    
    // Đặt lại từ đầu
    m_secondsRemaining = 30;
    m_scrolledToBottom = false;
    m_timerExpired = false;
    
    updateButtonState();
}

SafetyAcknowledgementDialog::~SafetyAcknowledgementDialog() {
    if (m_countdownTimer) {
        m_countdownTimer->stop();
        delete m_countdownTimer;
    }
}

// ============================================================================
// Đọc 3 file DISCLAIMER, SAFETY_WARNING, OPERATION_LIMITATIONS
// ============================================================================
void SafetyAcknowledgementDialog::loadSafetyDocuments() {
    // Đường dẫn tới file (từ thư mục gốc project)
    QString baseDir = QApplication::applicationDirPath();
    // Hoặc có thể hardcode đường dẫn tuyệt đối nếu cần
    
    // DEBUG: In ra đường dẫn
    qDebug() << "Loading safety documents from:" << baseDir;
    
    // Load DISCLAIMER.md
    QFile disclaimerFile(baseDir + "/../DISCLAIMER.md");
    if (disclaimerFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&disclaimerFile);
        in.setCodec("UTF-8");
        m_disclaimerText = in.readAll();
        disclaimerFile.close();
        qDebug() << "DISCLAIMER.md loaded successfully";
    } else {
        m_disclaimerText = "⚠️ KHÔNG THỂ ĐỌC FILE DISCLAIMER.md\n\n"
                          "Kiểm tra xem file có tồn tại tại:\n" + 
                          (baseDir + "/../DISCLAIMER.md");
        qWarning() << "Failed to load DISCLAIMER.md";
    }
    
    // Load SAFETY_WARNING.md
    QFile safetyFile(baseDir + "/../SAFETY_WARNING.md");
    if (safetyFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&safetyFile);
        in.setCodec("UTF-8");
        m_safetyWarningText = in.readAll();
        safetyFile.close();
        qDebug() << "SAFETY_WARNING.md loaded successfully";
    } else {
        m_safetyWarningText = "⚠️ KHÔNG THỂ ĐỌC FILE SAFETY_WARNING.md";
        qWarning() << "Failed to load SAFETY_WARNING.md";
    }
    
    // Load OPERATION_LIMITATIONS.md
    QFile operationFile(baseDir + "/../OPENRATION_LIMITATIONS.md");
    if (operationFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&operationFile);
        in.setCodec("UTF-8");
        m_operationLimitText = in.readAll();
        operationFile.close();
        qDebug() << "OPERATION_LIMITATIONS.md loaded successfully";
    } else {
        m_operationLimitText = "⚠️ KHÔNG THỂ ĐỌC FILE OPERATION_LIMITATIONS.md";
        qWarning() << "Failed to load OPERATION_LIMITATIONS.md";
    }
}

// ============================================================================
// Khởi tạo UI
// ============================================================================
void SafetyAcknowledgementDialog::setupUI() {
    setWindowTitle("⚠️ CẢNH BÁO AN TOÀN - PHẢI ĐỌC TRƯỚC KHI TIẾP TỤC");
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(10);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    
    // ========================================================================
    // Phần 1: Tiêu đề cảnh báo (Có nền đỏ)
    // ========================================================================
    QLabel *warningLabel = new QLabel(
        "⚠️ CẢNH BÁO AN TOÀN HỆ THỐNG ⚠️\n"
        "Bạn phải đọc và hiểu toàn bộ tài liệu dưới đây trước khi vận hành hệ thống!\n"
        "Hệ thống này hoạt động với đạn thật - Có nguy cơ gây thương tích hoặc tử vong!",
        this
    );
    warningLabel->setStyleSheet(
        "background-color: #FF4444; "
        "color: white; "
        "font-weight: bold; "
        "font-size: 12px; "
        "padding: 10px; "
        "border-radius: 5px;"
    );
    warningLabel->setWordWrap(true);
    warningLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(warningLabel);
    
    // ========================================================================
    // Phần 2: Text area hiển thị 3 tài liệu
    // ========================================================================
    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setStyleSheet(
        "QTextEdit { "
        "    border: 2px solid #333; "
        "    background-color: #f9f9f9; "
        "    font-size: 10px; "
        "    font-family: 'Courier New'; "
        "}"
    );
    
    // Kết hợp 3 tài liệu với dấu phân cách
    QString combinedText = 
        "═══════════════════════════════════════════════════════════════════\n"
        "1. TUYÊN BỐ MIỄN TRỪ TRÁCH NHIỆM (DISCLAIMER)\n"
        "═══════════════════════════════════════════════════════════════════\n\n" +
        m_disclaimerText +
        "\n\n═══════════════════════════════════════════════════════════════════\n"
        "2. CẢNH BÁO AN TOÀN (SAFETY WARNING)\n"
        "═══════════════════════════════════════════════════════════════════\n\n" +
        m_safetyWarningText +
        "\n\n═══════════════════════════════════════════════════════════════════\n"
        "3. GIỚI HẠN VẬN HÀNH (OPERATION LIMITATIONS)\n"
        "═══════════════════════════════════════════════════════════════════\n\n" +
        m_operationLimitText +
        "\n\n═══════════════════════════════════════════════════════════════════\n"
        "CUỐI CỰ - BẠN PHẢI LƯỚT ĐẾN ĐÂY\n"
        "═══════════════════════════════════════════════════════════════════\n";
    
    m_textEdit->setText(combinedText);
    
    // ← Khi user cuộn, kiểm tra xem đã đến cuối chưa
    connect(m_textEdit->verticalScrollBar(), &QScrollBar::valueChanged,
            this, &SafetyAcknowledgementDialog::onTextScrolled);
    
    mainLayout->addWidget(m_textEdit, 1);
    
    // ========================================================================
    // Phần 3: Status label (Trạng thái và countdown)
    // ========================================================================
    m_statusLabel = new QLabel(
        "❌ Bạn phải:\n"
        "   1. Lướt hết tài liệu (cuộn xuống cuối)\n"
        "   2. Đợi 30 giây",
        this
    );
    m_statusLabel->setStyleSheet(
        "background-color: #FFEE99; "
        "color: #333333; "
        "font-weight: bold; "
        "font-size: 11px; "
        "padding: 8px; "
        "border-radius: 3px;"
    );
    m_statusLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(m_statusLabel);
    
    // ========================================================================
    // Phần 4: Nút hành động
    // ========================================================================
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(10);
    
    // Nút xác nhận (khóa lúc đầu)
    m_confirmBtn = new QPushButton("✓ Tôi đã hiểu rủi ro - Tiếp tục", this);
    m_confirmBtn->setStyleSheet(
        "QPushButton { "
        "    background-color: #CCCCCC; "
        "    color: #666666; "
        "    font-weight: bold; "
        "    font-size: 11px; "
        "    padding: 10px; "
        "    border-radius: 5px; "
        "} "
        "QPushButton:hover:enabled { "
        "    background-color: #00CC00; "
        "    color: white; "
        "}"
    );
    m_confirmBtn->setFixedHeight(40);
    m_confirmBtn->setEnabled(false);  // ← Khóa lúc đầu
    connect(m_confirmBtn, &QPushButton::clicked, this, &SafetyAcknowledgementDialog::onAcknowledgeClicked);
    buttonLayout->addWidget(m_confirmBtn, 2);
    
    // Nút từ chối
    m_rejectBtn = new QPushButton("✕ Thoát chương trình", this);
    m_rejectBtn->setStyleSheet(
        "QPushButton { "
        "    background-color: #FF6666; "
        "    color: white; "
        "    font-weight: bold; "
        "    font-size: 11px; "
        "    padding: 10px; "
        "    border-radius: 5px; "
        "} "
        "QPushButton:hover { "
        "    background-color: #CC0000; "
        "}"
    );
    m_rejectBtn->setFixedHeight(40);
    connect(m_rejectBtn, &QPushButton::clicked, this, &SafetyAcknowledgementDialog::onRejectClicked);
    buttonLayout->addWidget(m_rejectBtn, 1);
    
    mainLayout->addLayout(buttonLayout);
    
    setLayout(mainLayout);
}

// ============================================================================
// Kiểm tra user đã cuộn đến cuối tài liệu chưa
// ============================================================================
bool SafetyAcknowledgementDialog::isScrolledToBottom() const {
    QScrollBar *scrollBar = m_textEdit->verticalScrollBar();
    // Nếu vị trí = giá trị max thì đã ở cuối
    return (scrollBar->value() >= scrollBar->maximum() - 10);  // ← Cho phép sai số 10px
}

// ============================================================================
// Slot: Khi user cuộn text
// ============================================================================
void SafetyAcknowledgementDialog::onTextScrolled() {
    // Kiểm tra nếu đã cuộn đến cuối
    if (isScrolledToBottom() && !m_scrolledToBottom) {
        m_scrolledToBottom = true;
        qDebug() << "User scrolled to bottom - Starting 30 second countdown";
        
        // Bắt đầu timer đếm ngược 30 giây
        if (!m_countdownTimer) {
            m_countdownTimer = new QTimer(this);
            connect(m_countdownTimer, &QTimer::timeout, this, &SafetyAcknowledgementDialog::onTimerTick);
        }
        m_secondsRemaining = 30;
        m_countdownTimer->start(1000);  // ← Cứ 1 giây gọi onTimerTick
    }
    
    updateButtonState();
}

// ============================================================================
// Slot: Timer tick - Cập nhật countdown
// ============================================================================
void SafetyAcknowledgementDialog::onTimerTick() {
    m_secondsRemaining--;
    
    if (m_secondsRemaining <= 0) {
        m_countdownTimer->stop();
        m_timerExpired = true;
        qDebug() << "30 seconds countdown completed - Confirm button enabled";
    }
    
    updateButtonState();
}

// ============================================================================
// Cập nhật trạng thái nút và label
// ============================================================================
void SafetyAcknowledgementDialog::updateButtonState() {
    // Kiểm tra điều kiện để unlock nút xác nhận
    bool canConfirm = m_scrolledToBottom && m_timerExpired;
    
    m_confirmBtn->setEnabled(canConfirm);
    
    // Cập nhật status label
    if (!m_scrolledToBottom) {
        m_statusLabel->setText(
            "❌ Bạn phải lướt hết tài liệu (cuộn xuống cuối)"
        );
        m_statusLabel->setStyleSheet(
            "background-color: #FFEE99; color: #333333; font-weight: bold; font-size: 11px; padding: 8px; border-radius: 3px;"
        );
    } else if (!m_timerExpired) {
        m_statusLabel->setText(
            QString("⏱️ Bạn đã lướt hết tài liệu!\n"
                    "Vui lòng chờ %1 giây trước khi nhấn xác nhận...").arg(m_secondsRemaining)
        );
        m_statusLabel->setStyleSheet(
            "background-color: #FFDD44; color: #333333; font-weight: bold; font-size: 11px; padding: 8px; border-radius: 3px;"
        );
    } else {
        m_statusLabel->setText(
            "✅ Bạn có thể nhấn nút 'Tôi đã hiểu rủi ro' để tiếp tục"
        );
        m_statusLabel->setStyleSheet(
            "background-color: #99FF99; color: #006600; font-weight: bold; font-size: 11px; padding: 8px; border-radius: 3px;"
        );
    }
}

// ============================================================================
// Slot: Nút xác nhận được ấn
// ============================================================================
void SafetyAcknowledgementDialog::onAcknowledgeClicked() {
    qDebug() << "User acknowledged all safety documents";
    // Trả về QDialog::Accepted (1)
    accept();
}

// ============================================================================
// Slot: Nút từ chối được ấn
// ============================================================================
void SafetyAcknowledgementDialog::onRejectClicked() {
    qDebug() << "User rejected safety documents - Exiting application";
    // Trả về QDialog::Rejected (0)
    reject();
}
