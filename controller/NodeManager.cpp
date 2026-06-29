// NodeManager.cpp
#include "NodeManager.hpp"
#include "UdpGateway.hpp"
#include <QDateTime>
#include <QDebug>

NodeManager::NodeManager(UdpGateway *gateway, QObject *parent)
    : QObject(parent), m_gateway(gateway)
{
    connect(gateway, &UdpGateway::dataReceived, this, &NodeManager::onDataReceived);
    m_statusTimer = new QTimer(this);
    m_statusTimer->setInterval(STATUS_POLL_INTERVAL_MS);
    connect(m_statusTimer, &QTimer::timeout, this, &NodeManager::onStatusTimeout);
    m_statusTimer->start();
}

void NodeManager::initializeNodes() {
    for (int col = 1; col <= 5; ++col) {
        for (char row = 'A'; row <= 'D'; ++row) {
            QString id = QString("%1%2").arg(col).arg(row);
            NodeInfo info;
            info.id = id;
            info.col = col;
            info.row = row - 'A';
            m_nodes[id] = info;
        }
    }
}

void NodeManager::sendUpDown(const QString &target, bool up) {
    QString cmd = target + (up ? ",UP" : ",DOWN");
    m_gateway->sendCommand(cmd);
    emit debugLog(QString("SEND: %1").arg(cmd), "green");
    // Cập nhật trạng thái nội bộ nếu là node cụ thể hoặc marking
    if (target == "MARKING") {
        QString markingState = up ? "MARKING" : "DEACTIVATED";
        for (auto &node : m_nodes) {
            node.state = up ? NodeInfo::MARKING : NodeInfo::DEACTIVATED;
        }
        emit nodeStatusChanged();
    } else if (target.size() == 2 && target[0].isDigit() && target[1].isLetter()) {
        NodeInfo *node = findNode(target);
        if (node) {
            node->state = up ? NodeInfo::ACTIVATED : NodeInfo::DEACTIVATED;
            emit nodeStatusChanged();
        }
    }
    // Với broadcast (col/row), không thay đổi trạng thái cụ thể, chỉ gửi lệnh.
}

void NodeManager::sendStatusRequest() {
    m_gateway->sendCommand("STATUS");
    emit debugLog("SEND: STATUS", "green");
}

void NodeManager::clearScoreboard() {
    m_sessionNumber++;
    for (auto &node : m_nodes) {
        for (int i = 0; i < 3; ++i) {
            node.shots[i] = NodeInfo::Shot();
        }
        node.totalScore = 0;
        node.avgX = node.avgY = 0;
        node.classification.clear();
    }
    emit scoreboardUpdated();
    emit debugLog(QString("CLEAR: Lượt %1").arg(m_sessionNumber), "blue");
}

const NodeInfo* NodeManager::nodeInfo(const QString &id) const {
    auto it = m_nodes.find(id);
    return (it != m_nodes.end()) ? &it.value() : nullptr;
}

QList<QString> NodeManager::allNodeIDs() const {
    return m_nodes.keys();
}

void NodeManager::onDataReceived(const QString &msg) {
    emit debugLog(QString("RECV: %1").arg(msg), "green");
    // Phân tích: định dạng "NODE<col><row>,<...>"
    if (msg.startsWith("NODE") && msg.length() > 6) {
        QString id = msg.mid(5,2); // lấy "1A"
        NodeInfo *node = findNode(id);
        if (!node) return;

        QStringList parts = msg.split(',');
        if (parts.size() == 3) {
            // Toạ độ: "NODE1A,12.5,-3.2"
            bool ok1, ok2;
            double x = parts[1].trimmed().toDouble(&ok1);
            double y = parts[2].trimmed().toDouble(&ok2);
            if (ok1 && ok2) {
                // Tìm lần bắn trống đầu tiên
                for (int i = 0; i < 3; ++i) {
                    if (!node->shots[i].valid) {
                        node->shots[i].valid = true;
                        node->shots[i].x = x;
                        node->shots[i].y = y;
                        node->shots[i].score = ScoreCalculator::calculateScore(x, y);
                        break;
                    }
                }
                evaluateNode(id);
                emit scoreboardUpdated();
            }
        } else if (parts.size() >= 5 && parts[1].trimmed() == "STATUS") {
            // "NODE1A,STATUS,85,DEACTIVATED,31,SF7-915"
            handleStatusMessage(parts);
        }
    }
}

void NodeManager::handleStatusMessage(const QStringList &parts) {
    if (parts.size() < 6) return;
    QString id = parts[0].mid(5,2);
    NodeInfo *node = findNode(id);
    if (!node) return;
    node->battery = parts[2].trimmed().toInt();
    QString stateStr = parts[3].trimmed().toUpper();
    if (stateStr == "ACTIVATED") node->state = NodeInfo::ACTIVATED;
    else if (stateStr == "DEACTIVATED") node->state = NodeInfo::DEACTIVATED;
    else if (stateStr == "MARKING") node->state = NodeInfo::MARKING;
    else node->state = NodeInfo::WARN;
    node->temperature = parts[4].trimmed().toDouble();
    node->connectionInfo = parts[5].trimmed();
    node->lastStatusTime = QDateTime::currentDateTime();
    emit nodeStatusChanged();
}

void NodeManager::evaluateNode(const QString &nodeId) {
    NodeInfo *node = findNode(nodeId);
    if (!node) return;
    int count = 0;
    double sumX = 0, sumY = 0;
    int sumScore = 0;
    for (int i = 0; i < 3; ++i) {
        if (node->shots[i].valid) {
            sumX += node->shots[i].x;
            sumY += node->shots[i].y;
            sumScore += node->shots[i].score;
            ++count;
        }
    }
    node->totalScore = sumScore;
    if (count > 0) {
        node->avgX = sumX / count;
        node->avgY = sumY / count;
    } else {
        node->avgX = node->avgY = 0;
    }
    //node->classification = ScoreCalculator::classify(node->totalScore);
    node->classification = QString::fromStdString(ScoreCalculator::classify(node->totalScore));
}

NodeInfo* NodeManager::findNode(const QString &id) {
    auto it = m_nodes.find(id);
    return (it != m_nodes.end()) ? &it.value() : nullptr;
}

void NodeManager::onStatusTimeout() {
    QDateTime now = QDateTime::currentDateTime();
    for (auto &node : m_nodes) {
        if (node.lastStatusTime.secsTo(now) > STATUS_TIMEOUT_SEC) {
            if (node.state != NodeInfo::WARN) {
                node.state = NodeInfo::WARN;
                emit nodeStatusChanged();
            }
        }
    }
}
