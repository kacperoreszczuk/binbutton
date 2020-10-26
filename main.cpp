#include "widget.h"
#include <iostream>
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget w;
    a.setQuitOnLastWindowClosed(false);

    return a.exec();
}
