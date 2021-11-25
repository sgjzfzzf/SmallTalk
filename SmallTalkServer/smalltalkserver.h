#ifndef SMALLTALKSERVER_H
#define SMALLTALKSERVER_H

#include <QDialog>
#include <QTcpServer>
#include <QTcpSocket>
#include <QLabel>
#include <QListWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>

QT_BEGIN_NAMESPACE
namespace Ui
{
    class SmallTalkServer;
}
QT_END_NAMESPACE

class SmallTalkServer : public QDialog
{
    Q_OBJECT

public:
    SmallTalkServer(QWidget *parent = nullptr);
    ~SmallTalkServer();
    void updateClientsText(QByteArray);
    void updateClientsFile(QByteArray, QByteArray);

public slots:
    void createOrDestoryRoomServer();
    void handleNewConnection();
    void handleNewData();

private:
    const int BLOCK_SIZE = 0x10000;
    const QString FLAG_RECEIVE = "Receive json.";
    Ui::SmallTalkServer *ui;
    int port;
    QTcpServer *serverSocket;
    QList<QTcpSocket *> clientSockets;
    QLabel *contentLabel;
    QListWidget *contentListWidget;
    QLabel *portLabel;
    QLineEdit *portEdit;
    QPushButton *portConfirmBtn;
    QGridLayout *mainLayout;
};
#endif // SMALLTALKSERVER_H
