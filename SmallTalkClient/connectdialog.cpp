#include "connectdialog.h"
#include <QDebug>

ConnectDialog::ConnectDialog()
{
    addressLabel = new QLabel(QString::fromLocal8Bit("服务器地址"));
    addressEdit = new QLineEdit;
    portLabel = new QLabel(QString::fromLocal8Bit("服务器端口"));
    portEdit = new QLineEdit;
    userNameLabel = new QLabel(QString::fromLocal8Bit("用户名"));
    userNameEdit = new QLineEdit;
    connectBtn = new QPushButton(QString::fromLocal8Bit("连接到服务器"));
    mainLayout = new QGridLayout(this);

    mainLayout->addWidget(addressLabel, 0, 0, 1, 1);
    mainLayout->addWidget(addressEdit, 0, 1, 1, 2);
    mainLayout->addWidget(portLabel, 1, 0, 1, 1);
    mainLayout->addWidget(portEdit, 1, 1, 1, 2);
    mainLayout->addWidget(userNameLabel, 2, 0, 1, 1);
    mainLayout->addWidget(userNameEdit, 2, 1, 1, 1);
    mainLayout->addWidget(connectBtn, 2, 2, 1, 1);

    connect(connectBtn, SIGNAL(clicked()), this, SLOT(buttonClicked()));
}

void ConnectDialog::buttonClicked()
{
    emit returnFileDialog(addressEdit->text(), portEdit->text().toInt(), userNameEdit->text());
    close();
}
