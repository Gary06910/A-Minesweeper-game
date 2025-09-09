#include "cell.h"
#include <QMouseEvent> // 【新增】使用QMouseEvent必须包含此头文件

// 构造函数的实现
Cell::Cell(int row, int col, QWidget *parent)
    : QPushButton(parent), m_row(row), m_col(col)
{
    // 构造函数体，目前为空
}

// 重写的鼠标双击事件处理函数的实现
void Cell::mouseDoubleClickEvent(QMouseEvent *event)
{
    // 首先，检查是不是鼠标左键双击
    if (event->button() == Qt::LeftButton) {
        // 如果是，就发出我们自定义的 doubleClicked 信号
        emit doubleClicked(m_row, m_col);
    }

    // 调用父类的同名函数，以确保默认行为（如果有的话）得以执行
    QPushButton::mouseDoubleClickEvent(event);
}

