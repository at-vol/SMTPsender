#include <QtGui/QApplication>
#include "mailwidget.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MailWidget w;
    w.start();
    
    return a.exec();
}
