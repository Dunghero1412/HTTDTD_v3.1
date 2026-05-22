// UdpGateway.cpp
#include "UdpGateway.hpp"

UdpGateway::UdpGateway(QObject *parent) : QObject(parent), socket(new QUdpSocket(this)) {}

bool UdpGateway::start(quint16 localPort, const QHostAddress &remoteHost, quint16 remotePort) {
    remoteAddr = remoteHost;
    this->remotePort = remotePort;
    if (!socket->bind(localPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        return false;
    }
    connect(socket, &QUdpSocket::readyRead, this, &UdpGateway::onReadyRead);
    return true;
}

void UdpGateway::sendCommand(const QString &cmd) {
    QByteArray datagram = cmd.toUtf8();
    socket->writeDatagram(datagram, remoteAddr, remotePort);
}

void UdpGateway::onReadyRead() {
    while (socket->hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(socket->pendingDatagramSize());
        socket->readDatagram(buffer.data(), buffer.size());
        emit dataReceived(QString::fromUtf8(buffer));
    }
}