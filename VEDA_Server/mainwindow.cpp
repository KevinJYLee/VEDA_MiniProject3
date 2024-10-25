#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dbmanagerwidget.h"
#include <QTcpServer>
#include <QNetworkInterface>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_chatServer(new ChatServer(this))
    , m_chatLogger(new ChatLogger())
    , m_dbManager(new DBManager(this))
{
    ui->setupUi(this);
    updatePortNumber();
    m_chatLogger->startLogging();

    connect(m_chatServer, &ChatServer::newConnection, this, &MainWindow::onNewConnection);
    connect(m_chatServer, &ChatServer::newMessage, this, &MainWindow::onNewMessage);
    connect(m_chatServer, &ChatServer::clientDisconnected, this, &MainWindow::onClientDisconnected);
    connect(m_chatServer,&ChatServer::loginRequested,this,&MainWindow::onClientLogin);

    connect(ui->startServerButton, &QPushButton::clicked, this, &MainWindow::onStartServerClicked);
    connect(ui->stopServerButton, &QPushButton::clicked, this, &MainWindow::onStopServerClicked);


}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_dbManager;
    delete m_chatLogger;
}

void MainWindow::updatePortNumber()
{
    QTcpServer tempServer;
    if (tempServer.listen(QHostAddress::Any, 0)) {
        int port = tempServer.serverPort();
        ui->portNumberLabel->setText(QString::number(port));
        tempServer.close();
    } else {
        ui->portNumberLabel->setText("Unable to find an available port");
    }
}

void MainWindow::onStartServerClicked()
{
    int port = ui->portNumberLabel->text().toInt();
    if (m_chatServer->start(port)) {
        ui->textBrowser->append("Server started on " + m_chatServer->getIp()+ "::" +QString::number(port));
        ui->startServerButton->setEnabled(false);
        ui->stopServerButton->setEnabled(true);
    } else {
        ui->textBrowser->append("Failed to start server");
    }
}

void MainWindow::onStopServerClicked()
{
    m_chatServer->stop();
    ui->textBrowser->append("Server stopped");
    ui->startServerButton->setEnabled(true);
    ui->stopServerButton->setEnabled(false);
}

void MainWindow::onNewConnection(const QString &message)
{
    ui->textBrowser->append(message);
    m_chatLogger->logConnection(message);
}

void MainWindow::onNewMessage(const QString &message)
{
    QString senderInfo = m_chatServer->getClientInfo(sender());
    QString fullMessage = senderInfo + ": " + message;
    ui->textBrowser->append(fullMessage);
    m_chatLogger->logChat(fullMessage);
}

void MainWindow::onClientDisconnected(const QString &message)
{
    ui->textBrowser->append(message);
    m_chatLogger->logConnection(message);
}

void MainWindow::onClientLogin(QTcpSocket* client, QString& name, QString& team, QString& position)
{
    bool loginSuccess = m_dbManager->checkUser(name, team, position);

    // 클라이언트에게 보낼 응답 JSON 생성
    QJsonObject response;
    response["type"] = "login_response";

    if (loginSuccess) {
        response["status"] = "success";
        response["message"] = "Login successful";
        qDebug() << "Login successful for user:" << name;
    } else {
        response["status"] = "failed";
        response["message"] = "Invalid credentials";
        qDebug() << "Login failed for user:" << name;
    }

    // JSON을 문자열로 변환하여 클라이언트에게 전송
    QJsonDocument doc(response);
    client->write(doc.toJson());
    client->flush();
}

//Connect 없이 직접 연결한다.
void MainWindow::on_pushButton_clicked()
{
    DBManagerWidget *dbManagerWidget = new DBManagerWidget(m_dbManager, m_chatLogger);
    dbManagerWidget->show();
}
