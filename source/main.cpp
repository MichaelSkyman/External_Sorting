#include <QApplication>
#include <QFile>
#include <QDebug>
#include "gui/main_window.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QFile file(":/assets/style/css.qss");

    if(file.open(QFile::ReadOnly))
    {
        QString style = file.readAll();
        app.setStyleSheet(style);
        file.close();
        qDebug() << "CSS Loaded!";
    }
    else
    {
        qDebug() << "Cannot load CSS!";
    }

    MainWindow w;
    w.show();

    return app.exec();
}