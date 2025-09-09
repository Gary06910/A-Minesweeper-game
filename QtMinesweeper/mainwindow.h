#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QLCDNumber>
#include <QTimer>
#include <vector>
#include <QSoundEffect> // 【修复】这是正确的QSoundEffect头文件
#include <QAction>
#include <QPushButton> // 确保QPushButton被包含

#include "cell.h"

// 包含C语言后端头文件
extern "C" {
#include "game.h"
#include "scoreboard.h"
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onNewGameTriggered();
    void onEasyLevelTriggered();
    void onMediumLevelTriggered();
    void onHardLevelTriggered();
    void onShowScoreboardTriggered();
    void onAboutTriggered();
    void resetGame();
    void onCellLeftClicked(int r, int c);
    void onCellRightClicked(int r, int c);
    void onCellDoubleClicked(int r, int c);
    void updateTimer();
    void onAiTriggered();
    void onAiTimerTimeout();

private:
    Game *m_game;
    int m_rows, m_cols, m_mines;
    QWidget *m_centralWidget;
    QWidget *m_infoPanel;
    QGridLayout *m_mineFieldLayout;
    std::vector<std::vector<Cell*>> m_cells;
    QLCDNumber *m_mineCountDisplay;
    QLCDNumber *m_timerDisplay;
    QPushButton *m_resetButton;
    QTimer *m_timer;
    int m_seconds;
    QSoundEffect *m_clickSound;
    QSoundEffect *m_flagSound;
    QSoundEffect *m_explosionSound;
    QSoundEffect *m_winSound;
    bool m_isGameOverSoundPlayed;
    QAction *m_aiAction;
    QTimer *m_aiTimer;
    bool m_isAiActive;
    void setupUI();
    void setupMenu();
    void setupInfoPanel();
    void setupSounds();
    void createMineField();
    void cleanupMineField();
    void updateUI();
    void connectCellSignals();
};
#endif // MAINWINDOW_H

