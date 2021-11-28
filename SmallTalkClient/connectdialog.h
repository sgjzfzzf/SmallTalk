#ifndef CONNECTDIALOG_H
#define CONNECTDIALOG_H

#include <QDialog>
#include <QObject>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>

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
