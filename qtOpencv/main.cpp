#include "qtOpencv.h"
#include "stdafx.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    qtOpencv w;
    w.show();
    return a.exec();
}
