#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logindialogue.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), m_chatclient(new ChatClient())
{
    ui->setupUi(this);

    connect(ui->connectButton, &QPushButton::clicked, this, &MainWindow::onConnectServerClicked);
    connect(m_chatclient, &ChatClient::loginResult, this, &MainWindow::handleLoginResult);

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

MainWindow::~MainWindow()
{
    delete ui;
}
