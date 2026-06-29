// MainWindow.hpp
#pragma once
#include <QMainWindow>
#include <QTableWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QCheckBox>
#include <QLabel>
#include "UdpGateway.hpp"
#include "NodeManager.hpp"
#include "Logger.hpp"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNodeButtonClicked();     // cho 20 nút cụ thể
    void onColButtonClicked();
    void onRowButtonClicked();
    void onMarkingClicked();
    void onStatusClicked();
    void onClearClicked();
    void updateScoreboard();
    void updateNodeStatus();
    void appendDebugLog(const QString &msg, const QString &color);

private:
    void setupUI();
    void createScoreboardTable();
    void createStatusTable();
    //void createControlButtons();
    QLayout* createControlButtons();
    QPushButton* createNodeButton(const QString &text);

    // UDP và logic
    UdpGateway *m_gateway;
    NodeManager *m_nodeManager;
    Logger *m_logger;

    // Widgets chính
    QTableWidget *m_scoreTable;    // bảng điểm
    QTextEdit *m_debugLog;         // debug log
    QTableWidget *m_statusTable;   // trạng thái node
    // Các nút điều khiển
    QPushButton *m_colButtons[5];
    QPushButton *m_rowButtons[4];
    QPushButton *m_nodeButtons[5][4]; // 5 cột x 4 hàng
    QPushButton *m_markingBtn;
    QPushButton *m_statusBtn;
    QPushButton *m_clearBtn;
    QCheckBox *m_autoClearCheck;
    QLabel *m_sessionLabel;

    // Trạng thái nút (UP/DOWN)
    bool m_nodeUpState[5][4]; // true nếu đang UP
    bool m_markingActive = false;
};
