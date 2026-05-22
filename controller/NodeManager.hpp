// NodeManager.hpp
#pragma once
#include <QObject>
#include <QTimer>
#include <QMap>
#include <QString>
#include <array>
#include "ScoreCalculator.hpp"

struct NodeInfo {
    QString id;     // "1A"
    int col, row;   // 1-5, 0-3 (A=0,B=1,...)
    // Trạng thái hiện tại
    enum State { DEACTIVATED, ACTIVATED, MARKING, WARN } state = DEACTIVATED;
    int battery = -1;
    double temperature = 25.0;
    QString connectionInfo;
    // Dữ liệu 3 lần bắn
    struct Shot {
        bool valid = false;
        double x = 0, y = 0;
        int score = 0;
    } shots[3];
    int totalScore = 0;
    double avgX = 0, avgY = 0;
    QString classification;
    // Thời gian nhận STATUS cuối
    QDateTime lastStatusTime;
};

class UdpGateway;

class NodeManager : public QObject {
    Q_OBJECT
public:
    explicit NodeManager(UdpGateway *gateway, QObject *parent = nullptr);
    void initializeNodes();

    // Xử lý lệnh từ GUI: gửi UP/DOWN cho node(s)
    void sendUpDown(const QString &target, bool up); // target có thể là "1A", "1", "A", "MARKING"
    void sendStatusRequest();
    void clearScoreboard(); // reset bảng điểm, tăng lượt

    int sessionNumber() const { return m_sessionNumber; }

    // Truy cập dữ liệu node (cho GUI)
    const NodeInfo* nodeInfo(const QString &id) const;
    QList<QString> allNodeIDs() const;

signals:
    void scoreboardUpdated();   // báo GUI cập nhật bảng điểm
    void nodeStatusChanged();   // báo cập nhật trạng thái node
    void debugLog(const QString &msg, const QString &color); // gửi log

private slots:
    void onDataReceived(const QString &msg);
    void onStatusTimeout();     // kiểm tra node không phản hồi STATUS

private:
    void handleUplink(const QString &msg);
    void handleStatusMessage(const QStringList &parts);
    void evaluateNode(const QString &nodeId);
    NodeInfo* findNode(const QString &id);

    UdpGateway *m_gateway;
    QMap<QString, NodeInfo> m_nodes;
    int m_sessionNumber = 1;
    QTimer *m_statusTimer;
    static const int STATUS_TIMEOUT_SEC = 10;
    static const int STATUS_POLL_INTERVAL_MS = 5000;
};