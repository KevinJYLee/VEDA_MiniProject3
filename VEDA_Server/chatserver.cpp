#include "chatserver.h"
#include <QDateTime>

ChatServer::ChatServer(QObject *parent) : QObject(parent), m_server(new QTcpServer(this))
{
    connect(m_server, &QTcpServer::newConnection, this, &ChatServer::onNewConnection);
}

// 서버 시작
bool ChatServer::start(int port)
{
    if (!m_server->listen(QHostAddress::Any, port)) {
        return false;
    }
    return true;
}

// 서버 중지
void ChatServer::stop()
{
    m_server->close();
}

// 새 클라이언트 연결 처리
void ChatServer::onNewConnection()
{
    QTcpSocket *clientSocket = m_server->nextPendingConnection();
    connect(clientSocket, &QTcpSocket::readyRead, this, &ChatServer::onReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &ChatServer::onClientDisconnected);

    m_clients.insert(clientSocket, clientSocket->peerAddress().toString() + ":" + QString::number(clientSocket->peerPort()));

    QString connectionMessage = QString("New client connected: %1").arg(m_clients[clientSocket]);
    emit newConnection(connectionMessage);
}

void ChatServer::onReadyRead()
{

    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket)
        return;

    QByteArray data = clientSocket->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);




    if (!jsonDoc.isNull() && jsonDoc.isObject()) {
        QJsonObject jsonObj = jsonDoc.object();
        QString msgType = jsonObj["type"].toString();



        if (msgType == "login") {
            qDebug()<<"Message Received and Type is login"<<msgType;
            QString name = jsonObj["name"].toString();
            QString team = jsonObj["team"].toString();
            QString position = jsonObj["position"].toString();
            emit loginRequested(clientSocket, name, team, position);
            return;
        }


        qDebug()<<"Message Received and Type is "<<msgType;
        // 채팅 메시지 처리
        QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

        // 브로드캐스팅용 JSON 메시지 생성
        QJsonObject broadcastObj;
        broadcastObj["type"] = "chat";
        broadcastObj["timestamp"] = timestamp;
        broadcastObj["message"] = jsonObj["message"].toString();

        QJsonDocument broadcastDoc(broadcastObj);
        QByteArray broadcastData = broadcastDoc.toJson();

        // 모든 클라이언트에게 메시지 브로드캐스트
        for (QTcpSocket *socket : m_clients.keys()) {
            socket->write(broadcastData);
            socket->flush();  // 즉시 전송 보장
        }

        emit newMessage(QString("%1 - %2").arg(timestamp).arg(QString::fromUtf8(data)));
    }
}

// 클라이언트 연결 종료 처리
void ChatServer::onClientDisconnected()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket)
        return;

    QString disconnectionMessage = QString("Client disconnected: %1").arg(m_clients[clientSocket]);
    m_clients.remove(clientSocket);
    clientSocket->deleteLater();

    emit clientDisconnected(disconnectionMessage);
}

QString ChatServer::getClientInfo(QObject* sender)
{
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender);
    if (clientSocket && m_clients.contains(clientSocket)) {
        return m_clients[clientSocket];
    }
    return QString();
}

QString ChatServer::getIp(){
    QHostAddress serverAddress = m_server->serverAddress();

    return serverAddress.toString();
}

quint16 ChatServer::getPort(){
    auto serverPort = m_server->serverPort();

    return serverPort;
}
