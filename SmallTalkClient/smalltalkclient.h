#ifndef SMALLTALKCLIENT_H
#define SMALLTALKCLIENT_H

#include <QDialog>
#include <QTcpSocket>
#include <QMenu>
#include <QMenuBar>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>
#include "connectdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class SmallTalkClient;
}
QT_END_NAMESPACE

class SmallTalkClient : public QDialog
{
    Q_OBJECT

public:
    SmallTalkClient(QWidget *parent = nullptr);
    ~SmallTalkClient();

public slots:
    void connectActionTriggered();
    void disconnectActionTriggered();
    void connectToServer(QString, int, QString);
    void disconnectToServer();
    void updateClient();
    void sendFile();
    void sendImg();
    void sendDataToServer();

private:
    const int BLOCK_SIZE = 0x10000;
    const QString FLAG_RECEIVE = "Receive json.";
    Ui::SmallTalkClient *ui;
    QString userName;
    QTcpSocket *clientSocket;
    QMenuBar *menuBar;
    QMenu *connectMenu;
    QMenu *fileMenu;
    QAction *connectAction;
    QAction *disconnectAction;
    QAction *fileSendAction;
    QAction *imgSendAction;
    QListWidget *contentListWidget;
    QTextEdit *contentEdit;
    QPushButton *contentSendBtn;
    QGridLayout *mainLayout;
};
#endif // SMALLTALKCLIENT_H
