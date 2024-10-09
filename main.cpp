#include <QCoreApplication>
#include <QCommandLineParser>
#include <QCommandLineOption>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDebug>
#include <QStandardPaths>
#include "timeserver.h"

QJsonObject loadConfig(const QString &filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Не удалось открыть файл конфигурации" << filename;
        return QJsonObject();
    }

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "Ошибка разбора JSON файла:" << parseError.errorString();
        return QJsonObject();
    }

    return jsonDoc.object();
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("TimeServer");
    QCoreApplication::setApplicationVersion("1.0");

    QString configPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/projector/timeServer.json";

    QJsonObject config = loadConfig(configPath);

    QCommandLineParser parser;
    parser.setApplicationDescription("TimeServer - сервер синхронизации тактов");
    parser.addHelpOption();  // Добавляем опцию -h/--help для отображения справки
    parser.addVersionOption();  // Добавляем опцию --version для отображения версии

    QString defaultPort = config.contains("port") ? QString::number(config["port"].toInt()) : "1234";
    QString defaultLogLevel = config.contains("logLevel") ? QString::number(config["logLevel"].toInt()) : "0";
    QString defaultTimeline = config.contains("timeline") ? QString::number(config["timeline"].toInt()) : "1000";

    QCommandLineOption portOption(QStringList() << "p" << "port", "Указать порт для сервера.", "port", defaultPort);
    parser.addOption(portOption);

    QCommandLineOption debugOption(QStringList() << "D" << "debug", "Включить отладочную печать.");
    parser.addOption(debugOption);

    QCommandLineOption logLevelOption(QStringList() << "L" << "log-level", "Уровень логирования.", "level", defaultLogLevel);
    parser.addOption(logLevelOption);

    QCommandLineOption timelineOption(QStringList() << "T" << "timeline", "Шкала времени.", "timeline", defaultTimeline);
    parser.addOption(timelineOption);

    // Разбор аргументов командной строки
    parser.process(app);

    // Проверяем, была ли запрошена справка или версия
    if (parser.isSet("help")) {
        parser.showHelp();  // Показать справку и завершить программу
        return 0;
    }

    if (parser.isSet("version")) {
        parser.showVersion();  // Показать версию и завершить программу
        return 0;
    }

    // Получаем порт
    bool ok;
    quint16 port = parser.value(portOption).toUShort(&ok);
    if (!ok || port == 0) {
        qDebug() << "Ошибка: некорректный номер порта.";
        return 1;
    }

    quint16 timeline = parser.value(timelineOption).toUInt(&ok);
    if (!ok || (timeline <= 1 && timeline >= 1000)) {
        qDebug() << "Ошибка: некорректный таймлайн (Введите в пределе от 1 до 1000).";
        return 1;
    }

    // Создаем сервер
    TimeServer server(timeline);

    if (parser.isSet(debugOption)) {
        server.setLogLevel(1);  // Включаем отладочную печать
    } else {
        server.setLogLevel(parser.value(logLevelOption).toInt());
    }

    // Запускаем сервер на указанном порту
    if (!server.startServer(port)) {
        return 1;
    }

    return app.exec();
}
