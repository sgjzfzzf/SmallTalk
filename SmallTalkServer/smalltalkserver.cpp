#include "smalltalkserver.h"
#include "ui_smalltalkserver.h"
#include <QMessageBox>
#include <QHostAddress>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QDir>
#include <QMovie>

SmallTalkServer::SmallTalkServer(QWidget *parent)
    : QDialog(parent), ui(new Ui::SmallTalkServer)
{
    ui->setupUi(this);
    serverSocket = new QTcpServer;
    contentLabel = new QLabel(QString::fromLocal8Bit("聊天室内容"));
    contentListWidget = new QListWidget;
    portLabel = new QLabel(QString::fromLocal8Bit("端口号"));
    portEdit = new QLineEdit;
    portConfirmBtn = new QPushButton(QString::fromLocal8Bit("创建"));
    mainLayout = new QGridLayout(this);

    contentListWidget->setIconSize(QSize(200, 200));

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
    QDir dir(QDir::currentPath());
    if (dir.exists("doc"))
    {
        dir.cd("doc");
        dir.removeRecursively();
    }
    delete ui;
}

void SmallTalkServer::updateClientsText(QByteArray content)
{
    for (int i = 0; i < clientSockets.length(); ++i)
    {
        clientSockets[i]->write(content);
    }
}

void SmallTalkServer::updateClientsFile(QByteArray content, QByteArray file)
{
    for (int i = 0; i < clientSockets.length(); ++i)
    {
        clientSockets[i]->write(content);
        disconnect(clientSockets[i], SIGNAL(readyRead()), this, SLOT(handleNewData()));
        if (clientSockets[i]->waitForReadyRead())
        {
            QByteArray receiveData = clientSockets[i]->readAll();
            if (receiveData == SmallTalkServer::FLAG_RECEIVE)
            {
                clientSockets[i]->write(file);
            }
        }
        connect(clientSockets[i], SIGNAL(readyRead()), this, SLOT(handleNewData()));
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
        updateClientsText(document.toJson());
    }
    else if (contentType == "file")
    {
        int fileSize;
        QString fileName;
        QJsonObject fileInfo;
        if (json.contains("content"))
        {
            fileInfo = json.value("content").toObject();
            if (fileInfo.contains("fileName"))
            {
                fileName = fileInfo.value("fileName").toString();
            }
            else
            {
                return;
            }
            if (fileInfo.contains("fileSize"))
            {
                fileSize = fileInfo.value("fileSize").toInt();
            }
            else
            {
                return;
            }
        }
        else
        {
            return;
        }
        contentListWidget->addItem(QString::fromLocal8Bit("%1 发送了文件 %2.").arg(userName, fileName));
        disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
        clientSocket->write(SmallTalkServer::FLAG_RECEIVE.toLocal8Bit());
        QByteArray contentByteArray, subContentByteArray;
        while (fileSize > 0)
        {
            if (clientSocket->waitForReadyRead())
            {
                subContentByteArray = clientSocket->readAll();
                contentByteArray.append(subContentByteArray);
                fileSize -= subContentByteArray.size();
            }
        }
        updateClientsFile(document.toJson(), contentByteArray);
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
    }
    else if (contentType == "img")
    {
        int imgSize;
        QString imgName;
        QJsonObject imgInfo;
        if (json.contains("content"))
        {
            imgInfo = json.value("content").toObject();
            if (imgInfo.contains("imgName"))
            {
                imgName = imgInfo.value("imgName").toString().split('/').last();
            }
            else
            {
                return;
            }
            if (imgInfo.contains("imgSize"))
            {
                imgSize = imgInfo.value("imgSize").toInt();
            }
            else
            {
                return;
            }
        }
        else
        {
            return;
        }
        disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
        clientSocket->write(SmallTalkServer::FLAG_RECEIVE.toLocal8Bit());
        QByteArray contentByteArray, subContentByteArray;
        while (imgSize > 0)
        {
            if (clientSocket->waitForReadyRead())
            {
                subContentByteArray = clientSocket->readAll();
                contentByteArray.append(subContentByteArray);
                imgSize -= subContentByteArray.size();
            }
        }
        updateClientsFile(document.toJson(), contentByteArray);
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
        if (imgName.split('.').last() == "gif")
        {
            QDir dir(QDir::currentPath());
            if (!dir.exists("doc"))
            {
                dir.mkdir("doc");
            }
            dir.cd("doc");
            imgName = QString("%1/%2").arg(dir.absolutePath(), imgName);
            QFile imgFile(imgName);
            if (!imgFile.open(QIODevice::WriteOnly))
            {
                return;
            }
            imgFile.write(contentByteArray);
            QLabel *gifLabel = new QLabel;
            QMovie *gif = new QMovie(imgName);
            gif->setScaledSize(QSize(200, 200));
            if (!gif->isValid())
            {
                delete gifLabel;
                return;
            }
            gifLabel->setMovie(gif);
            QListWidgetItem *item = new QListWidgetItem;
            item->setSizeHint(QSize(200, 200));
            contentListWidget->addItem(QString("%1:").arg(userName));
            contentListWidget->addItem(item);
            contentListWidget->setItemWidget(item, gifLabel);
            gif->start();
        }
        else
        {
            QPixmap pixmap;
            pixmap.loadFromData(contentByteArray);
            QIcon img(pixmap);
            QListWidgetItem *item = new QListWidgetItem(img, "");
            contentListWidget->addItem(QString("%1:").arg(userName));
            contentListWidget->addItem(item);
        }
    }
}
