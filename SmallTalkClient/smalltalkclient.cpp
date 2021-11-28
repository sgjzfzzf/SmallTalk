#include "smalltalkclient.h"
#include "ui_smalltalkclient.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMovie>

SmallTalkClient::SmallTalkClient(QWidget *parent)
    : QDialog(parent), ui(new Ui::SmallTalkClient)
{
    ui->setupUi(this);
    clientSocket = new QTcpSocket;
    menuBar = new QMenuBar;
    connectMenu = new QMenu(QString::fromLocal8Bit("连接"));
    fileMenu = new QMenu(QString::fromLocal8Bit("文件"));
    connectAction = new QAction(QString::fromLocal8Bit("连接到服务器"));
    disconnectAction = new QAction(QString::fromLocal8Bit("从服务器断开"));
    fileSendAction = new QAction(QString::fromLocal8Bit("发送文件"));
    imgSendAction = new QAction(QString::fromLocal8Bit("本地表情发送"));
    contentListWidget = new QListWidget;
    contentEdit = new QTextEdit;
    contentSendBtn = new QPushButton(QString::fromLocal8Bit("发送"));
    mainLayout = new QGridLayout(this);

    menuBar->addMenu(connectMenu);
    menuBar->addMenu(fileMenu);
    connectMenu->addAction(connectAction);
    connectMenu->addAction(disconnectAction);
    fileMenu->addAction(fileSendAction);
    fileMenu->addAction(imgSendAction);
    contentSendBtn->setEnabled(false);
    connectAction->setEnabled(true);
    disconnectAction->setEnabled(false);
    fileSendAction->setEnabled(false);
    imgSendAction->setEnabled(false);
    contentListWidget->setIconSize(QSize(200, 200));

    mainLayout->addWidget(menuBar, 0, 0, 1, 3);
    mainLayout->addWidget(contentListWidget, 1, 0, 1, 3);
    mainLayout->addWidget(contentEdit, 2, 0, 1, 3);
    mainLayout->addWidget(contentSendBtn, 3, 2, 1, 1);

    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(disconnectToServer()));
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
    connect(connectAction, SIGNAL(triggered()), this, SLOT(connectActionTriggered()));
    connect(disconnectAction, SIGNAL(triggered()), this, SLOT(disconnectActionTriggered()));
    connect(fileSendAction, SIGNAL(triggered()), this, SLOT(sendFile()));
    connect(imgSendAction, SIGNAL(triggered()), this, SLOT(sendImg()));
    connect(contentSendBtn, SIGNAL(clicked()), this, SLOT(sendDataToServer()));
}

SmallTalkClient::~SmallTalkClient()
{
    QDir dir(QDir::currentPath());
    if (dir.exists("doc"))
    {
        dir.cd("doc");
        dir.removeRecursively();
    }
    delete ui;
}

void SmallTalkClient::connectActionTriggered()
{
    ConnectDialog *dialog = new ConnectDialog;
    connect(dialog, SIGNAL(returnFileDialog(QString, int, QString)), this, SLOT(connectToServer(QString, int, QString)));
    dialog->show();
}

void SmallTalkClient::disconnectActionTriggered()
{
    clientSocket->disconnectFromHost();
    if (clientSocket->state() == QAbstractSocket::ConnectedState)
    {
        QMessageBox::information(this, QString::fromLocal8Bit("连接服务器"), QString::fromLocal8Bit("断开失败，请重试"));
        return;
    }
    contentSendBtn->setEnabled(false);
    connectAction->setEnabled(true);
    disconnectAction->setEnabled(false);
    fileSendAction->setEnabled(false);
    imgSendAction->setEnabled(false);
}

