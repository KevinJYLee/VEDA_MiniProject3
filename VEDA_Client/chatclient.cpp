#include "chatclient.h"

// 생성자 수정
ChatClient::ChatClient() : sock(new QTcpSocket(this)), waitingForLoginResponse(false)
{
    // readyRead 시그널을 새로운 통합 처리 함수에 연결
    connect(sock, &QTcpSocket::readyRead, this, &ChatClient::onSocketReadyRead);

}


bool ChatClient::connectServer(int port, QString& ip){
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
    QByteArray data = sock->readAll();
    qDebug() << "Received data:" << data;

    QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
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
        else if (msgType == "file") {
            processFileMessage(jsonObj);
        }
        else {
            qDebug() << "Unknown message type:" << msgType;
        }
    }
}

void ChatClient::processFileMessage(const QJsonObject& jsonObj)
{
    QString fileName = jsonObj["name"].toString();
    QByteArray fileData = QByteArray::fromBase64(jsonObj["data"].toString().toLatin1());

    // 파일 저장 대화상자 표시
    QString savePath = QFileDialog::getSaveFileName(nullptr, "Save File", fileName, "All Files (*)");
    if (!savePath.isEmpty()) {
        QFile file(savePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(fileData);
            file.close();
            emit fileReceived(fileName);
            qDebug() << "File received and saved:" << fileName;
        } else {
            qDebug() << "Failed to save received file:" << fileName;
        }
    } else {
        qDebug() << "File save cancelled by user:" << fileName;
    }
}

void ChatClient::processLoginResponse(const QJsonObject& response)
{
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
    QString formattedMessage = QString("%1 - %2").arg(timestamp).arg(content);

    qDebug() << "Emitting msgReceived signal with message:" << formattedMessage;
    emit msgReceived(formattedMessage);
}


bool ChatClient::sendMessage(QString& message)
{


    // 소켓이 연결되어 있지 않으면 false 반환
    if (!sock || sock->state() != QAbstractSocket::ConnectedState) {
        return false;
    }


    // 일반 메시지인 경우 직접 전송
    if (!message.isEmpty()) {


        qint64 bytesWritten = sock->write(message.toUtf8());



        // 메시지 전송이 실패하면 false 반환
        if (bytesWritten == -1) {
            return false;
        }

        // 버퍼에 있는 데이터를 즉시 전송
        if (!sock->flush()) {
            return false;
        }


    }

    return true;
}

bool ChatClient::sendFile(const QByteArray &fileData)
{
    if (sock->state() == QAbstractSocket::ConnectedState) {
        sock->write(fileData);
        return sock->waitForBytesWritten();
    }
    return false;
}
