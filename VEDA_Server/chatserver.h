#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>

class ChatServer : public QObject
{
    Q_OBJECT
public:
    explicit ChatServer(QObject *parent = nullptr);
    bool start(int port);
    void stop();
    QString getClientInfo(QObject* sender);
    QString getIp();
    quint16 getPort();

signals:
    void newConnection(const QString &message);
    void newMessage(const QString &message);
    void clientDisconnected(const QString &message);
    void loginRequested(QTcpSocket* client, QString& name, QString& team, QString& position);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    QTcpServer *m_server;
    void broadcastToClients(const QByteArray& data);
    QHash<QTcpSocket*, QString> m_clients;
};

#endif // CHATSERVER_H
