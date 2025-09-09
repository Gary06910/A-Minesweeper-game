#include "mainwindow.h"
#include <QApplication>
#include <QStyleFactory>
#include <QPalette>
#include <QColor>


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // --- 在显示任何窗口之前，强制设定全局风格和颜色 ---
    // 1. 设置为 "Fusion" 风格。
    // Fusion是Qt自带的一款跨平台风格，外观简洁，最重要的是它不会跟随系统的暗夜/白天模式变化。
    // 这能确保我们的游戏在所有操作系统上看起来都一样。
    app.setStyle(QStyleFactory::create("Fusion"));

    // 2. 创建一个新的调色板，并设定经典的浅灰色背景。
    QPalette palette;
    // 使用 QPalette::ColorRole 来确保我们设置的是窗口背景色
    palette.setColor(QPalette::Window, QColor(220, 220, 220)); // 经典的浅灰色 (#DCDCDC)
    app.setPalette(palette); // 将这个调色板应用到整个应用程序



    // 现在，当 MainWindow 被创建时，它会自动继承我们上面设定的全局风格和颜色。
    MainWindow w;  //创建窗口对象
    w.show();  //显示w窗口对象

    return app.exec(); //启动一个无限循环，这个循环中，程序会不断地侦听用户的操作，比如移动鼠标、点击按钮、按下键盘等
} 

