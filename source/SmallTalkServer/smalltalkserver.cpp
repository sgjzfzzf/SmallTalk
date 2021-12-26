#include "smalltalkserver.h"
#include "ui_smalltalkserver.h"
#include <QMessageBox>
#include <QHostAddress>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QDir>
#include <QMovie>
#include <QMap>

SmallTalkServer::SmallTalkServer(QWidget *parent)
    : QDialog(parent), ui(new Ui::SmallTalkServer)
{
    ui->setupUi(this);
    // Initialize the ui.
    serverSocket = new QTcpServer;
    contentLabel = new QLabel(QString::fromLocal8Bit("聊天室内容"));
    contentListWidget = new QListWidget;
    portLabel = new QLabel(QString::fromLocal8Bit("端口号"));
    portEdit = new QLineEdit;
    portConfirmBtn = new QPushButton(QString::fromLocal8Bit("创建"));
    mainLayout = new QGridLayout(this);

    contentListWidget->setIconSize(QSize(200, 200));

    // Describe the whole ui.
    mainLayout->addWidget(contentLabel, 0, 0, 1, 3);
    mainLayout->addWidget(contentListWidget, 1, 0, 1, 3);
    mainLayout->addWidget(portLabel, 2, 0);
    mainLayout->addWidget(portEdit, 2, 1);
    mainLayout->addWidget(portConfirmBtn, 2, 2);

    // Define these connect functions.
    connect(portConfirmBtn, SIGNAL(clicked()), this, SLOT(createOrDestoryRoomServer()));
    connect(serverSocket, SIGNAL(newConnection()), this, SLOT(handleNewConnection()));
}

SmallTalkServer::~SmallTalkServer()
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

void SmallTalkServer::updateClientsText(QByteArray content)
{
    // This method will send raw data to all clients connected.
    for (QMap<QString, QTcpSocket *>::iterator it = clientSockets.begin(); it != clientSockets.end(); ++it)
    {
        it.value()->write(content);
    }
}

void SmallTalkServer::updateClientsFile(QByteArray content, QByteArray file)
{
    // This method will send raw data to all clients connected.
    // The difference is it will follow the rules of send files,
    // which means it will send a json firstly and send raw file when it makes sure clients have received the json.
    for (QMap<QString, QTcpSocket *>::iterator it = clientSockets.begin(); it != clientSockets.end(); ++it)
    {
        it.value()->write(content);
        disconnect(it.value(), SIGNAL(readyRead()), this, SLOT(handleNewData()));
        if (it.value()->waitForReadyRead())
        {
            QByteArray receiveData = it.value()->readAll();
            if (receiveData == SmallTalkServer::FLAG_RECEIVE)
            {
                it.value()->write(file);
            }
        }
        connect(it.value(), SIGNAL(readyRead()), this, SLOT(handleNewData()));
    }
}

void SmallTalkServer::createOrDestoryRoomServer()
{
    // Start or close a room.
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
    // When a new client join in this room, it will add a new "UserSocket" to the map to store the socket and the user name.
    QTcpSocket *clientSocket = serverSocket->nextPendingConnection();
    QString userName;
    if (clientSocket->waitForReadyRead())
    {
        userName = QString::fromLocal8Bit(clientSocket->readAll());
    }
    clientSockets.insert(userName, clientSocket);
    QString enterString = QString::fromLocal8Bit("进入聊天室");
    contentListWidget->addItem(QString::fromLocal8Bit("%1 : %2").arg(userName, enterString));
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(handleDisconnection()));
    QJsonObject json;
    json.insert("userName", userName);
    json.insert("contentType", "text");
    json.insert("content", enterString);
    QJsonDocument document;
    document.setObject(json);
    updateClientsText(document.toJson());
}

void SmallTalkServer::handleNewData()
{
    // This method is used to handle new data sent into this server.
    QTcpSocket *clientSocket = (QTcpSocket *)sender();
    QByteArray contentByteArray = clientSocket->readAll();
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(contentByteArray, &error);
    // If the json data is illegal, it'll stop this process.
    if (document.isNull() || (error.error != QJsonParseError::NoError))
    {
        return;
    }
    QJsonObject json = document.object();
    QString userName, contentType;
    // Judge what kind of data it is and decide what it'll do in the next step.
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
        // If it's text data, send it to all clients and update it.
        QString content;
        if (json.contains("content"))
        {
            content = json.value("content").toString();
        }
        else
        {
            return;
        }
        QString msg = QString::fromLocal8Bit("%1 : %2").arg(userName, content);
        contentListWidget->addItem(msg);
        updateClientsText(document.toJson());
    }
    else if (contentType == "file")
    {
        // If it's a file, send it to all clients and updata it.
        // This process will follow the rules of sending files.
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
        // If it's a image, send it to all clients and display it lively.
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
            // If it's a gif file, use a "QMovie" to play it lively.
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
    else if (contentType == "usersList")
    {
        // Send all users' information to target client.
        QStringList userNamesList;
        for (QMap<QString, QTcpSocket *>::iterator it = clientSockets.begin(); it != clientSockets.end(); ++it)
        {
            userNamesList.append(it.key());
        }
        QJsonArray userNamesJsonList = QJsonArray::fromStringList(userNamesList);
        updateClientsText(QJsonDocument(userNamesJsonList).toJson());
    }
}

void SmallTalkServer::handleDisconnection()
{
    // Handle the disconnetion from client, remove the "UserSocket" from the map and disconnet functions.
    // The server will also broadcast this information to all clients.
    QString userName;
    QTcpSocket *clientSocket = (QTcpSocket *)sender();
    for (QMap<QString, QTcpSocket *>::iterator it = clientSockets.begin(); it != clientSockets.end(); ++it)
    {
        if (it.value() == clientSocket)
        {
            userName = it.key();
            break;
        }
    }
    QString leaveString = QString::fromLocal8Bit("离开聊天室");
    contentListWidget->addItem(QString::fromLocal8Bit("%1 : %2").arg(userName, leaveString));
    QJsonObject json;
    json.insert("userName", userName);
    json.insert("contentType", "text");
    json.insert("content", leaveString);
    QJsonDocument document;
    document.setObject(json);
    updateClientsText(document.toJson());
    clientSockets.remove(userName);
    disconnect(clientSocket, SIGNAL(readyRead()), this, SLOT(handleNewData()));
    disconnect(clientSocket, SIGNAL(disconnected()), this, SLOT(handleDisconnection()));
}
