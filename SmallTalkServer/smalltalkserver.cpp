#include "smalltalkserver.h"
#include "ui_smalltalkserver.h"
#include <QMessageBox>
#include <QHostAddress>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDir>

SmallTalkServer::SmallTalkServer(QWidget *parent)
    : QDialog(parent), ui(new Ui::SmallTalkServer)
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
    connect(serverSocket, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
}

SmallTalkServer::~SmallTalkServer()
{
    delete ui;
}

void SmallTalkServer::updateClients(QByteArray content)
{
    for (int i = 0; i < clientSockets.length(); ++i)
    {
        clientSockets[i]->write(content);
    }
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

void SmallTalkServer::handleNewConnection()
{
    QTcpSocket *clientSocket = serverSocket->nextPendingConnection();
    clientSockets.append(clientSocket);
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
}

void SmallTalkServer::handleNewData()
{
    QTcpSocket *clientSocket = (QTcpSocket *)sender();
    QByteArray contentByteArray = clientSocket->readAll();
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(contentByteArray, &error);
    if (document.isNull() || (error.error != QJsonParseError::NoError))
    {
        //Receive error json.
        return;
    }
    QJsonObject json = document.object();
    QString userName, contentType;
    if (json.contains("userName"))
    {
        userName = json.value("userName").toString();
    }
    else
    {
        return;
    }
    if (json.contains("contentType"))
    {
        contentType = json.value("contentType").toString();
    }
    else
    {
        return;
    }
    if (contentType == "text")
    {
        QString content;
        if (json.contains("content"))
        {
            content = json.value("content").toString();
        }
        else
        {
            return;
        }
        QString msg = userName + ":" + content;
        contentListWidget->addItem(msg);
    }
    else if (contentType == "file")
    {
        QString fileName;
        if (json.contains("fileName"))
        {
            fileName = json.value("fileName").toString();
        }
        else
        {
            return;
        }
        contentListWidget->addItem(QString("%1 send the file %2.").arg(userName, fileName));
    }
    updateClients(document.toJson());
}
