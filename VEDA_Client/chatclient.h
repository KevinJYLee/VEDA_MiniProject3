#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>


class ChatClient : public QObject
{
    Q_OBJECT
public:
    ChatClient();
    bool connectServer(int, QString);
    bool tryLogin(QString, QString, QString);

private slots:
    void onLoginResponse();  // 서버로부터의 응답을 처리하는 슬롯

signals:
    void loginResult(bool success, QString message);  // 로그인 결과를 알리는 시그널

private:
    QTcpSocket* sock;
    bool waitingForLoginResponse;  // 로그인 응답 대기 상태를 추적
};

#endif // CHATCLIENT_H
