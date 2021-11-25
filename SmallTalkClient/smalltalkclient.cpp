#include "smalltalkclient.h"
#include "ui_smalltalkclient.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>

SmallTalkClient::SmallTalkClient(QWidget *parent)
    : QDialog(parent), ui(new Ui::SmallTalkClient)
{
    ui->setupUi(this);
    clientSocket = new QTcpSocket;
    contentListWidget = new QListWidget;
    contentEdit = new QTextEdit;
    contentSendBtn = new QPushButton;
    fileNameLabel = new QLabel;
    fileChooseBtn = new QPushButton;
    fileSendBtn = new QPushButton;
    addressLabel = new QLabel;
    addressEdit = new QLineEdit;
    portLabel = new QLabel;
    portEdit = new QLineEdit;
    userNameLabel = new QLabel;
    userNameEdit = new QLineEdit;
    connectBtn = new QPushButton;
    mainLayout = new QGridLayout(this);

    contentSendBtn->setText(QString::fromLocal8Bit("发送"));
    contentSendBtn->setEnabled(false);
    fileChooseBtn->setText(QString::fromLocal8Bit("选择文件"));
    fileChooseBtn->setEnabled(false);
    fileSendBtn->setText(QString::fromLocal8Bit("发送"));
    fileSendBtn->setEnabled(false);
    addressLabel->setText(QString::fromLocal8Bit("服务器地址"));
    portLabel->setText(QString::fromLocal8Bit("服务器端口"));
    userNameLabel->setText(QString::fromLocal8Bit("用户名"));
    connectBtn->setText(QString::fromLocal8Bit("连接到服务器"));
    connectBtn->setEnabled(true);

    mainLayout->addWidget(contentListWidget, 0, 0, 1, 3);
    mainLayout->addWidget(contentEdit, 1, 0, 1, 3);
    mainLayout->addWidget(contentSendBtn, 2, 2, 1, 1);
    mainLayout->addWidget(fileNameLabel, 3, 0, 1, 1);
    mainLayout->addWidget(fileChooseBtn, 3, 1, 1, 1);
    mainLayout->addWidget(fileSendBtn, 3, 2, 1, 1);
    mainLayout->addWidget(addressLabel, 4, 0, 1, 1);
    mainLayout->addWidget(addressEdit, 4, 1, 1, 2);
    mainLayout->addWidget(portLabel, 5, 0, 1, 1);
    mainLayout->addWidget(portEdit, 5, 1, 1, 2);
    mainLayout->addWidget(userNameLabel, 6, 0, 1, 1);
    mainLayout->addWidget(userNameEdit, 6, 1, 1, 1);
    mainLayout->addWidget(connectBtn, 6, 2, 1, 1);

    connect(contentSendBtn, SIGNAL(clicked()), this, SLOT(sendDataToServer()));
    connect(fileChooseBtn, SIGNAL(clicked()), this, SLOT(chooseFile()));
    connect(fileSendBtn, SIGNAL(clicked()), this, SLOT(sendFile()));
    connect(connectBtn, SIGNAL(clicked()), this, SLOT(connectOrDisconnectToServer()));
    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(disconnectToServer()));
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
}

SmallTalkClient::~SmallTalkClient()
{
    QDir dir(QDir::currentPath());
    if (dir.exists("doc"))
    {
        dir.rmpath("doc");
    }
    delete ui;
}

void SmallTalkClient::sendDataToServer()
{
    QString content = contentEdit->toPlainText();
    QJsonObject json;
    json.insert("userName", userName);
    json.insert("contentType", "text");
    json.insert("content", content);
    QJsonDocument document;
    document.setObject(json);
    clientSocket->write(document.toJson());
}

void SmallTalkClient::chooseFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择发送文件"), ".", "");
    fileNameLabel->setText(filePath);
}

