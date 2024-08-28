#include <QCoreApplication>
#include "timeserver.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    QStringList arguments = a.arguments();

    TimeServer server;

    quint16 port = -1;

    if (arguments.size() > 1) {
        bool ok;
        quint16 parsedPort = arguments[1].toUShort(&ok);
        if (ok) {
            port = parsedPort;
        } else {
            qDebug() << "Введите желаемый номер порта. Пример ./TimeServer 45454";
        }
    }

    server.startServer(port);

    return a.exec();
}
