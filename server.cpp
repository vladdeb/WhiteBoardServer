#include "server.h"
#include <QDebug>
#include <QRandomGenerator>
#include <QDataStream>
#include <QJsonArray>

Server::Server(QObject *parent) : QObject(parent) {
    server = new QTcpServer(this);

    connect(server, &QTcpServer::newConnection, this, &Server::onNewConnection);
}

bool Server::start(quint16 port) {
    bool r = server->listen(QHostAddress::Any, port);
    if(r) {
        qDebug() << "Server running on port " << port;
    }
    else {
        qDebug() << "Server run failed: " << server->errorString();
    }
    return r;
}

qint32 Server::generateKey() {
    if (boards.size() >= std::numeric_limits<int>::max()) {
        qWarning("QMap переполнен, невозможно сгенерировать уникальный ключ!");
        return 1e9+1;
    }

    int randomKey;

    do {
        randomKey = QRandomGenerator::global()->bounded(0, (int)1e9);

    } while (boards.contains(randomKey));

    return randomKey;
}

QByteArray Server::packJson(const QJsonObject &obj) {
    QJsonDocument doc(obj);
    QByteArray body = doc.toJson(QJsonDocument::Compact);

    QByteArray packet;
    QDataStream out(&packet, QIODevice::WriteOnly);
    out << static_cast<quint32>(body.size());
    packet.append(body);

    return packet;
}

std::string JsonToStd(QJsonObject obj) {
    return QJsonDocument(obj).toJson().toStdString();
}

void Server::proceedRequest(const QByteArray &data, QTcpSocket *clientSocket) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    QJsonObject json = doc.object();
    QString type = json["type"].toString();
    qDebug().noquote() << doc.toJson().toStdString();
    if(type == "host") {
        quint32 id = generateKey();
        if(id == 1e9 + 1) {
            QJsonObject toSend;
            toSend["type"] = "host_failed";
            qDebug() << "Host failed";
            clientSocket->write(packJson(toSend));
            return;
        }
        QJsonObject toSend;
        toSend["type"] = "host_success";
        toSend["id"] = static_cast<int>(id);
        auto newData = packJson(toSend);
        qDebug() << "Host success";
        clientSocket->write(newData);
        redactors[id] = {clientSocket};
        current[id] = json["current"].toInt();
        boards[id] = {};
        for(auto figure: json["figures"].toArray()) {
            //qDebug() << "Received:" << JsonToStd(figure.toObject());
            boards[id].push_back(figure.toObject());
        }
        ids[clientSocket] = id;
        return;
    }
    quint32 id = json["id"].toInt(0);
    for(auto redactor: redactors[id]) {
        qDebug() << "Redacto: " << redactor;
    }
    if(type == "connect") {
        if(boards.find(id) == boards.end()) {
            QJsonObject toSend;
            qDebug() << "Connection failed";
            toSend["type"] = "connection_failed";
            clientSocket->write(packJson(toSend));
            return;
        }
        QJsonArray arr;
        for(auto figure: boards[id]) {
            arr.append(figure);
        }
        QJsonObject toSend;
        toSend["type"] = "connection_success";
        toSend["figures"] = arr;
        toSend["id"] = static_cast<int>(id);
        toSend["current"] = static_cast<int>(current[id]);
        qDebug() << "Connect success";
        clientSocket->write(packJson(toSend));
        redactors[id].insert(clientSocket);
        ids[clientSocket] = id;
        return;
    }
    if(!id || boards.find(id) == boards.end()) {
        return;
    }
    if(type == "draw") {
        boards[id].resize(current[id]);
        boards[id].push_back(json["figure"].toObject());
        current[id] = boards[id].size();
    }
    if(type == "undo") {
        if(current[id] > 0) current[id]--;
    }
    if(type == "redo") {
        if(current[id] < boards[id].size()) current[id]++;
    }
    if(type == "clear") {
        boards[id].clear();
        current[id] = 0;
    }
    for(auto redactor: redactors[id]) {
        if (redactor != clientSocket) {
            qDebug() << "sended";
            redactor->write(packJson(json));
        }
    }
}

void Server::onNewConnection() {
    QTcpSocket *clientSocket = server->nextPendingConnection();

    clientSocket->setProperty("buffer", {});
    clientSocket->setProperty("expected", -1);

    connect(clientSocket, &QTcpSocket::readyRead, this, &Server::onReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &Server::onClientDisconnected);
    qDebug() << "New connection";
}

void Server::onReadyRead() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());

    QByteArray buffer = clientSocket->property("buffer").toByteArray();
    qint32 expected = clientSocket->property("expected").toInt();

    buffer.append(clientSocket->readAll());
    //qDebug() << "Ready read: " << expected;
    while(true) {
        if(expected == -1) {
            if(buffer.size() < sizeof(qint32)) {
                break;
            }
            QDataStream ds(buffer);
            ds >> expected;
            buffer.remove(0, sizeof(qint32));
        }
        if(buffer.size() < expected) {
            break;
        }

        //qDebug() << "Ready buffer: " << expected;
        QByteArray data = buffer.left(expected);
        proceedRequest(data, clientSocket);
        buffer.remove(0, expected);
        expected = -1;
    }
    clientSocket->setProperty("buffer", buffer);
    clientSocket->setProperty("expected", expected);
}

void Server::onClientDisconnected() {
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket) return;

    if (ids.contains(clientSocket)) {
        quint32 id = ids[clientSocket];
        ids.remove(clientSocket);

        if (redactors.contains(id)) {
            redactors[id].remove(clientSocket);
            if (redactors[id].isEmpty()) {
                redactors.remove(id);
                boards.remove(id);
                current.remove(id);
            }
        }
    }
    clientSocket->deleteLater();
}
