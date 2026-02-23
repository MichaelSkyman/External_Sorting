#include <QApplication>
#include <QDebug>
#include "gui/main_window.h"

int main(int argc, char *argv[])
{
    qDebug() << "Application starting...";
    QApplication app(argc, argv);
    qDebug() << "QApplication created";

    MainWindow w;
    qDebug() << "MainWindow created";
    w.show();
    qDebug() << "MainWindow shown";

    return app.exec();
}
