#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "chatserver.h"
#include "chatlogger.h"
#include "dbmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    ChatServer *m_chatServer;
    ChatLogger *m_chatLogger;
    DBManager *m_dbManager;

    void updatePortNumber();

private slots:
    void onStartServerClicked();
    void onStopServerClicked();
    void onNewConnection(const QString &message);
    void onNewMessage(const QString &message);
    void onClientDisconnected(const QString &message);
    void onClientLogin(QTcpSocket* client, QString& name, QString& team, QString& position);



    void on_pushButton_clicked();
};

#endif // MAINWINDOW_H
