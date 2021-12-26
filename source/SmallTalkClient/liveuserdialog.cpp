#include "liveuserdialog.h"

LiveUserDialog::LiveUserDialog()
{
    usersList = new QListWidget;
    closeBtn = new QPushButton(QString::fromLocal8Bit("关闭"));
    mainLayout = new QGridLayout(this);

    mainLayout->addWidget(usersList, 0, 0, 1, 3);
    mainLayout->addWidget(closeBtn, 1, 2, 1, 1);
    connect(closeBtn, SIGNAL(clicked()), this, SLOT(closeDialog()));
}

void LiveUserDialog::setUsersList(QJsonArray &usersListJson)
{
    for (QJsonArray::iterator it = usersListJson.begin(); it != usersListJson.end(); ++it)
    {
        QString userName = it->toString();
        usersList->addItem(userName);
    }
}

void LiveUserDialog::closeDialog()
{
    close();
}
