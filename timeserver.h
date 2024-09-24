#ifndef TIMESERVER_H
#define TIMESERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QNetworkDatagram>
#include <QDebug>
#include <QThread>
#include <QTimer>

class TimeServer : public QObject {
    Q_OBJECT

public:
    explicit TimeServer(QObject *parent = nullptr);
    ~TimeServer();
    bool startServer(quint16 port);

private slots:
    void processPendingDatagrams();
    void startTactUpdate();   // Метод для запуска обновления тактов
    void updateTact();        // Метод для обновления такта

private:
    QUdpSocket *udpSocket;
    QThread *timeThread;      // Нить для обновления тактов
    qint64 currentTact;  // Текущее значение такта
    qint64 lastUpdateTime; // Время последнего обновления

    void logReceivedRequest(const QNetworkDatagram &datagram);
    void logSentResponse(const QHostAddress &address, quint16 port, qint64 tact);
};

#endif // TIMESERVER_H
