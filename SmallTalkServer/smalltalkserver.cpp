#include "smalltalkserver.h"
#include "ui_smalltalkserver.h"
#include <QMessageBox>
#include <QHostAddress>

SmallTalkServer::SmallTalkServer(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SmallTalkServer)
{
    ui->setupUi(this);
    serverSocket = new QTcpServer;
    contentLabel = new QLabel;
    contentListWidget = new QListWidget;
    portLabel = new QLabel;
    portEdit = new QLineEdit;
    portConfirmBtn = new QPushButton;
    mainLayout = new QGridLayout(this);
    contentLabel->setText(QString::fromLocal8Bit("聊天室内容"));
    portLabel->setText(QString::fromLocal8Bit("端口号"));
    portConfirmBtn->setText(QString::fromLocal8Bit("创建"));
    mainLayout->addWidget(contentLabel, 0, 0, 1, 3);
    mainLayout->addWidget(contentListWidget, 1, 0, 1, 3);
    mainLayout->addWidget(portLabel, 2, 0);
    mainLayout->addWidget(portEdit, 2, 1);
    mainLayout->addWidget(portConfirmBtn, 2, 2);
    connect(portConfirmBtn, SIGNAL(clicked()), this, SLOT(createOrDestoryRoomServer()));
}

SmallTalkServer::~SmallTalkServer()
{
    delete ui;
}

void SmallTalkServer::createOrDestoryRoomServer()
{
    if (portConfirmBtn->text() == QString::fromLocal8Bit("创建"))
    {
        port = (portEdit->text()).toInt();
        if (port <= 0)
        {
            QMessageBox::information(this, QString::fromLocal8Bit("创建聊天室"), QString::fromLocal8Bit("端口号不合法，请仔细检查"));
            return;
        }
        if (!serverSocket->listen(QHostAddress::Any, port))
        {
            QMessageBox::information(this, QString::fromLocal8Bit("创建聊天室"), QString::fromLocal8Bit("端口监听失败"));
            return;
        }
        portConfirmBtn->setText(QString::fromLocal8Bit("关闭"));
    }
    else if (portConfirmBtn->text() == QString::fromLocal8Bit("关闭"))
    {
        serverSocket->close();
        portConfirmBtn->setText(QString::fromLocal8Bit("创建"));
    }
}
