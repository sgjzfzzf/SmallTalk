#include "smalltalkclient.h"
#include "ui_smalltalkclient.h"
#include <QMessageBox>
#include <QFileDialog>

SmallTalkClient::SmallTalkClient(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::SmallTalkClient)
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
    mainLayout->addWidget(portEdit, 5, 1, 1, 1);
    mainLayout->addWidget(connectBtn, 5, 2, 1, 1);

    connect(fileChooseBtn, SIGNAL(clicked()), this, SLOT(chooseFile()));
    connect(connectBtn, SIGNAL(clicked()), this, SLOT(connectOrDisconnectToServer()));
}

SmallTalkClient::~SmallTalkClient()
{
    delete ui;
}

void SmallTalkClient::chooseFile()
{
    QString filePath = QFileDialog::getOpenFileName(this, QString::fromLocal8Bit("选择发送文件"), ".", "");
    fileNameLabel->setText(filePath);
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
