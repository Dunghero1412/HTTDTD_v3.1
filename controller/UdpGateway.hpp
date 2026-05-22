// UdpGateway.hpp
#pragma once
#include <QObject>
#include <QUdpSocket>
#include <QHostAddress>

class UdpGateway : public QObject {
    Q_OBJECT
public:
    explicit UdpGateway(QObject *parent = nullptr);
    bool start(quint16 localPort, const QHostAddress &remoteHost, quint16 remotePort);
    void sendCommand(const QString &cmd);

signals:
    void dataReceived(const QString &message); // chuỗi nhận được từ node

private slots:
    void onReadyRead();

private:
    QUdpSocket *socket;
    QHostAddress remoteAddr;
    quint16 remotePort;
};