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
    // Initialize the ui.
    clientSocket = new QTcpSocket;
    menuBar = new QMenuBar;
    connectMenu = new QMenu(QString::fromLocal8Bit("连接"));
    fileMenu = new QMenu(QString::fromLocal8Bit("文件"));
    connectAction = new QAction(QString::fromLocal8Bit("连接到服务器"));
    disconnectAction = new QAction(QString::fromLocal8Bit("从服务器断开"));
    usersListAction = new QAction(QString::fromLocal8Bit("用户列表"));
    fileSendAction = new QAction(QString::fromLocal8Bit("发送文件"));
    imgSendAction = new QAction(QString::fromLocal8Bit("本地表情发送"));
    contentListWidget = new QListWidget;
    contentEdit = new QTextEdit;
    contentSendBtn = new QPushButton(QString::fromLocal8Bit("发送"));
    mainLayout = new QGridLayout(this);

    // Initialize the premium modes of these widgets.
    menuBar->addMenu(connectMenu);
    menuBar->addMenu(fileMenu);
    connectMenu->addAction(connectAction);
    connectMenu->addAction(disconnectAction);
    connectMenu->addAction(usersListAction);
    fileMenu->addAction(fileSendAction);
    fileMenu->addAction(imgSendAction);
    contentSendBtn->setEnabled(false);
    connectAction->setEnabled(true);
    disconnectAction->setEnabled(false);
    usersListAction->setEnabled(false);
    fileSendAction->setEnabled(false);
    imgSendAction->setEnabled(false);
    contentListWidget->setIconSize(QSize(200, 200));

    // Describe the whole ui.
    mainLayout->addWidget(menuBar, 0, 0, 1, 3);
    mainLayout->addWidget(contentListWidget, 1, 0, 1, 3);
    mainLayout->addWidget(contentEdit, 2, 0, 1, 3);
    mainLayout->addWidget(contentSendBtn, 3, 2, 1, 1);

    // Define these connect functions.
    connect(connectAction, SIGNAL(triggered()), this, SLOT(connectActionTriggered()));
    connect(disconnectAction, SIGNAL(triggered()), this, SLOT(disconnectActionTriggered()));
    connect(usersListAction, SIGNAL(triggered()), this, SLOT(checkUserActionTriggered()));
    connect(fileSendAction, SIGNAL(triggered()), this, SLOT(sendFile()));
    connect(imgSendAction, SIGNAL(triggered()), this, SLOT(sendImg()));
    connect(contentSendBtn, SIGNAL(clicked()), this, SLOT(sendDataToServer()));
}

SmallTalkClient::~SmallTalkClient()
{
    // When the program exits, it'll clean the doc directory automatically.
    // So if the user need to save these files shared by this program, they need to finish this before close the client.
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
    // When user want to connect to server, execute the ConnectDialog.
    ConnectDialog *dialog = new ConnectDialog;
    connect(dialog, SIGNAL(returnFileDialog(QString, int, QString)), this, SLOT(connectToServer(QString, int, QString)));
    dialog->show();
}

void SmallTalkClient::disconnectActionTriggered()
{
    // Deal with the situations when disconnection happens, such as set buttons enabled or unenables.
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
    usersListAction->setEnabled(false);
    imgSendAction->setEnabled(false);
}

void SmallTalkClient::checkUserActionTriggered()
{
    // Require the list of live users with json requests.
    QJsonObject json;
    json.insert("userName", userName);
    json.insert("contentType", "usersList");
    json.insert("content", NULL);
    QJsonDocument document;
    document.setObject(json);
    clientSocket->write(document.toJson());
    QByteArray userListContent;
    disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
    if (clientSocket->waitForReadyRead())
    {
        userListContent = clientSocket->readAll();
    }
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
    QJsonParseError error;
    document = QJsonDocument::fromJson(userListContent, &error);
    if (document.isNull() || (error.error != QJsonParseError::NoError))
    {
        qDebug() << "Error";
        return;
    }
    QJsonArray usersList = document.array();
    LiveUserDialog *dialog = new LiveUserDialog;
    dialog->setUsersList(usersList);
    dialog->show();
}

void SmallTalkClient::connectToServer(QString address, int port, QString newUserName)
{
    // The method try to connect to server.
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
    clientSocket->write(userName.toLocal8Bit());
    contentSendBtn->setEnabled(true);
    connectAction->setEnabled(false);
    disconnectAction->setEnabled(true);
    usersListAction->setEnabled(true);
    fileSendAction->setEnabled(true);
    imgSendAction->setEnabled(true);
    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(disconnectToServer()));
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
}

void SmallTalkClient::disconnectToServer()
{
    // The method try to diconnect to server.
    contentSendBtn->setEnabled(false);
    connectAction->setEnabled(true);
    disconnectAction->setEnabled(false);
    usersListAction->setEnabled(false);
    fileSendAction->setEnabled(false);
    imgSendAction->setEnabled(false);
    disconnect(clientSocket, SIGNAL(disconnected()), this, SLOT(disconnectToServer()));
    disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
}

void SmallTalkClient::sendFile()
{
    // The method is used to send files to other clients.
    // When this method begins to work, it will send a json to server first to tell it related information about this file such as name and size.
    // Then it send the whole file to server.
    // It's noticable that the package may be divided into serveral ones because tcp protcol allows only 65535 bytes sent once.
    // So when when server receives data, it depends on the size sent in the first step to judge whether it receives all data.
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
    // Close the connect function to avoid the program deal with these data in a wrong way.
    disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
    if (clientSocket->waitForReadyRead())
    {
        QString receiveData = QString::fromLocal8Bit(clientSocket->readAll());
        if (receiveData == SmallTalkClient::FLAG_RECEIVE)
        {
            clientSocket->write(content);
        }
    }
    // Restart the connect function.
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
}

void SmallTalkClient::updateClient()
{
    // Receive the data from server and update itself.
    // When the server send files big enough, it need to read from socket for several times and make sure it has received all data.
    QByteArray contentByteArray = clientSocket->readAll();
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(contentByteArray, &error);
    if (document.isNull() || (error.error != QJsonParseError::NoError))
    {
        qDebug() << "Error";
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
    // When it receives a file, it will save it in the "doc" directory.
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
        // Continue to read from the socket until the client makes sure to receive all data.
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
    // When it receives a file marked as "img", it will display it in the client dialog.
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
        // If it's a gif file, the client will use a QMovie to display it lively.
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
            contentListWidget->addItem(QString("%1 :").arg(userName));
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
            contentListWidget->addItem(QString("%1 :").arg(userName));
            contentListWidget->addItem(item);
        }
    }
}

void SmallTalkClient::sendImg()
{
    // Send image to the server.
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
    // Close the updateClient method.
    disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
    if (clientSocket->waitForReadyRead())
    {
        QString receiveContent = clientSocket->readAll();
        // When it receives the flag which means that the server has received json data, it will begin to send images.
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
    // Restart the updateClient method.
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(updateClient()));
}

void SmallTalkClient::sendDataToServer()
{
    // Send text data to server.
    QString content = contentEdit->toPlainText().trimmed();
    QJsonObject json;
    json.insert("userName", userName);
    json.insert("contentType", "text");
    json.insert("content", content);
    QJsonDocument document;
    document.setObject(json);
    clientSocket->write(document.toJson());
    contentEdit->clear();
}
