QT       += core gui widgets
CONFIG   += c++17
TARGET   = LoopCampus
TEMPLATE = app

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    gamestate.cpp \
    inventorywindow.cpp \
    main.cpp \
    mainwindow.cpp \
    scenemanager.cpp \
    storydispaly.cpp

HEADERS += \
    gamestate.h \
    inventorywindow.h \
    mainwindow.h \
    scenemanager.h \
    storydisplay.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    LoopCampus.gitignore \
    scenario_data.json

# 自动复制 scenario_data.json 到构建目录
copydata.commands = $(COPY_FILE) $$shell_path($$PWD/scenario_data.json) $$shell_path($$OUT_PWD)
first.depends = $(first) copydata
export(first.depends)
export(copydata.commands)
QMAKE_EXTRA_TARGETS += first copydata