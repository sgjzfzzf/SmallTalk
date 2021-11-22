#include "smalltalkserver.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    SmallTalkServer w;
    w.show();
    return a.exec();
}
