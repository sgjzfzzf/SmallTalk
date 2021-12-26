#include "smalltalkclient.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SmallTalkClient w;
    w.show();
    return a.exec();
}
