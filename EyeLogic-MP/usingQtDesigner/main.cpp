#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    Widget w;
//    QPushButton *button = app.findChild<QPushButton*>("Quit Eyelogic");
//    QObject::connect(button, SIGNAL(clicked()), &app, SLOT(quit()));


    w.setWindowTitle("Welcome to Eyelogic Setup");
    w.show();
    return app.exec();
}

