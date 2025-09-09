#ifndef CELL_H
#define CELL_H

// 【新增】必须包含<QPushButton>才能继承它。
// 【新增】Q_OBJECT宏和信号/槽机制需要<QObject>。
#include <QPushButton>
#include <QObject>

// Cell类公开继承自QPushButton
class Cell : public QPushButton
{
    // Q_OBJECT 宏是所有自定义信号/槽的类所必需的
    Q_OBJECT

public:
    // 构造函数，传入坐标和父窗口指针
    explicit Cell(int row, int col, QWidget *parent = nullptr);

signals:
    // 自定义信号，当按钮被双击时发出，并携带自己的坐标
    void doubleClicked(int row, int col);

protected:
    // 重写父类的鼠标双击事件处理函数
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    int m_row; // 存储自己的行坐标
    int m_col; // 存储自己的列坐标
};

#endif // CELL_H

