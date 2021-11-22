#ifndef SMALLTALKCLIENT_H
#define SMALLTALKCLIENT_H

#include <QDialog>
#include <QTcpSocket>
#include <QLabel>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>

QT_BEGIN_NAMESPACE
namespace Ui { class SmallTalkClient; }
QT_END_NAMESPACE

class SmallTalkClient : public QDialog
{
    Q_OBJECT

public:
    SmallTalkClient(QWidget *parent = nullptr);
    ~SmallTalkClient();

public slots:
    void chooseFile();
    void connectOrDisconnectToServer();

private:
    Ui::SmallTalkClient *ui;
    QTcpSocket *clientSocket;
    QListWidget *contentListWidget;
    QTextEdit *contentEdit;
    QPushButton *contentSendBtn;
    QLabel *fileNameLabel;
    QPushButton *fileChooseBtn;
    QPushButton *fileSendBtn;
    QLabel *addressLabel;
    QLineEdit *addressEdit;
    QLabel *portLabel;
    QLineEdit *portEdit;
    QPushButton *connectBtn;
    QGridLayout *mainLayout;
};
#endif // SMALLTALKCLIENT_H
