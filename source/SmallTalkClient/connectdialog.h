#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>
#include <QObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>

/*
    The class of dialog which is used to connect client to server.
    This dialog will collect related information and return it to the client so the client can make use of it and build connection with server.
*/
class ConnectDialog : public QDialog
{

    Q_OBJECT

private:
    QLabel *addressLabel;
    QLineEdit *addressEdit;
    QLabel *portLabel;
    QLineEdit *portEdit;
    QLabel *userNameLabel;
    QLineEdit *userNameEdit;
    QGridLayout *mainLayout;
    QPushButton *connectBtn;

signals:
    void returnFileDialog(QString address, int port, QString userName);

public slots:
    void buttonClicked();

public:
    ConnectDialog();
};

#endif // CONNECTDIALOG_H
