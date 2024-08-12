#include "timeserver.h"

TimeServer::TimeServer(QObject *parent) : QObject(parent) {
    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::Any, 45454);

    connect(udpSocket, &QUdpSocket::readyRead, this, &TimeServer::processPendingDatagrams);
    qDebug() << "TimeServer started on port 45454.";
}

void TimeServer::processPendingDatagrams() {
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        logReceivedRequest(datagram);

        QJsonDocument jsonDoc = QJsonDocument::fromJson(datagram.data());
        QJsonObject jsonObject = jsonDoc.object();

        if (jsonObject["type"] == "get_tact") {
            qint64 currentTact = QDateTime::currentSecsSinceEpoch(); // Используем текущее время в миллисекундах

            QJsonObject responseJson;
            responseJson["tact"] = currentTact;

            QJsonDocument responseDoc(responseJson);
            udpSocket->writeDatagram(responseDoc.toJson(), datagram.senderAddress(), datagram.senderPort());

            logSentResponse(datagram.senderAddress(), datagram.senderPort(), currentTact);
        }
    }
}

void TimeServer::logReceivedRequest(const QNetworkDatagram &datagram) {
    qDebug() << "Received request from" << datagram.senderAddress().toString()
             << "on port" << datagram.senderPort() << "- data:" << datagram.data();
}

void TimeServer::logSentResponse(const QHostAddress &address, quint16 port, qint64 tact) {
    qDebug() << "Sent synchronized tact:" << tact << "to" << address.toString() << "on port" << port;
}
