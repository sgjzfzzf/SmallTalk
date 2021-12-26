#ifndef LIVEUSERDIALOG_H
#define LIVEUSERDIALOG_H

#include <QDialog>
#include <QObject>
#include <QListWidget>
#include <QPushButton>
#include <QGridLayout>
#include <QJsonArray>

/*
    This class of dialog is used to require other live users information from server.
*/
class LiveUserDialog : public QDialog
{

    Q_OBJECT

public:
    LiveUserDialog();
    void setUsersList(QJsonArray &usersListJson);

public slots:
    void closeDialog();

private:
    QListWidget *usersList;
    QPushButton *closeBtn;
    QGridLayout *mainLayout;
};

#endif // LIVEUSERDIALOG_H
