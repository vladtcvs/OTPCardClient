#include "otpclient.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    OTPClient w;
    w.show();
    return a.exec();
}
