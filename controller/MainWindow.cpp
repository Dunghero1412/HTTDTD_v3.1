// MainWindow.cpp
#include "MainWindow.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QTimer>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), m_gateway(new UdpGateway(this)), m_nodeManager(new NodeManager(m_gateway, this)) {
    setupUI();
    // Khởi tạo gateway UDP (giả sử packet forwarder trên localhost:1680 cho uplink, :1780 cho downlink)
    if (!m_gateway->start(1680, QHostAddress::LocalHost, 1780)) {
        m_logger->log("Failed to bind UDP port 1680", "red");
    } else {
        m_logger->log("UDP gateway started on port 1680", "blue");
    }
    m_nodeManager->initializeNodes();
    connect(m_nodeManager, &NodeManager::scoreboardUpdated, this, &MainWindow::updateScoreboard);
    connect(m_nodeManager, &NodeManager::nodeStatusChanged, this, &MainWindow::updateNodeStatus);
    connect(m_nodeManager, &NodeManager::debugLog, this, &MainWindow::appendDebugLog);

    // Trạng thái ban đầu các nút
    memset(m_nodeUpState, 0, sizeof(m_nodeUpState));
    m_markingActive = false;
}

MainWindow::~MainWindow() {}

void MainWindow::setupUI() {
    QWidget *central = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(central);

    // --- Bên trái: Bảng điểm (chiếm 1/3) ---
    QVBoxLayout *leftLayout = new QVBoxLayout();
    m_sessionLabel = new QLabel(QString("Lượt: %1").arg(m_nodeManager->sessionNumber()));
    m_sessionLabel->setStyleSheet("font-size:16px; font-weight:bold;");
    leftLayout->addWidget(m_sessionLabel);
    createScoreboardTable();
    leftLayout->addWidget(m_scoreTable);
    mainLayout->addLayout(leftLayout, 1);

    // --- Bên phải: 2/3 chia làm 3 phần dọc ---
    QVBoxLayout *rightLayout = new QVBoxLayout();
    // Phần trên: Debug log
    m_debugLog = new QTextEdit(this);
    m_debugLog->setReadOnly(true);
    m_logger = new Logger(m_debugLog, this);
    rightLayout->addWidget(m_debugLog, 1);

    // Phần giữa và dưới: chia ngang
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    // Trạng thái node (bên trái của bottom)
    createStatusTable();
    bottomLayout->addWidget(m_statusTable, 1);

    // Các nút điều khiển (bên phải của bottom)
    createControlButtons();
    bottomLayout->addLayout(createControlButtons(), 1);
    rightLayout->addLayout(bottomLayout, 2);
    mainLayout->addLayout(rightLayout, 2);

    setCentralWidget(central);
    setWindowTitle("TDOA Controller");
    resize(1200, 800);
}

void MainWindow::createScoreboardTable() {
    m_scoreTable = new QTableWidget(20, 11, this); // 20 nodes, 11 cột: Node, L1 Điểm, L1 X, L1 Y, L2 Điểm, L2 X, L2 Y, L3 Điểm, L3 X, L3 Y, Tổng điểm, Avg X, Avg Y, Xếp loại
    QStringList headers = {"Node", "L1 Điểm", "L1 X", "L1 Y", "L2 Điểm", "L2 X", "L2 Y", "L3 Điểm", "L3 X", "L3 Y", "Tổng điểm", "Avg X", "Avg Y", "Xếp loại"};
    m_scoreTable->setColumnCount(headers.size());
    m_scoreTable->setHorizontalHeaderLabels(headers);
    m_scoreTable->verticalHeader()->setVisible(false);
    m_scoreTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    // Điền danh sách node
    int row = 0;
    for (int col = 1; col <= 5; ++col) {
        for (char r = 'A'; r <= 'D'; ++r) {
            QString id = QString("%1%2").arg(col).arg(r);
            m_scoreTable->setItem(row, 0, new QTableWidgetItem(id));
            row++;
        }
    }
    m_scoreTable->resizeColumnsToContents();
}

void MainWindow::createStatusTable() {
    m_statusTable = new QTableWidget(20, 5, this);
    QStringList headers = {"Node", "Pin%", "Trạng thái", "Nhiệt độ", "Kết nối"};
    m_statusTable->setHorizontalHeaderLabels(headers);
    m_statusTable->verticalHeader()->setVisible(false);
    m_statusTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    int row = 0;
    for (int col = 1; col <= 5; ++col) {
        for (char r = 'A'; r <= 'D'; ++r) {
            QString id = QString("%1%2").arg(col).arg(r);
            m_statusTable->setItem(row, 0, new QTableWidgetItem(id));
            row++;
        }
    }
    m_statusTable->resizeColumnsToContents();
}

