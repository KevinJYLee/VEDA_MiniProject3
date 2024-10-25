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

// 클라이언트로부터 데이터 수신 처리
void ChatServer::onReadyRead()
{
    
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket*>(sender());
    if (!clientSocket)
        return;

    QByteArray data = clientSocket->readAll();

    // JSON 파싱을 통한 메시지 타입 확인
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
    if (!jsonDoc.isNull() && jsonDoc.isObject()) {
        QJsonObject jsonObj = jsonDoc.object();
        QString msgType = jsonObj["type"].toString();

        // 로그인 요청 처리
        if (msgType == "login") {
            QString name = jsonObj["name"].toString();
            QString team = jsonObj["team"].toString();
            QString position = jsonObj["position"].toString();

            // 로그인 요청 시그널 발생
            emit loginRequested(clientSocket, name, team, position);
            return;  // 로그인 요청은 여기서 처리 종료
        }
    }

    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString message = QString("%1 - %2").arg(timestamp).arg(QString::fromUtf8(data));

    emit newMessage(message);

    // 모든 클라이언트에게 메시지 브로드캐스트
    for (QTcpSocket *socket : m_clients.keys()) {
        socket->write(message.toUtf8());
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
