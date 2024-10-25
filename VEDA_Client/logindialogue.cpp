#include "logindialogue.h"

// logindialog.cpp
LoginDialog::LoginDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle("Login");

    QVBoxLayout *layout = new QVBoxLayout(this);

    nameEdit = new QLineEdit(this);
    teamEdit = new QLineEdit(this);
    positionEdit = new QLineEdit(this);

    layout->addWidget(new QLabel("Name:"));
    layout->addWidget(nameEdit);
    layout->addWidget(new QLabel("Team:"));
    layout->addWidget(teamEdit);
    layout->addWidget(new QLabel("Position:"));
    layout->addWidget(positionEdit);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    loginButton = new QPushButton("Login", this);
    cancelButton = new QPushButton("Cancel", this);

    buttonLayout->addWidget(loginButton);
    buttonLayout->addWidget(cancelButton);
    layout->addLayout(buttonLayout);

    connect(loginButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::reject);
}
