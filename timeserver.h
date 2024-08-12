#ifndef TIMESERVER_H
#define TIMESERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QNetworkDatagram>
#include <QDebug>

class TimeServer : public QObject {
    Q_OBJECT

public:
    explicit TimeServer(QObject *parent = nullptr);

private slots:
    void processPendingDatagrams();

private:
    QUdpSocket *udpSocket;
    void logReceivedRequest(const QNetworkDatagram &datagram);
    void logSentResponse(const QHostAddress &address, quint16 port, qint64 tact);
};

#endif // TIMESERVER_H
