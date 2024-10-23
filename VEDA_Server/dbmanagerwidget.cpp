#include "dbmanagerwidget.h"
#include "ui_dbmanagerwidget.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QDialog>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>
#include <QFormLayout>
#include <QPlainTextEdit>
#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

DBManagerWidget::DBManagerWidget(DBManager* dbManager, ChatLogger* chatLogger, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::DBManagerWidget)
    , m_dbManager(dbManager)
    , m_chatLogger(chatLogger)
{
    ui->setupUi(this);
    setupUserTable();

    connect(ui->addButton, &QPushButton::clicked, this, &DBManagerWidget::onAddButtonClicked);
    connect(ui->editButton, &QPushButton::clicked, this, &DBManagerWidget::onEditButtonClicked);
    connect(ui->deleteButton, &QPushButton::clicked, this, &DBManagerWidget::onDeleteButtonClicked);
    connect(ui->chatHistoryButton, &QPushButton::clicked, this, &DBManagerWidget::onChatHistoryButtonClicked);
    connect(ui->connectionHistoryButton, &QPushButton::clicked, this, &DBManagerWidget::onConnectionHistoryButtonClicked);
}

DBManagerWidget::~DBManagerWidget()
{
    delete ui;
}

void DBManagerWidget::setupUserTable()
{
    ui->userTableView->setModel(m_dbManager->getUserModel());
    ui->userTableView->resizeColumnsToContents();
}

void DBManagerWidget::onAddButtonClicked()
{
    showUserDialog();
}

void DBManagerWidget::onEditButtonClicked()
{
    QModelIndex currentIndex = ui->userTableView->currentIndex();
    if (currentIndex.isValid()) {
        int userId = m_dbManager->getUserModel()->data(m_dbManager->getUserModel()->index(currentIndex.row(), 0)).toInt();
        showUserDialog(userId);
    } else {
        QMessageBox::warning(this, "Warning", "Please select a user to edit.");
    }
}

void DBManagerWidget::onDeleteButtonClicked()
{
    QModelIndex currentIndex = ui->userTableView->currentIndex();
    if (currentIndex.isValid()) {
        int userId = m_dbManager->getUserModel()->data(m_dbManager->getUserModel()->index(currentIndex.row(), 0)).toInt();
        if (m_dbManager->deleteUser(userId)) {
            QMessageBox::information(this, "Success", "User deleted successfully.");
        } else {
            QMessageBox::warning(this, "Error", "Failed to delete user.");
        }
    } else {
        QMessageBox::warning(this, "Warning", "Please select a user to delete.");
    }
}

void DBManagerWidget::showUserDialog(int userId)
{
    QDialog dialog(this);
    dialog.setWindowTitle(userId == -1 ? "Add User" : "Edit User");

    QFormLayout* layout = new QFormLayout(&dialog);

    QLineEdit* nameEdit = new QLineEdit(&dialog);
    QLineEdit* teamEdit = new QLineEdit(&dialog);
    QLineEdit* positionEdit = new QLineEdit(&dialog);

    layout->addRow("Name:", nameEdit);
    layout->addRow("Team:", teamEdit);
    layout->addRow("Position:", positionEdit);

    QPushButton* saveButton = new QPushButton("Save", &dialog);
    QPushButton* cancelButton = new QPushButton("Cancel", &dialog);

    layout->addRow(saveButton, cancelButton);

    if (userId != -1) {
        // Load existing user info
        QSqlTableModel* model = m_dbManager->getUserModel();
        QModelIndex index = model->index(model->match(model->index(0, 0), Qt::DisplayRole, userId, 1, Qt::MatchExactly).first().row(), 0);
        
        nameEdit->setText(model->data(model->index(index.row(), 1)).toString());
        teamEdit->setText(model->data(model->index(index.row(), 2)).toString());
        positionEdit->setText(model->data(model->index(index.row(), 3)).toString());
    }

    connect(saveButton, &QPushButton::clicked, [&]() {
        QString name = nameEdit->text();
        QString team = teamEdit->text();
        QString position = positionEdit->text();

        if (name.isEmpty() || team.isEmpty() || position.isEmpty()) {
            QMessageBox::warning(&dialog, "Error", "All fields must be filled.");
            return;
        }

        bool success;
        if (userId == -1) {
            success = m_dbManager->addUser(name, team, position);
        } else {
            success = m_dbManager->updateUser(userId, name, team, position);
        }

        if (success) {
            dialog.accept();
        } else {
            QMessageBox::critical(&dialog, "Error", "Failed to save user information.");
        }
    });

    connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        setupUserTable();  // Update table view
    }
}


void DBManagerWidget::onChatHistoryButtonClicked()
{
    showHistoryDialog("Chat History", "chat_log.txt");
}

void DBManagerWidget::onConnectionHistoryButtonClicked()
{
    showHistoryDialog("Connection History", "connection_log.txt");
}

void DBManagerWidget::showHistoryDialog(const QString& title, const QString& fileName)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.resize(600, 400);

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QPlainTextEdit* textEdit = new QPlainTextEdit(&dialog);
    textEdit->setReadOnly(true);

    // DBManager를 통해 데이터 경로를 얻습니다.
    QString filePath = DBManager::getDataPath() + QDir::separator() + fileName;

    QFile file(filePath);
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        textEdit->setPlainText(in.readAll());
        file.close();
    } else {
        textEdit->setPlainText("Unable to open log file: " + filePath);
    }

    layout->addWidget(textEdit);

    dialog.exec();
}
