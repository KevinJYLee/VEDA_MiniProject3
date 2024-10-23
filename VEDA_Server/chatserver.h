#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>

class ChatServer : public QObject
{
    Q_OBJECT
public:
    explicit ChatServer(QObject *parent = nullptr);
    bool start(int port);
    void stop();
    QString getClientInfo(QObject* sender);

signals:
    void newConnection(const QString &message);
    void newMessage(const QString &message);
    void clientDisconnected(const QString &message);

private slots:
    void onNewConnection();
    void onReadyRead();
    void onClientDisconnected();

private:
    QTcpServer *m_server;
    QMap<QTcpSocket*, QString> m_clients;
};

#endif // CHATSERVER_H
