#include "chatclient.h"

// 생성자 수정
ChatClient::ChatClient() : waitingForLoginResponse(false)
{


}


bool ChatClient::connectServer(int port, QString& ip){
    sock = new QTcpSocket();
    // 연결 시도
    sock->connectToHost(ip, port);
    // 연결 성공 여부 확인
    if (sock->waitForConnected(3000)) { // 3초 동안 대기
        qDebug() << "Connected to server at" << ip << ":" << port;

        // readyRead 시그널을 통합 처리 함수에 연결
        connect(sock, &QTcpSocket::readyRead, this, &ChatClient::onSocketReadyRead);

        return true;
    } else {
        qDebug() << "Connection failed:" << sock->errorString();
        sock->deleteLater(); // 실패한 경우에만 삭제
        return false;
    }
}

bool ChatClient::tryLogin(QString& name, QString& team, QString& position)
{
    if (!sock || sock->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Socket is not connected";
        return false;
    }

    QJsonObject loginData;
    loginData["type"] = "login";
    loginData["name"] = name;
    loginData["team"] = team;
    loginData["position"] = position;

    m_name = name;
    m_team = team;
    m_position = position;

    QJsonDocument doc(loginData);
    QByteArray jsonData = doc.toJson();

    qint64 bytesWritten = sock->write(jsonData);
    if (bytesWritten == -1) {
        qDebug() << "Failed to send login request:" << sock->errorString();
        return false;
    }

    if (!sock->flush()) {
        qDebug() << "Failed to flush data:" << sock->errorString();
        return false;
    }

    waitingForLoginResponse = true;
    return true;
}

// 통합 수신 처리 함수
void ChatClient::onSocketReadyRead()
{
    // 모든 가용 데이터를 읽을 때까지 반복
    while (sock->bytesAvailable() > 0) {
        QByteArray data = sock->readAll();
        qDebug() << "Received data:" << data;

        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data, &parseError);

        if (parseError.error != QJsonParseError::NoError) {
            qDebug() << "JSON parse error:" << parseError.errorString();
            return;
        }

        if (!jsonDoc.isObject()) {
            qDebug() << "Received invalid JSON data";
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        QString msgType = jsonObj["type"].toString();

        if (msgType.isEmpty()) {
            qDebug() << "Received JSON data without type field";
            return;
        }

        if (msgType == "login_response" && waitingForLoginResponse) {
            processLoginResponse(jsonObj);
        }
        else if (msgType == "chat") {
            processRegularMessage(jsonObj);
        }
        else {
            qDebug() << "Unknown message type:" << msgType;
        }
    }
}

void ChatClient::processLoginResponse(const QJsonObject& response)
{

    qDebug()<<"We got login Response!";
    bool isSuccess = (response["status"].toString() == "success");
    QString message = response["message"].toString();
    waitingForLoginResponse = false;

    emit loginResult(isSuccess, message);
    qDebug() << "Login" << (isSuccess ? "successful" : "failed") << ":" << message;
}

// 일반 메시지 처리 함수
void ChatClient::processRegularMessage(const QJsonObject& jsonObj)
{
    QString timestamp = jsonObj["timestamp"].toString();
    QString content = jsonObj["message"].toString();
    QString name = jsonObj["name"].toString();
    QString team = jsonObj["team"].toString();
    QString position = jsonObj["position"].toString();

    // 메시지 포맷: [시간] [팀] 이름(직책): 메시지내용
    QString formattedMessage = QString("[%1] %2(%3/%4): %5")
                                   .arg(timestamp)
                                   .arg(name)
                                   .arg(team)
                                   .arg(position)
                                   .arg(content);

    qDebug() << "Emitting msgReceived signal with message:" << formattedMessage;
    emit msgReceived(formattedMessage);
}


bool ChatClient::sendMessage(QString& message)
{
    // 소켓이 연결되어 있지 않으면 false 반환
    if (!sock || sock->state() != QAbstractSocket::ConnectedState) {
        qDebug() << "Connection Lost";
        return false;
    }

    // 메시지가 비어있지 않은 경우에만 전송
    if (!message.isEmpty()) {
        // JSON 메시지 구성
        QJsonObject messageObj;
        messageObj["type"] = "chat";              // 메시지 타입을 chat으로 설정
        messageObj["message"] = message;          // 실제 메시지 내용

        // 사용자 정보 추가
        messageObj["name"] = m_name;              // 사용자 이름
        messageObj["team"] = m_team;              // 팀 정보
        messageObj["position"] = m_position;      // 직위 정보

        // JSON 문서 생성 및 바이트 배열로 변환
        QJsonDocument doc(messageObj);
        QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

        // 데이터 전송
        qint64 bytesWritten = sock->write(jsonData);

        // 메시지 전송이 실패하면 false 반환
        if (bytesWritten == -1) {
            qDebug() << "Failed to send message:" << sock->errorString();
            return false;
        }

        // 버퍼에 있는 데이터를 즉시 전송
        if (!sock->flush()) {
            qDebug() << "Failed to flush socket:" << sock->errorString();
            return false;
        }

        return true;
    }

    return false;
}
