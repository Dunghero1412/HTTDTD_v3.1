// UdpGateway.cpp
#include "UdpGateway.hpp"

// khởi tạo socket UDP và thiết lập kết nối
UdpGateway::UdpGateway(QObject *parent) : QObject(parent), socket(new QUdpSocket(this)) {}

// bắt đầu lắng nghe trên cổng địa phương và thiết lập địa chỉ từ xa
bool UdpGateway::start(quint16 localPort, const QHostAddress &remoteHost, quint16 remotePort) {
    remoteAddr = remoteHost;
    this->remotePort = remotePort;
    if (!socket->bind(localPort, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint)) {
        return false;
    }
    connect(socket, &QUdpSocket::readyRead, this, &UdpGateway::onReadyRead);
    return true;
}

// gửi lệnh đến thiết bị qua UDP
void UdpGateway::sendCommand(const QString &cmd) {
    QByteArray datagram = cmd.toUtf8();
    socket->writeDatagram(datagram, remoteAddr, remotePort);
}

// xử lý dữ liệu nhận được từ thiết bị
void UdpGateway::onReadyRead() {
    while (socket->hasPendingDatagrams()) {
        QByteArray buffer;
        buffer.resize(socket->pendingDatagramSize());
        socket->readDatagram(buffer.data(), buffer.size());
        emit dataReceived(QString::fromUtf8(buffer));
    }
}