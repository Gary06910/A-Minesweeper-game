#include "mainwindow.h"
#include "config.h"
#include "scoreboard.h"
#include <QApplication>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QInputDialog>
#include <QDebug>
#include <QUrl>

// --- 【新增】为UI定义固定的样式常量，以适应暗夜模式 ---
const QString STYLE_UNREVEALED = "background-color: #E0E0E0; border: 2px solid #767676; border-style: outset;";
const QString STYLE_REVEALED = "background-color: #C0C0C0; border: 1px solid #767676; border-style: inset;";

// ... (构造函数、析构函数、setupUI等函数保持不变) ...
// ... (为简洁起见，这里省略了未改动的函数，请参考您已有的版本) ...


// 【关键修改】只修改这个函数，应用我们定义的样式
void MainWindow::updateUI() {
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            Cell *cell = m_cells[r][c];
            char state = m_game->displayBoard[r][c];

            // 默认重置为“未揭开”的样式
            cell->setEnabled(true);
            cell->setText("");
            cell->setIcon(QIcon());
            cell->setStyleSheet(STYLE_UNREVEALED);

            switch(state) {
            case '#':
                // 保持默认的未揭开样式
                break;
            case '@':
                cell->setIcon(QIcon(":/icons/flag.png"));
                cell->setStyleSheet(STYLE_UNREVEALED); // 插旗的格子也用凸起样式
                break;
            case ' ':
                cell->setEnabled(false); // 空白格子可以安全地禁用
                cell->setStyleSheet(STYLE_REVEALED); // 应用“凹陷”样式
                break;
            case '*':
                cell->setIcon(QIcon(":/icons/mine.png"));
                cell->setStyleSheet(STYLE_REVEALED);
                break;
            case '!':
                cell->setIcon(QIcon(":/icons/mine_triggered.png"));
                // 可以给引爆的雷一个特殊的红色背景
                cell->setStyleSheet("background-color: red; border: 1px solid #767676;");
                break;
            case 'X':
                cell->setIcon(QIcon(":/icons/mine.png"));
                cell->setStyleSheet(STYLE_REVEALED);
                break;
            default: // 处理数字 '1' 到 '8'
                cell->setText(QString(state));
                // 在“凹陷”样式的基础上，添加字体颜色
                QString numberStyle = STYLE_REVEALED;
                switch(state) {
                case '1': numberStyle += "color: blue;"; break;
                case '2': numberStyle += "color: green;"; break;
                case '3': numberStyle += "color: red;"; break;
                case '4': numberStyle += "color: darkblue;"; break;
                case '5': numberStyle += "color: brown;"; break;
                case '6': numberStyle += "color: cyan;"; break;
                case '7': numberStyle += "color: black;"; break;
                case '8': numberStyle += "color: grey;"; break;
                }
                cell->setStyleSheet(numberStyle);
                break;
            }
        }
    }
    m_mineCountDisplay->display(m_game->mines - m_game->flagsPlaced);

    // --- 游戏结束逻辑保持不变 ---
    if (m_game->gameOver && !m_isGameOverSoundPlayed) {
        m_isGameOverSoundPlayed = true;
        m_timer->stop();
        m_resetButton->setIcon(QIcon(":/icons/dead.png"));
        m_explosionSound->play();
        QMessageBox::information(this, "游戏结束", "你踩到雷了！");
    } else if (checkWinCondition(m_game) && !m_game->gameOver) {
        m_game->gameOver = 1;
        if (!m_isGameOverSoundPlayed) {
            m_isGameOverSoundPlayed = true;
            m_timer->stop();
            m_resetButton->setIcon(QIcon(":/icons/win.png"));
            m_winSound->play();
            QMessageBox::information(this, "胜利！", QString("恭喜你，你赢了！\n用时: %1 秒").arg(m_seconds));
            bool ok;
            QString name = QInputDialog::getText(this, "记录成绩", "请输入你的名字:", QLineEdit::Normal, "玩家", &ok);
            if (ok && !name.isEmpty()) {
                scoreboard_add_score(name.toStdString().c_str(), m_seconds);
            }
        }
    }
}


// --- 所有其他函数都保持不变 ---
// --- 为方便您，下面是完整的、未改动的其余部分 ---

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_game(nullptr),
    m_rows(EASY_ROWS),
    m_cols(EASY_COLS),
    m_mines(EASY_MINES),
    m_isAiActive(false)
{
    setupUI();
    setupSounds();
    m_aiTimer = new QTimer(this);
    connect(m_aiTimer, &QTimer::timeout, this, &MainWindow::onAiTimerTimeout);
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::updateTimer);
    resetGame();
}

MainWindow::~MainWindow()
{
    if (m_game) {
        game_destroy(m_game);
    }
}

void MainWindow::setupUI() {
    setWindowTitle("扫雷");
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    QVBoxLayout *mainLayout = new QVBoxLayout(m_centralWidget);
    setupMenu();
    setupInfoPanel();
    m_mineFieldLayout = new QGridLayout();
    m_mineFieldLayout->setSpacing(1);
    mainLayout->addWidget(m_infoPanel);
    mainLayout->addLayout(m_mineFieldLayout);
}

