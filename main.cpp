#include "server.h"

#include <QCoreApplication>
#include <QString>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if(argc != 2) {
        qDebug() << "Usage: " << argv[0] << " <port>";
        return 1;
    }

    Server server;
    QString port_str(argv[1]);
    int port = port_str.toInt();
    if(!server.start(port)) {
        return 1;
    }
    // Set up code that uses the Qt event loop here.
    // Call a.quit() or a.exit() to quit the application.
    // A not very useful example would be including
    // #include <QTimer>
    // near the top of the file and calling
    // QTimer::singleShot(5000, &a, &QCoreApplication::quit);
    // which quits the application after 5 seconds.

    // If you do not need a running Qt event loop, remove the call
    // to a.exec() or use the Non-Qt Plain C++ Application template.

    return a.exec();
}
