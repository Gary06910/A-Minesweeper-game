#ifndef GAME_H
#define GAME_H

//C++和C语言之间的“翻译官”，实现兼容
#ifdef __cplusplus
extern "C" {
#endif

#include <time.h>

// ... (Game 结构体保持不变) ...
//封装了关于一局游戏所有的状态信息：棋盘有多大 (rows, cols)、有多少雷 (mines)、
//玩家插了多少旗 (flagsPlaced)、游戏是否结束 (gameOver)，以及最重要的两个棋盘
//数据 (mineBoard 和 displayBoard)
typedef struct {
    int rows; int cols; int mines; int flagsPlaced;
    int isFirstMove; int gameOver; time_t startTime;
    int** mineBoard; char** displayBoard;
} Game;

//定义AI操作的类型
typedef enum {
    AI_MOVE_SAFE_CLICK, // 确定安全的点击
    AI_MOVE_FLAG,       // 确定是地雷，插旗
    AI_MOVE_GUESS,      // 无法确定，进行随机猜测
    AI_MOVE_STUCK,      // 卡关，且无处可猜（通常意味着胜利）
    AI_MOVE_WIN,        // AI检测到已胜利
    AI_MOVE_LOSE       // AI检测到已失败
} AI_Move_Type;

//封装AI操作结果的结构体
typedef struct {
    AI_Move_Type type; // 操作类型
    int row;           // 操作目标的行
    int col;           // 操作目标的列
} AI_Move;


// --- 公共API函数 ---
//定义了前端可以对后端发出的所有合法命令，即API
Game* game_create(int rows, int cols, int mines);
void game_destroy(Game* game);
void game_reveal_cell(Game* game, int row, int col);
void game_toggle_flag(Game* game, int row, int col);
void game_chord_action(Game* game, int row, int col);
int checkWinCondition(Game* game);

//AI执行一步操作的函数
AI_Move game_ai_play_one_step(Game* game);


#ifdef __cplusplus
}
#endif

#endif // GAME_H

