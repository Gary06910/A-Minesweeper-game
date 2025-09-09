# ------------------------------------------------
# QtMinesweeper.pro - Qt 项目配置文件
# ------------------------------------------------

# 【关键修复】在这里添加 'multimedia' 模块，
# 这样编译器才能找到 QSoundEffect 等多媒体相关的类。
QT += core gui widgets multimedia

# 最终生成的可执行文件的名称
TARGET = QtMinesweeper

# 项目模板
TEMPLATE = app

# 所有源代码文件
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    cell.cpp \
    game.c \
    scoreboard.c

# 所有头文件
HEADERS += \
    mainwindow.h \
    cell.h \
    game.h \
    scoreboard.h \
    config.h

# 资源文件
RESOURCES += \
    resources.qrc

# Windows平台特定配置
win32 {
    QMAKE_LFLAGS += -mconsole
}

