#include <QtGui/QApplication>
#include <QDebug>
#include "Factory.h"


int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Factory factor;

//    while(!factor.IsQuiting())
//    {

//        a.processEvents(QEventLoop::AllEvents, 1);       // Process QT Evetns
//        factor.ProcessEvents(); // Process our audio Library events

//    }

   int res = a.exec();

   qDebug() <<  "Quit.." ;

   return res;
}
