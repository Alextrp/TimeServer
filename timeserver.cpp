#include "timeserver.h"
#include <QTimer>

TimeServer::TimeServer(QObject *parent) : QObject(parent), currentTact(0), lastUpdateTime(0) {
    udpSocket = new QUdpSocket(this);
    timeThread = new QThread(this);

    currentTact = QDateTime::currentSecsSinceEpoch(); // Счётчик тактов начинается с 0
    lastUpdateTime = QDateTime::currentSecsSinceEpoch(); // Фиксируем текущее время

    connect(timeThread, &QThread::started, this, &TimeServer::startTactUpdate);
    timeThread->start(); // Запуск отдельной нити для синхронизации тактов

    timeThread->setPriority(QThread::HighPriority);
}

TimeServer::~TimeServer() {
    timeThread->quit();
    timeThread->wait();
}

bool TimeServer::startServer(quint16 port) {
    if (udpSocket->bind(QHostAddress::Any, port)) {
        connect(udpSocket, &QUdpSocket::readyRead, this, &TimeServer::processPendingDatagrams);
        qDebug() << "Сервер запущен на порте" << port;
        return true;
    } else {
        qDebug() << "Ошибка подключения к порту. Попробуйте другой порт" << port;
        return false;
    }
}

void TimeServer::startTactUpdate() {
    // Этот таймер будет обновлять такты каждую секунду
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TimeServer::updateTact);
    timer->start(1000); // Обновляем каждую секунду
}

void TimeServer::updateTact() {
    qint64 currentTime = QDateTime::currentSecsSinceEpoch();

    // Корректируем количество пропущенных тактов в зависимости от времени
    if (lastUpdateTime > 0) {
        qint64 timeDifference = currentTime - lastUpdateTime;
        currentTact += timeDifference; // Увеличиваем счётчик на пропущенные такты
    }

    lastUpdateTime = currentTime; // Обновляем последнее время обновления такта
    qDebug() << "Текущий синхронизированный такт:" << currentTact;
}

void TimeServer::processPendingDatagrams() {
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        logReceivedRequest(datagram);

        QJsonDocument jsonDoc = QJsonDocument::fromJson(datagram.data());
        QJsonObject jsonObject = jsonDoc.object();

        if (jsonObject["type"] == "get_tact") {
            // Отправляем текущий такт, поддерживаемый в отдельной нити
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
