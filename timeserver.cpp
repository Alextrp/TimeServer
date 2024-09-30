#include "timeserver.h"
#include <QCoreApplication>
#include <QCommandLineParser>

TimeServer::TimeServer(quint16 time, QObject *parent) : QObject(parent), currentTact(0), lastUpdateTime(0), logLevel(0), timeline(0) {
    timeline = time;

    udpSocket = new QUdpSocket(this);
    timeThread = new QThread(this);



    currentTact = QDateTime::currentSecsSinceEpoch(); // Счётчик тактов начинается с текущего времени
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
        if (logLevel > 0) {
            qDebug() << "Сервер запущен на порте" << port;
        }
        return true;
    } else {
        if (logLevel > 0) {
            qDebug() << "Ошибка подключения к порту. Попробуйте другой порт" << port;
        }
        return false;
    }
}

void TimeServer::startTactUpdate() {
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TimeServer::updateTact);
    timer->start(timeline); // Обновляем в зависимости от шкалы времени
}

void TimeServer::updateTact() {
    qint64 currentTime = QDateTime::currentSecsSinceEpoch();

    // Корректируем количество пропущенных тактов в зависимости от времени
    if (lastUpdateTime > 0) {
        qint64 timeDifference = currentTime - lastUpdateTime;
        currentTact += timeDifference; // Увеличиваем счётчик на пропущенные такты
    }

    lastUpdateTime = currentTime; // Обновляем последнее время обновления такта
    if (logLevel > 0) {
        qDebug() << "Текущий синхронизированный такт:" << currentTact;
    }
}

void TimeServer::processPendingDatagrams() {
    while (udpSocket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = udpSocket->receiveDatagram();
        logReceivedRequest(datagram);

        QJsonDocument jsonDoc = QJsonDocument::fromJson(datagram.data());
        QJsonObject jsonObject = jsonDoc.object();

        if (jsonObject["type"] == "get_tact") {
            QJsonObject responseJson;
            responseJson["tact"] = currentTact;

            QJsonDocument responseDoc(responseJson);
            udpSocket->writeDatagram(responseDoc.toJson(), datagram.senderAddress(), datagram.senderPort());

            logSentResponse(datagram.senderAddress(), datagram.senderPort(), currentTact);
        }
    }
}

void TimeServer::logReceivedRequest(const QNetworkDatagram &datagram) {
    if (logLevel > 1) {
        qDebug() << "Получен запрос от" << datagram.senderAddress().toString()
                 << "на порту" << datagram.senderPort() << "- данные:" << datagram.data();
    }
}

void TimeServer::logSentResponse(const QHostAddress &address, quint16 port, qint64 tact) {
    if (logLevel > 1) {
        qDebug() << "Отправлен синхронизированный такт:" << tact << "на" << address.toString() << "на порту" << port;
    }
}

void TimeServer::setLogLevel(int level) {
    logLevel = level;
}

void TimeServer::setTimeline(int time){
    timeline = time;
}

