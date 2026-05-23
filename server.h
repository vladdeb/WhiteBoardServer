#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QJsonObject>
#include <QByteArray>
#include <QVector>
#include <QSet>

class Server : public QObject
{
    Q_OBJECT
private:
    QMap<quint32, QVector<QJsonObject>> boards;
    QMap<quint32, quint32> current;
    QMap<quint32, QSet<QTcpSocket*>> redactors;
    QMap<QTcpSocket*, quint32> ids;
    QTcpServer *server;

    qint32 generateKey();
    void proceedRequest(const QByteArray&, QTcpSocket*);
    QByteArray packJson(const QJsonObject&);
public:
    Server(QObject* parent = nullptr);
    bool start(quint16 port);
private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();
};

#endif // SERVER_H