void SmallTalkClient::sendFile()
{
    QString filePath = fileNameLabel->text(), fileName = filePath.split("/").last();
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::information(this, QString::fromLocal8Bit("文件传输"), QString::fromLocal8Bit("文件打开失败"));
        return;
    }
    QByteArray content = file.readAll();
    file.close();
    QJsonObject json;
    json.insert("userName", userName);
    json.insert("contentType", "file");
    json.insert("content", fileName);
    QJsonDocument document;
    document.setObject(json);
    clientSocket->write(document.toJson());
    disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
    if (clientSocket->waitForReadyRead())
    {
        QString receiveData = QString::fromLocal8Bit(clientSocket->readAll());
        if (receiveData == SmallTalkClient::FLAG_RECEIVE)
        {
            clientSocket->write(content);
        }
    }
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
}

void SmallTalkClient::connectOrDisconnectToServer()
{
    if (connectBtn->text() == QString::fromLocal8Bit("连接到服务器"))
    {
        clientSocket->connectToHost(addressEdit->text(), (portEdit->text()).toInt());
        if (clientSocket->state() == QAbstractSocket::UnconnectedState)
        {
            QMessageBox::information(this, QString::fromLocal8Bit("连接服务器"), QString::fromLocal8Bit("连接失败，请检查您的输入"));
            return;
        }
        if ((userName = userNameEdit->text()) == "")
        {
            QMessageBox::information(this, QString::fromLocal8Bit("连接到服务器"), QString::fromLocal8Bit("请输入用户名"));
            return;
        }
        contentSendBtn->setEnabled(true);
        fileChooseBtn->setEnabled(true);
        fileSendBtn->setEnabled(true);
        connectBtn->setText(QString::fromLocal8Bit("从服务器断开"));
    }
    else if (connectBtn->text() == QString::fromLocal8Bit("从服务器断开"))
    {
        clientSocket->disconnectFromHost();
        if (clientSocket->state() == QAbstractSocket::ConnectedState)
        {
            QMessageBox::information(this, QString::fromLocal8Bit("连接服务器"), QString::fromLocal8Bit("断开失败，请重试"));
            return;
        }
        contentSendBtn->setEnabled(false);
        fileChooseBtn->setEnabled(false);
        fileSendBtn->setEnabled(false);
        connectBtn->setText(QString::fromLocal8Bit("连接到服务器"));
    }
}

void SmallTalkClient::disconnectToServer()
{
    contentSendBtn->setEnabled(false);
    fileChooseBtn->setEnabled(false);
    fileSendBtn->setEnabled(false);
    connectBtn->setText(QString::fromLocal8Bit("连接到服务器"));
}

void SmallTalkClient::updateClient()
{
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
        contentListWidget->addItem(QString("%1 : %2").arg(userName, content));
    }
    else if (contentType == "file")
    {
        QString fileName;
        if (json.contains("content"))
        {
            fileName = json.value("content").toString();
        }
        else
        {
            return;
        }
        clientSocket->write(SmallTalkClient::FLAG_RECEIVE.toLocal8Bit());
        disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
        QByteArray contentByteArray, subContentByteArray;
        int len = 0;
        do
        {
            len = 0;
            if (clientSocket->waitForReadyRead(1000))
            {
                subContentByteArray = clientSocket->readAll();
                contentByteArray.append(subContentByteArray);
                len = subContentByteArray.size();
            }
        } while (len == SmallTalkClient::BLOCK_SIZE);
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
        QDir dir(QDir::currentPath());
        if (!dir.exists("doc"))
        {
            dir.mkdir("doc");
        }
        QFile file(QString("doc/%1").arg(fileName));
        if (file.open(QIODevice::WriteOnly))
        {
            file.write(contentByteArray);
            contentListWidget->addItem(QString("%1 send the file %2.").arg(userName, fileName));
        }
        else
        {
            QMessageBox::information(this, QString::fromLocal8Bit("文件传输"), QString::fromLocal8Bit("文件接收错误"));
        }
    }
}
