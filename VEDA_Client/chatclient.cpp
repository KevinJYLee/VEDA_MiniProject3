#include "chatclient.h"

ChatClient::ChatClient() : sock(new QTcpSocket(this)), waitingForLoginResponse(false)
{
    connect(sock, &QTcpSocket::readyRead, this, &ChatClient::onLoginResponse);
}


bool ChatClient::connectServer(int port, QString ip){
    sock = new QTcpSocket();
    // 연결 시도
    sock->connectToHost(ip, port);
    // 연결 성공 여부 확인
    if (sock->waitForConnected(3000)) { // 3초 동안 대기
        qDebug() << "Connected to server at" << ip << ":" << port;
        return true;
    } else {
        qDebug() << "Connection failed:" << sock->errorString();
        sock->deleteLater(); // 실패한 경우에만 삭제
        return false;
    }
}

bool ChatClient::tryLogin(QString name, QString team, QString position)
{
    if (!sock || sock->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Socket is not connected";
        return false;
    }

    // JSON 객체 생성
    QJsonObject loginData;
    loginData["type"] = "login";
    loginData["name"] = name;
    loginData["team"] = team;
    loginData["position"] = position;

    // JSON 문서로 변환
    QJsonDocument doc(loginData);
    QByteArray jsonData = doc.toJson();

    // 서버로 전송
    qint64 bytesWritten = sock->write(jsonData);

    if (bytesWritten == -1) {
        qDebug() << "Failed to send login request:" << sock->errorString();
        return false;
    }

    if (!sock->flush()) {
        qDebug() << "Failed to flush data:" << sock->errorString();
        return false;
    }

    waitingForLoginResponse = true;  // 로그인 응답 대기 상태로 설정
    return true;
}

void ChatClient::onLoginResponse()
{
    if (!waitingForLoginResponse) {
        return;  // 로그인 응답을 기다리고 있지 않다면 무시
    }

    QByteArray data = sock->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

    if (jsonDoc.isObject()) {
        QJsonObject response = jsonDoc.object();

        if (response["type"].toString() == "login_response") {
            bool success = (response["status"].toString() == "success");
            QString message = response["message"].toString();

            waitingForLoginResponse = false;  // 응답 처리 완료
            emit loginResult(success, message);  // 결과 시그널 발생

            qDebug() << "Login" << (success ? "successful" : "failed") << ":" << message;
        }
    }
}
