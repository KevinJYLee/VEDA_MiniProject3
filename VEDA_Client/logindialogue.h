// logindialog.h
#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    QString getName() const { return nameEdit->text(); }
    QString getTeam() const { return teamEdit->text(); }
    QString getPosition() const { return positionEdit->text(); }

private:
    QLineEdit *nameEdit;
    QLineEdit *teamEdit;
    QLineEdit *positionEdit;
    QPushButton *loginButton;
    QPushButton *cancelButton;
};

#endif
