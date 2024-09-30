#ifndef TIMESERVER_H
#define TIMESERVER_H

#include <QObject>
#include <QUdpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDateTime>
#include <QNetworkDatagram>
#include <QDebug>
#include <QTimer>
#include <QThread>

class TimeServer : public QObject {
    Q_OBJECT

public:
    explicit TimeServer(quint16 timeline, QObject *parent = nullptr);
    ~TimeServer();
    bool startServer(quint16 port);
    void setLogLevel(int level);
    void setTimeline(int timeline);

private slots:
    void processPendingDatagrams();
    void startTactUpdate();
    void updateTact();

private:
    QUdpSocket *udpSocket;
    QThread *timeThread;
    qint64 currentTact;        // Счётчик тактов
    qint64 lastUpdateTime;     // Время последнего обновления тактов
    int logLevel;              // Уровень логирования
    int timeline;              // Шкала времени

    void logReceivedRequest(const QNetworkDatagram &datagram);
    void logSentResponse(const QHostAddress &address, quint16 port, qint64 tact);
};

#endif // TIMESERVER_H
