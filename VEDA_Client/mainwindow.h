#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMessageBox>
#include <QThread>
#include <QScrollBar>
#include <QFileDialog>

#include "chatclient.h"


QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectServerClicked();
    void onMsgSendClicked();
    bool onFileSendClicked();
    void handleLoginResult(bool success, QString message);
    void onMsgReceived(QString& message);

private:
    Ui::MainWindow *ui;

    int Port;
    QString IP;
    ChatClient* m_chatclient;

    QScrollBar* m_scrollbar;
};
#endif // MAINWINDOW_H
