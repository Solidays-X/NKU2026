#include <QApplication>
#include "mainwindow.h"
#include "scenemanager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("LoopCampus");

    MainWindow w;
    SceneManager manager(&w);
    w.setSceneManager(&manager);

    manager.startGame();
    w.show();
    return app.exec();
}