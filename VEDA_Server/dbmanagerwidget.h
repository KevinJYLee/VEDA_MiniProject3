#ifndef DBMANAGERWIDGET_H
#define DBMANAGERWIDGET_H

#include <QWidget>
#include "dbmanager.h"
#include "chatlogger.h"

namespace Ui {
class DBManagerWidget;
}

class DBManagerWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DBManagerWidget(DBManager *dbManager, ChatLogger *chatLogger, QWidget *parent = nullptr);
    ~DBManagerWidget();

private slots:
    void onAddButtonClicked();
    void onEditButtonClicked();
    void onDeleteButtonClicked();
    void onChatHistoryButtonClicked();
    void onConnectionHistoryButtonClicked();

private:
    Ui::DBManagerWidget *ui;
    DBManager *m_dbManager;
    ChatLogger *m_chatLogger;

    void setupUserTable();
    void showUserDialog(int userId = -1);
    void showHistoryDialog(const QString& title, const QString& filePath);

};

#endif // DBMANAGERWIDGET_H