void MainWindow::setupMenu() {
    QMenu *gameMenu = menuBar()->addMenu("游戏(&G)");
    QAction *newGameAction = new QAction("新游戏 (&N)", this);
    connect(newGameAction, &QAction::triggered, this, &MainWindow::onNewGameTriggered);
    gameMenu->addAction(newGameAction);
    gameMenu->addSeparator();
    QMenu *levelMenu = gameMenu->addMenu("难度 (&D)");
    QAction *easyAction = new QAction("初级 (&E)", this);
    connect(easyAction, &QAction::triggered, this, &MainWindow::onEasyLevelTriggered);
    levelMenu->addAction(easyAction);
    QAction *mediumAction = new QAction("中级 (&M)", this);
    connect(mediumAction, &QAction::triggered, this, &MainWindow::onMediumLevelTriggered);
    levelMenu->addAction(mediumAction);
    QAction *hardAction = new QAction("高级 (&H)", this);
    connect(hardAction, &QAction::triggered, this, &MainWindow::onHardLevelTriggered);
    levelMenu->addAction(hardAction);
    gameMenu->addSeparator();
    m_aiAction = new QAction("AI玩家开始运行", this);
    m_aiAction->setCheckable(true);
    connect(m_aiAction, &QAction::triggered, this, &MainWindow::onAiTriggered);
    gameMenu->addAction(m_aiAction);
    gameMenu->addSeparator();
    QAction *scoreboardAction = new QAction("排行榜 (&S)", this);
    connect(scoreboardAction, &QAction::triggered, this, &MainWindow::onShowScoreboardTriggered);
    gameMenu->addAction(scoreboardAction);
    gameMenu->addSeparator();
    QAction *exitAction = new QAction("退出 (&X)", this);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
    gameMenu->addAction(exitAction);
    QMenu *helpMenu = menuBar()->addMenu("帮助(&H)");
    QAction *aboutAction = new QAction("关于 (&A)...", this);
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAboutTriggered);
    helpMenu->addAction(aboutAction);
}

void MainWindow::setupInfoPanel() {
    m_infoPanel = new QWidget(this);
    QHBoxLayout *infoLayout = new QHBoxLayout(m_infoPanel);
    m_mineCountDisplay = new QLCDNumber(3);
    m_mineCountDisplay->setSegmentStyle(QLCDNumber::Flat);
    m_resetButton = new QPushButton(this);
    m_resetButton->setFixedSize(40, 40);
    m_resetButton->setIconSize(QSize(35,35));
    connect(m_resetButton, &QPushButton::clicked, this, &MainWindow::resetGame);
    m_timerDisplay = new QLCDNumber(3);
    m_timerDisplay->setSegmentStyle(QLCDNumber::Flat);
    infoLayout->addWidget(m_mineCountDisplay);
    infoLayout->addStretch();
    infoLayout->addWidget(m_resetButton);
    infoLayout->addStretch();
    infoLayout->addWidget(m_timerDisplay);
}

void MainWindow::setupSounds()
{
    m_clickSound = new QSoundEffect(this);
    m_clickSound->setSource(QUrl("qrc:/sounds/click.wav"));
    m_clickSound->setVolume(0.8f);
    m_flagSound = new QSoundEffect(this);
    m_flagSound->setSource(QUrl("qrc:/sounds/flag.wav"));
    m_flagSound->setVolume(1.0f);
    m_explosionSound = new QSoundEffect(this);
    m_explosionSound->setSource(QUrl("qrc:/sounds/explosion.wav"));
    m_explosionSound->setVolume(1.0f);
    m_winSound = new QSoundEffect(this);
    m_winSound->setSource(QUrl("qrc:/sounds/win.wav"));
    m_winSound->setVolume(1.0f);
}

void MainWindow::resetGame() {
    cleanupMineField();
    if (m_game) {
        game_destroy(m_game);
    }
    m_game = game_create(m_rows, m_cols, m_mines);
    m_timer->stop();
    m_seconds = 0;
    m_timerDisplay->display(0);
    m_mineCountDisplay->display(m_mines);
    m_resetButton->setIcon(QIcon(":/icons/smile.png"));
    m_isGameOverSoundPlayed = false;
    if(m_aiTimer->isActive()) {
        m_aiTimer->stop();
    }
    m_isAiActive = false;
    if(m_aiAction) {
        m_aiAction->setChecked(false);
        m_aiAction->setText("AI玩家开始运行");
        m_aiAction->setEnabled(true);
    }
    createMineField();
    connectCellSignals();
    this->adjustSize();
}