void SmallTalkClient::connectToServer(QString address, int port, QString newUserName)
{
    if ((userName = newUserName) == "")
    {
        QMessageBox::information(this, QString::fromLocal8Bit("连接到服务器"), QString::fromLocal8Bit("请输入用户名"));
        return;
    }
    clientSocket->connectToHost(address, port);
    if (clientSocket->state() == QAbstractSocket::UnconnectedState)
    {
        QMessageBox::information(this, QString::fromLocal8Bit("连接服务器"), QString::fromLocal8Bit("连接失败，请检查您的输入"));
        return;
    }
    contentSendBtn->setEnabled(true);
    connectAction->setEnabled(false);
    disconnectAction->setEnabled(true);
    fileSendAction->setEnabled(true);
    imgSendAction->setEnabled(true);
}

void SmallTalkClient::disconnectToServer()
{
    contentSendBtn->setEnabled(false);
    connectAction->setEnabled(true);
    disconnectAction->setEnabled(false);
    fileSendAction->setEnabled(false);
    imgSendAction->setEnabled(false);
}

void SmallTalkClient::sendFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择发送文件"), ".", "");
    if (filePath == "")
    {
        return;
    }
    QString fileName = filePath.split("/").last();
    int fileSize;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
    {
        return;
    }
    fileSize = file.size();
    QByteArray content = file.readAll();
    file.close();
    QJsonObject json, fileInfo;
    json.insert("userName", userName);
    json.insert("contentType", "file");
    fileInfo.insert("fileName", fileName);
    fileInfo.insert("fileSize", fileSize);
    json.insert("content", fileInfo);
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
        int fileSize = 0;
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
        clientSocket->write(SmallTalkClient::FLAG_RECEIVE.toLocal8Bit());
        disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
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
            contentListWidget->addItem(QString::fromLocal8Bit("%1 发送了文件 %2.").arg(userName, fileName));
        }
        else
        {
            QMessageBox::information(this, QString::fromLocal8Bit("文件传输"), QString::fromLocal8Bit("文件接收错误"));
        }
    }
    else if (contentType == "img")
    {
        qDebug() << "enter img";
        int imgSize;
        QString imgName;
        QJsonObject imgInfo;
        if (json.contains("content"))
        {
            imgInfo = json.value("content").toObject();
            if (imgInfo.contains("imgName"))
            {
                imgName = imgInfo.value("imgName").toString();
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
        disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
        clientSocket->write(SmallTalkClient::FLAG_RECEIVE.toLocal8Bit());
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
        connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
        if (imgName.split('.').last() == "gif")
        {
            QDir dir(QDir::currentPath());
            if (!dir.exists("doc"))
            {
                dir.mkdir("doc");
            }
            imgName = QString("%1/%2").arg("doc", imgName);
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

void SmallTalkClient::sendImg()
{
    QDir dir;
    dir.currentPath();
    if (!dir.exists("img"))
    {
        dir.mkdir("img");
    }
    dir.cd("img");
    QString imgPath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("本地表情发送"), dir.currentPath() + "/img", "*.png;;*.jpg;;*.jpeg;;*.gif");
    if (imgPath == "")
    {
        return;
    }
    QFile img(imgPath);
    img.open(QIODevice::ReadOnly);
    if (!img.isOpen())
    {
        QMessageBox::information(this, QString::fromLocal8Bit("本地表情"), QString::fromLocal8Bit("获取本地资源失败"));
        return;
    }
    int imgSize = img.size();
    QByteArray content = img.readAll();
    img.close();
    QJsonObject json, imgInfo;
    json.insert("userName", userName);
    json.insert("contentType", "img");
    imgInfo.insert("imgName", imgPath.split('/').last());
    imgInfo.insert("imgSize", imgSize);
    json.insert("content", imgInfo);
    QJsonDocument document;
    document.setObject(json);
    clientSocket->write(document.toJson());
    disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
    if (clientSocket->waitForReadyRead())
    {
        QString receiveContent = clientSocket->readAll();
        if (receiveContent == SmallTalkClient::FLAG_RECEIVE)
        {
            clientSocket->write(content);
        }
        else
        {
            QMessageBox::information(this, QString::fromLocal8Bit("本地表情"), QString::fromLocal8Bit("接收响应失败"));
            connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
            return;
        }
    }
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
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