QLayout* MainWindow::createControlButtons() {
    QGridLayout *grid = new QGridLayout();
    // Col buttons
    for (int c = 1; c <= 5; ++c) {
        QPushButton *btn = new QPushButton(QString::number(c), this);
        btn->setFixedSize(40,40);
        btn->setStyleSheet("background-color: gray; color: white;");
        connect(btn, &QPushButton::clicked, this, &MainWindow::onColButtonClicked);
        m_colButtons[c-1] = btn;
        grid->addWidget(btn, 0, c);
    }
    // Row buttons
    for (int r = 0; r < 4; ++r) {
        QPushButton *btn = new QPushButton(QString(QChar('A' + r)), this);
        btn->setFixedSize(40,40);
        btn->setStyleSheet("background-color: gray; color: white;");
        connect(btn, &QPushButton::clicked, this, &MainWindow::onRowButtonClicked);
        m_rowButtons[r] = btn;
        grid->addWidget(btn, r+1, 0);
    }
    // 20 node buttons
    for (int col = 0; col < 5; ++col) {
        for (int row = 0; row < 4; ++row) {
            QPushButton *btn = new QPushButton(QString("%1%2").arg(col+1).arg(QChar('A'+row)), this);
            btn->setFixedSize(45,45);
            btn->setStyleSheet("background-color: gray; color: white;");
            connect(btn, &QPushButton::clicked, this, &MainWindow::onNodeButtonClicked);
            m_nodeButtons[col][row] = btn;
            grid->addWidget(btn, row+1, col+1);
        }
    }
    // Special buttons
    m_markingBtn = new QPushButton("MARKING\n(DEACTIVATED)", this);
    m_markingBtn->setStyleSheet("background-color: blue; color: white;");
    connect(m_markingBtn, &QPushButton::clicked, this, &MainWindow::onMarkingClicked);
    grid->addWidget(m_markingBtn, 5, 0, 1, 2);

    m_statusBtn = new QPushButton("STATUS", this);
    m_statusBtn->setStyleSheet("background-color: gray; color: white;");
    connect(m_statusBtn, &QPushButton::clicked, this, &MainWindow::onStatusClicked);
    grid->addWidget(m_statusBtn, 5, 2);

    m_clearBtn = new QPushButton("CLEAR", this);
    m_clearBtn->setStyleSheet("background-color: gray; color: white;");
    connect(m_clearBtn, &QPushButton::clicked, this, &MainWindow::onClearClicked);
    grid->addWidget(m_clearBtn, 5, 3);

    m_autoClearCheck = new QCheckBox("auto", this);
    grid->addWidget(m_autoClearCheck, 5, 4);

    return grid;
}

void MainWindow::onNodeButtonClicked() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    QString text = btn->text();
    // Tìm vị trí
    for (int c = 0; c < 5; ++c) {
        for (int r = 0; r < 4; ++r) {
            if (m_nodeButtons[c][r] == btn) {
                bool &up = m_nodeUpState[c][r];
                up = !up; // toggle
                QString cmd = QString("%1%2").arg(c+1).arg(QChar('A'+r));
                m_nodeManager->sendUpDown(cmd, up);
                if (up) {
                    btn->setStyleSheet("background-color: green; color: black;");
                    // Tự động chuyển về IDLE sau 70s (nếu không có DOWN thủ công)
                    QTimer::singleShot(70000, this, [this, btn, c, r]() {
                        if (m_nodeUpState[c][r]) { // vẫn đang UP
                            m_nodeUpState[c][r] = false;
                            btn->setStyleSheet("background-color: red; color: black;");
                            QTimer::singleShot(5000, btn, [btn]() {
                                btn->setStyleSheet("background-color: gray; color: white;");
                            });
                        }
                    });
                } else {
                    btn->setStyleSheet("background-color: red; color: black;");
                    QTimer::singleShot(5000, btn, [btn]() {
                        btn->setStyleSheet("background-color: gray; color: white;");
                    });
                }
                break;
            }
        }
    }
}

void MainWindow::onColButtonClicked() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    int col = btn->text().toInt();
    // Gửi UP? Mặc định UP, nhưng có thể toggle trạng thái broadcast không cần.
    m_nodeManager->sendUpDown(QString::number(col), true); // UP broadcast
}

void MainWindow::onRowButtonClicked() {
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    QString row = btn->text();
    m_nodeManager->sendUpDown(row, true);
}

