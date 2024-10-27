#ifndef CHATCLIENT_H
#define CHATCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFileDialog>


class ChatClient : public QObject
{
    Q_OBJECT
public:
    ChatClient();
    bool connectServer(int, QString&);
    bool tryLogin(QString&, QString&, QString&);
    bool sendMessage(QString& );
    bool sendFile(const QByteArray &fileData);

private slots:
    void onSocketReadyRead(); // 로그인 결과 및 메시지 통합 수신 처리

signals:
    void loginResult(bool success, QString message);  // 로그인 결과를 알리는 시그널
    void msgReceived(QString& message);
    void fileReceived(const QString& fileName);

private:
    QTcpSocket* sock;
    bool waitingForLoginResponse;  // 로그인 응답 대기 상태를 추적

<<<<<<< Updated upstream
=======
    QString m_name;
    QString m_team;
    QString m_position;

    void processFileMessage(const QJsonObject& jsonObj);
<<<<<<< Updated upstream
>>>>>>> Stashed changes
=======
>>>>>>> Stashed changes
    void processLoginResponse(const QJsonObject& jsonObj);  // 로그인 응답 처리
    void processRegularMessage(const QJsonObject& jsonObj); // 일반 메시지 처리
};

#endif // CHATCLIENT_H