void MainWindow::createMineField() {
    m_cells.resize(m_rows);
    for (int r = 0; r < m_rows; ++r) {
        m_cells[r].resize(m_cols);
        for (int c = 0; c < m_cols; ++c) {
            Cell *cell = new Cell(r, c, this);
            cell->setFixedSize(30, 30);
            cell->setFont(QFont("Arial", 12, QFont::Bold));
            cell->setContextMenuPolicy(Qt::CustomContextMenu);
            m_mineFieldLayout->addWidget(cell, r, c);
            m_cells[r][c] = cell;
        }
    }
}

void MainWindow::connectCellSignals() {
    for (int r = 0; r < m_rows; ++r) {
        for (int c = 0; c < m_cols; ++c) {
            Cell *cell = m_cells[r][c];
            connect(cell, &QPushButton::clicked, this, [this, r, c](){ onCellLeftClicked(r, c); });
            connect(cell, &QPushButton::customContextMenuRequested, this, [this, r, c](){ onCellRightClicked(r, c); });
            connect(cell, &Cell::doubleClicked, this, &MainWindow::onCellDoubleClicked);
        }
    }
}

void MainWindow::cleanupMineField() {
    for (int r = 0; r < static_cast<int>(m_cells.size()); ++r) {
        for (int c = 0; c < static_cast<int>(m_cells[r].size()); ++c) {
            delete m_cells[r][c];
        }
    }
    m_cells.clear();
}

void MainWindow::onNewGameTriggered() { resetGame(); }
void MainWindow::onEasyLevelTriggered() { m_rows = EASY_ROWS; m_cols = EASY_COLS; m_mines = EASY_MINES; resetGame(); }
void MainWindow::onMediumLevelTriggered() { m_rows = MEDIUM_ROWS; m_cols = MEDIUM_COLS; m_mines = MEDIUM_MINES; resetGame(); }
void MainWindow::onHardLevelTriggered() { m_rows = HARD_ROWS; m_cols = HARD_COLS; m_mines = HARD_MINES; resetGame(); }
void MainWindow::onAboutTriggered() { QMessageBox::about(this, "关于", "这是一个扫雷游戏"); }

void MainWindow::onShowScoreboardTriggered(){
    PlayerScore scores[MAX_SCORES];
    int count = scoreboard_load(scores, MAX_SCORES);
    QString boardText = "排名\t时间(秒)\t玩家\n--------------------------\n";
    if (count == 0) {
        boardText += "排行榜是空的！";
    } else {
        for (int i = 0; i < count; ++i) {
            boardText += QString("%1\t%2\t%3\n").arg(i + 1).arg(scores[i].time).arg(scores[i].name);
        }
    }
    QMessageBox::information(this, "排行榜", boardText);
}

void MainWindow::onCellLeftClicked(int r, int c) {
    if (m_game->gameOver) return;
    if (m_isAiActive) return;
    if (m_game->isFirstMove) {
        m_timer->start(1000);
    }
    m_aiAction->setEnabled(false);
    char previousState = m_game->displayBoard[r][c];
    game_reveal_cell(m_game, r, c);
    if (previousState == '#' && m_game->displayBoard[r][c] != '#') {
        m_clickSound->play();
    }
    updateUI();
}

void MainWindow::onCellRightClicked(int r, int c) {
    if (m_game->gameOver) return;
    if (m_isAiActive) return;
    m_aiAction->setEnabled(false);
    game_toggle_flag(m_game, r, c);
    m_flagSound->play();
    updateUI();
}

void MainWindow::onCellDoubleClicked(int r, int c) {
    if (m_game->gameOver) return;
    if (m_isAiActive) return;
    m_aiAction->setEnabled(false);
    m_clickSound->play();
    game_chord_action(m_game, r, c);
    updateUI();
}

void MainWindow::onAiTriggered() {
    if (m_isAiActive) {
        m_aiTimer->stop();
        m_isAiActive = false;
        m_aiAction->setText("AI玩家开始运行");
    } else {
        if (m_game->isFirstMove) {
            int r = m_game->rows / 2;
            int c = m_game->cols / 2;
            if (m_game->isFirstMove) {
                m_timer->start(1000);
            }
            game_reveal_cell(m_game, r, c);
            updateUI();
        }
        m_aiTimer->start(100);
        m_isAiActive = true;
        m_aiAction->setText("AI玩家停止运行");
    }
}

void MainWindow::onAiTimerTimeout() {
    if (m_game->gameOver || checkWinCondition(m_game)) {
        onAiTriggered();
        return;
    }
    AI_Move move = game_ai_play_one_step(m_game);
    switch(move.type) {
    case AI_MOVE_SAFE_CLICK:
    case AI_MOVE_GUESS:
        game_reveal_cell(m_game, move.row, move.col);
        m_clickSound->play();
        break;
    case AI_MOVE_FLAG:
        game_toggle_flag(m_game, move.row, move.col);
        m_flagSound->play();
        break;
    case AI_MOVE_STUCK:
    case AI_MOVE_WIN:
    case AI_MOVE_LOSE:
        onAiTriggered();
        break;
    }
    updateUI();
}

void MainWindow::updateTimer() {
    m_seconds++;
    m_timerDisplay->display(m_seconds);
}