void MainWindow::onMarkingClicked() {
    m_markingActive = !m_markingActive;
    m_nodeManager->sendUpDown("MARKING", m_markingActive);
    if (m_markingActive) {
        m_markingBtn->setText("MARKING (ACTIVATED)");
        m_markingBtn->setStyleSheet("background-color: red; color: black;");
        // Khóa các nút khác
        for (int c = 0; c < 5; ++c) {
            m_colButtons[c]->setEnabled(false);
            m_rowButtons[c<4?c:0]->setEnabled(false);
            for (int r = 0; r < 4; ++r) m_nodeButtons[c][r]->setEnabled(false);
        }
    } else {
        m_markingBtn->setText("MARKING (DEACTIVATED)");
        m_markingBtn->setStyleSheet("background-color: blue; color: white;");
        for (int c = 0; c < 5; ++c) {
            m_colButtons[c]->setEnabled(true);
            m_rowButtons[c<4?c:0]->setEnabled(true);
            for (int r = 0; r < 4; ++r) m_nodeButtons[c][r]->setEnabled(true);
        }
        if (m_autoClearCheck->isChecked()) {
            onClearClicked();
        }
    }
}

void MainWindow::onStatusClicked() {
    m_nodeManager->sendStatusRequest();
}

void MainWindow::onClearClicked() {
    m_nodeManager->clearScoreboard();
    m_sessionLabel->setText(QString("Lượt: %1").arg(m_nodeManager->sessionNumber()));
    // Reset tất cả nút về IDLE
    for (int c = 0; c < 5; ++c) {
        for (int r = 0; r < 4; ++r) {
            m_nodeUpState[c][r] = false;
            m_nodeButtons[c][r]->setStyleSheet("background-color: gray; color: white;");
        }
    }
    m_markingActive = false;
    m_markingBtn->setText("MARKING (DEACTIVATED)");
    m_markingBtn->setStyleSheet("background-color: blue; color: white;");
    // Bỏ lock
    for (int c = 0; c < 5; ++c) {
        m_colButtons[c]->setEnabled(true);
        m_rowButtons[c<4?c:0]->setEnabled(true);
        for (int r = 0; r < 4; ++r) m_nodeButtons[c][r]->setEnabled(true);
    }
}

void MainWindow::updateScoreboard() {
    // Cập nhật bảng điểm từ NodeManager
    const auto ids = m_nodeManager->allNodeIDs();
    for (int i = 0; i < ids.size(); ++i) {
        const NodeInfo *info = m_nodeManager->nodeInfo(ids[i]);
        int row = i; // giả sử thứ tự tương ứng
        // Lần 1
        if (info->shots[0].valid) {
            m_scoreTable->setItem(row, 1, new QTableWidgetItem(QString::number(info->shots[0].score)));
            m_scoreTable->setItem(row, 2, new QTableWidgetItem(QString::number(info->shots[0].x, 'f', 1)));
            m_scoreTable->setItem(row, 3, new QTableWidgetItem(QString::number(info->shots[0].y, 'f', 1)));
        } else {
            m_scoreTable->setItem(row, 1, new QTableWidgetItem(""));
            m_scoreTable->setItem(row, 2, new QTableWidgetItem("miss"));
            m_scoreTable->setItem(row, 3, new QTableWidgetItem(""));
        }
        // Lần 2, 3 tương tự cho cột 4-6, 7-9
        // Tổng và trung bình
        m_scoreTable->setItem(row, 10, new QTableWidgetItem(QString::number(info->totalScore)));
        m_scoreTable->setItem(row, 11, new QTableWidgetItem(QString::number(info->avgX, 'f', 1)));
        m_scoreTable->setItem(row, 12, new QTableWidgetItem(QString::number(info->avgY, 'f', 1)));
        m_scoreTable->setItem(row, 13, new QTableWidgetItem(info->classification));
    }
}

void MainWindow::updateNodeStatus() {
    const auto ids = m_nodeManager->allNodeIDs();
    for (int i = 0; i < ids.size(); ++i) {
        const NodeInfo *info = m_nodeManager->nodeInfo(ids[i]);
        int row = i;
        m_statusTable->setItem(row, 1, new QTableWidgetItem(QString::number(info->battery)));
        QString stateStr;
        switch (info->state) {
            case NodeInfo::DEACTIVATED: stateStr = "DEACTIVATED"; break;
            case NodeInfo::ACTIVATED: stateStr = "ACTIVATED"; break;
            case NodeInfo::MARKING: stateStr = "MARKING"; break;
            case NodeInfo::WARN: stateStr = "WARN"; break;
        }
        m_statusTable->setItem(row, 2, new QTableWidgetItem(stateStr));
        m_statusTable->setItem(row, 3, new QTableWidgetItem(QString::number(info->temperature, 'f', 1)));
        m_statusTable->setItem(row, 4, new QTableWidgetItem(info->connectionInfo));
    }
}

void MainWindow::appendDebugLog(const QString &msg, const QString &color) {
    m_logger->log(msg, color);
}