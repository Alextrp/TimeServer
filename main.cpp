#include <QCoreApplication>
#include "timeserver.h"

int main(int argc, char *argv[]) {
    QCoreApplication a(argc, argv);

    TimeServer server;
    return a.exec();
}
