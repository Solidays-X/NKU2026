#include "mainwindow.h"

#include <QApplication>
#include <QFontDatabase>
#include <QFile>
#include <QTextStream>

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("NankaiLoop");
    app.setOrganizationName("Nankai");

    QFont f = app.font();
#ifdef Q_OS_WIN
    f.setFamily("Microsoft YaHei");
#endif
    f.setPointSize(11);
    app.setFont(f);

    MainWindow w;
    w.show();

    return app.exec();
}
