#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logindialogue.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_chatclient(new ChatClient())
{
    ui->setupUi(this);

    connect(ui->connectButton, &QPushButton::clicked, this, &MainWindow::onConnectServerClicked);
    connect(m_chatclient, &ChatClient::loginResult, this, &MainWindow::handleLoginResult);
    connect(ui->msgSendButton,&QPushButton::clicked,this,&MainWindow::onMsgSendClicked);
    connect(m_chatclient,&ChatClient::msgReceived,this,&MainWindow::onMsgReceived);
    connect(ui->fileSendButton, &QPushButton::clicked, this, &MainWindow::onFileSendClicked);

    m_scrollbar = ui->textBrowser->verticalScrollBar();
}

void MainWindow::onConnectServerClicked(){
    Port = ui->portEdit->text().toInt();
    IP = ui->ipEdit->text();

    if (m_chatclient->connectServer(Port, IP)) {
        LoginDialog loginDialog(this);
        if (loginDialog.exec() == QDialog::Accepted) {
            // 로그인 시도
            QString name = loginDialog.getName();
            QString team = loginDialog.getTeam();
            QString position = loginDialog.getPosition();

            // ChatClient에 로그인 함수가 있다고 가정
            bool loginSuccess = m_chatclient->tryLogin(name, team, position);

            if (loginSuccess) {
                QMessageBox::information(this, "Success", "Login successful!");
                // 추가적인 로그인 성공 처리
            } else {
                QMessageBox::warning(this, "Error", "Login failed!");
                // 실패 처리
            }
        }
    } else {
        QMessageBox::critical(this, "Error", "Could not connect to server!");
    }



}

void MainWindow::handleLoginResult(bool success, QString message)
{
    if (success) {
        QMessageBox::information(this, "Login Successful", "Successfully logged in!");
        // 로그인 성공 후 필요한 UI 업데이트
    } else {
        QMessageBox::warning(this, "Login Failed", "Login failed: " + message);
        // 로그인 실패 시 UI 처리
    }
}

void MainWindow::onMsgSendClicked()
{


    QString message = ui->msgLineEdit->text().trimmed();  // 앞뒤 공백 제거

    qDebug()<<"Entered MsgSendClicked Slot. Message is : " + message;

    // 빈 메시지 체크
    if (message.isEmpty()) {
        ui->textBrowser->append(tr("메시지를 입력해주세요."));
        ui->msgLineEdit->setFocus();
        return;
    }

    // 메시지 길이 제한 체크 (예: 1000자)
    static const int MAX_MESSAGE_LENGTH = 1000;
    if (message.length() > MAX_MESSAGE_LENGTH) {
        ui->textBrowser->append(tr("메시지가 너무 깁니다. %1자 이내로 입력해주세요.").arg(MAX_MESSAGE_LENGTH));
        return;
    }

    try {
        // 메시지 전송 시도
        if (!m_chatclient) {
            throw std::runtime_error("채팅 클라이언트가 초기화되지 않았습니다.");
        }

        if (!m_chatclient->sendMessage(message)) {
            throw std::runtime_error("메시지 전송에 실패했습니다.");
        }

        // 전송 성공 시 입력창 초기화
        ui->msgLineEdit->clear();
        ui->msgLineEdit->setFocus();


    } catch (const std::exception& e) {
        // 에러 메시지를 상태바에 표시
        ui->textBrowser->append(tr("오류: %1").arg(e.what()));


    }
}

void MainWindow::onMsgReceived(QString& message)
{

    // MainWindow::onMsgReceived() 함수의 시작 부분에
    qDebug() << "MainWindow received message:" << message;

    // 빈 메시지 체크
    if (message.isEmpty()) {
        qDebug() << "Empty message received";
        return;
    }

    try {
        // UI 업데이트는 메인 스레드에서 실행되어야 함
        if (QThread::currentThread() != QApplication::instance()->thread()) {
            QMetaObject::invokeMethod(this, "onMsgReceived",
                                      Qt::QueuedConnection,
                                      Q_ARG(QString&, message));
            return;
        }

        // textBrowser가 유효한지 확인
        if (!ui || !ui->textBrowser) {
            throw std::runtime_error("UI components are not properly initialized");
        }

        // 메시지 표시 전 최대 라인 수 체크 및 관리
        static const int MAX_DISPLAYED_LINES = 1000;  // 적절한 값으로 조정 가능
        if (ui->textBrowser->document()->lineCount() > MAX_DISPLAYED_LINES) {
            // 오래된 메시지 제거
            QTextCursor cursor(ui->textBrowser->document());
            cursor.movePosition(QTextCursor::Start);
            cursor.movePosition(QTextCursor::Down, QTextCursor::KeepAnchor,
                                ui->textBrowser->document()->lineCount() - MAX_DISPLAYED_LINES);
            cursor.removeSelectedText();
        }

        // 메시지 표시
        ui->textBrowser->append(message);

        // 자동 스크롤
        m_scrollbar->setValue(m_scrollbar->maximum());

    } catch (const std::exception& e) {
        qCritical() << "Error displaying message:" << e.what();
        ui->textBrowser->append(tr("메시지 표시 중 오류가 발생했습니다: %1").arg(e.what()));
    }
}

bool MainWindow::onFileSendClicked()
{
    QString filePath = QFileDialog::getOpenFileName(this, "Select File to Send", "", "All Files (*)");
    if (filePath.isEmpty()) {
        return false;  // 파일 선택이 취소되면 false 반환
    }

    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", "Cannot open file for reading");
        return false;  // 파일을 열 수 없으면 false 반환
    }

    QByteArray fileData = file.readAll();
    QFileInfo fileInfo(filePath);
    QString fileName = fileInfo.fileName();

    // 파일 데이터를 JSON 형식으로 변환
    QJsonObject fileObj;
    fileObj["type"] = "file";
    fileObj["name"] = fileName;
    fileObj["data"] = QString(fileData.toBase64());

    QJsonDocument doc(fileObj);
    QByteArray jsonData = doc.toJson();

    // 파일 전송
    if (!m_chatclient->sendFile(jsonData)) {
        QMessageBox::warning(this, "Error", "Failed to send file");
        return false;  // 파일 전송 실패 시 false 반환
    }

    return true;  // 파일 전송 성공 시 true 반환
}

MainWindow::~MainWindow()
{
    delete ui;
}

