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
#include "liveuserdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui
{
    class SmallTalkClient;
}
QT_END_NAMESPACE

/*
    This is the main class of the client dialog.
    It defined several methods to deal with the connection, disconnection and data exchange with server.
    It works at the mode of single thread, so when some unexceptions may lead to that the whole program collapses.
    This is what need to improve in the future version.
*/
class SmallTalkClient : public QDialog
{
    Q_OBJECT

public:
    SmallTalkClient(QWidget *parent = nullptr);
    ~SmallTalkClient();

public slots:
    void connectActionTriggered();
    void disconnectActionTriggered();
    void checkUserActionTriggered();
    void connectToServer(QString, int, QString);
    void disconnectToServer();
    void updateClient();
    void sendFile();
    void sendImg();
    void sendDataToServer();

private:
    const QString FLAG_RECEIVE = "Receive json.";
    Ui::SmallTalkClient *ui;
    QString userName;
    QTcpSocket *clientSocket;
    QMenuBar *menuBar;
    QMenu *connectMenu;
    QMenu *fileMenu;
    QAction *connectAction;
    QAction *disconnectAction;
    QAction *usersListAction;
    QAction *fileSendAction;
    QAction *imgSendAction;
    QListWidget *contentListWidget;
    QTextEdit *contentEdit;
    QPushButton *contentSendBtn;
    QGridLayout *mainLayout;
};
#endif // SMALLTALKCLIENT_H
