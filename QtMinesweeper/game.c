#include "game.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

// --- 内部(static)函数声明 ---
// 这些函数只能在此文件内部使用，对外部不可见。
static void initializeBoards(Game* game);
static void placeMines(Game* game, int firstRow, int firstCol);
static void calculateNumbers(Game* game);
static void revealCell(Game* game, int row, int col);
static void toggleFlag(Game* game, int row, int col);
static int isValid(Game* game, int row, int col);

// --- 公共API函数实现 ---

//返回一个Game* 类型的值，也就是一个指向 Game 结构体的指针
Game* game_create(int rows, int cols, int mines) {
    Game* game = (Game*)malloc(sizeof(Game)); //计算Game所需内存，并预留空间，把内存地址存储在game指针变量中
    if (!game) return NULL; //检查game是否为空

    game->rows = rows;
    game->cols = cols;
    game->mines = mines;
    game->flagsPlaced = 0; //已插旗数
    game->isFirstMove = 1; //是否为第一步
    game->gameOver = 0; //游戏是否结束

    //为二维数组的第一维（行指针数组）申请内存
    game->mineBoard = (int**)malloc(rows * sizeof(int*));
    game->displayBoard = (char**)malloc(rows * sizeof(char*));

    //sizeof(int*) 计算出一个“指向整数的指针”需要多大空间
    //rows * sizeof(int*) 计算出存储 rows 个这样的指针总共需要多大空间
    //malloc(...) 申请这块空间
    //game->mineBoard = ... 将这块空间的地址存入 mineBoard
    //现在，mineBoard 指向一个可以存放 rows 个 int* 指针的数组。displayBoard 同理

    //对内存申请检查
    if (!game->mineBoard || !game->displayBoard) {
        // 简单的错误处理
        free(game->mineBoard);
        free(game->displayBoard);
        free(game);
        return NULL;
    }


    //为二维数组的第二维（每一行具体的列数据）申请内存。这是创建动态二维数组的第二步
    for (int i = 0; i < rows; ++i) {
        game->mineBoard[i] = (int*)malloc(cols * sizeof(int));
        game->displayBoard[i] = (char*)malloc(cols * sizeof(char));
        if (!game->mineBoard[i] || !game->displayBoard[i]) {
            // 复杂的错误处理，需要释放已分配的内存
            for (int j = 0; j < i; ++j) {
                free(game->mineBoard[j]);
                free(game->displayBoard[j]);
            }
            free(game->mineBoard);
            free(game->displayBoard);
            free(game);
            return NULL;
        }
    }
 
    //调用一个内部辅助函数，将我们刚刚申请的所有棋盘内存空间，都填上默认值
    initializeBoards(game);
    //初始化随机数生成器，保证了每次游戏的雷区分布都不同
    srand(time(NULL));
    return game;
}


//在游戏结束或重置时，安全、彻底地释放一局游戏占用的所有动态内存
void game_destroy(Game* game) {
    if (!game) return;
    for (int i = 0; i < game->rows; ++i) {
        //先释放二维数组的每一行
        free(game->mineBoard[i]);
        free(game->displayBoard[i]);
    }
    //然后释放存储行指针的数组
    free(game->mineBoard);
    free(game->displayBoard);
    // 最后释放Game结构体本身
    free(game);
}

void game_reveal_cell(Game* game, int row, int col) {
    if (game->gameOver || !isValid(game, row, col) || game->displayBoard[row][col] == '@') {
        return;  //确保游戏结束后用户的任何点击都无效
    }

    //如果是第一次点击，调用函数完成棋盘的生成
    if (game->isFirstMove) {
        game->startTime = time(NULL);
        placeMines(game, row, col);
        calculateNumbers(game);
        game->isFirstMove = 0;
    }
    revealCell(game, row, col);
}

void game_toggle_flag(Game* game, int row, int col) {
    if (game->gameOver || !isValid(game, row, col)) return;
    toggleFlag(game, row, col);
}

void game_chord_action(Game* game, int row, int col) {
    if (game->gameOver || !isValid(game, row, col)) return;

    char cell_state = game->displayBoard[row][col];
    if (cell_state < '1' || cell_state > '8') return;

    int number = game->mineBoard[row][col];
    int flag_count = 0;

    for (int dr = -1; dr <= 1; ++dr) {
        for (int dc = -1; dc <= 1; ++dc) {
            if (isValid(game, row + dr, col + dc) && game->displayBoard[row + dr][col + dc] == '@') {
                flag_count++;
            }
        }
    }

    if (flag_count == number) {
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) continue;
                int new_r = row + dr;
                int new_c = col + dc;
                if (isValid(game, new_r, new_c) && game->displayBoard[new_r][new_c] == '#') {
                    revealCell(game, new_r, new_c);
                }
            }
        }
    }
}

//统计所有不是“未揭开”也不是“旗帜”的格子数量。如果这个数量
//正好等于总格子数减去地雷总数，就意味着所有安全的格子都已经被揭开
int checkWinCondition(Game* game) {
    int revealed_count = 0;
    for (int r = 0; r < game->rows; ++r) {
        for (int c = 0; c < game->cols; ++c) {
            if (game->displayBoard[r][c] != '#' && game->displayBoard[r][c] != '@') {
                revealed_count++;
            }
        }
    }
    return revealed_count == (game->rows * game->cols - game->mines);
}

// --- AI 逻辑实现 ---
//决策优先级
//1.寻找100%安全的格子点击
//2.如果找不到，寻找100%是地雷的格子插旗
//3.如果还找不到，从所有未揭开的格子中随机猜一个

AI_Move game_ai_play_one_step(Game* game) {
    AI_Move move;
    move.row = -1;
    move.col = -1;

    // 首先，检查游戏是否已经结束
    if (game->gameOver) {
        move.type = AI_MOVE_LOSE;
        return move;
    }
    if (checkWinCondition(game)) {
        move.type = AI_MOVE_WIN;
        return move;
    }

    // --- 策略1：寻找确定安全的格子来点击 (类似双击展开) ---
    // 逻辑：如果一个数字N周围已经有N个旗帜，那么它旁边的其他未揭开格子一定是安全的。
    for (int r = 0; r < game->rows; ++r) {
        for (int c = 0; c < game->cols; ++c) {
            char state = game->displayBoard[r][c];
            if (state >= '1' && state <= '8') {
                int number = state - '0';
                int flag_count = 0;
                int hidden_count = 0;
                // 统计周围的旗帜和未揭开格子
                for (int dr = -1; dr <= 1; ++dr) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        if (!isValid(game, r + dr, c + dc) || (dr == 0 && dc == 0)) continue;
                        if (game->displayBoard[r + dr][c + dc] == '@') flag_count++;
                        if (game->displayBoard[r + dr][c + dc] == '#') hidden_count++;
                    }
                }
                // 如果旗帜数等于数字，且还有未揭开的格子，就点击它们
                if (flag_count == number && hidden_count > 0) {
                    for (int dr = -1; dr <= 1; ++dr) {
                        for (int dc = -1; dc <= 1; ++dc) {
                            if (isValid(game, r + dr, c + dc) && game->displayBoard[r + dr][c + dc] == '#') {
                                move.type = AI_MOVE_SAFE_CLICK;
                                move.row = r + dr;
                                move.col = c + dc;
                                return move; // 找到一个就立刻返回
                            }
                        }
                    }
                }
            }
        }
    }

    // --- 策略2：寻找确定是地雷的格子来插旗 ---
    // 逻辑：如果一个数字N周围的未揭开格子+旗帜数恰好等于N，那么所有未揭开的格子一定是地雷。
    for (int r = 0; r < game->rows; ++r) {
        for (int c = 0; c < game->cols; ++c) {
            char state = game->displayBoard[r][c];
            if (state >= '1' && state <= '8') {
                int number = state - '0';
                int flag_count = 0;
                int hidden_count = 0;
                for (int dr = -1; dr <= 1; ++dr) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        if (!isValid(game, r + dr, c + dc) || (dr == 0 && dc == 0)) continue;
                        if (game->displayBoard[r + dr][c + dc] == '@') flag_count++;
                        if (game->displayBoard[r + dr][c + dc] == '#') hidden_count++;
                    }
                }
                // 如果未揭开格子数大于0，且总数等于数字，就插旗
                if (hidden_count > 0 && number == flag_count + hidden_count) {
                     for (int dr = -1; dr <= 1; ++dr) {
                        for (int dc = -1; dc <= 1; ++dc) {
                             if (isValid(game, r + dr, c + dc) && game->displayBoard[r + dr][c + dc] == '#') {
                                move.type = AI_MOVE_FLAG;
                                move.row = r + dr;
                                move.col = c + dc;
                                return move; // 找到一个就立刻返回
                            }
                        }
                    }
                }
            }
        }
    }

    // --- 策略3：如果找不到确定性操作，就进行猜测 ---
    int hidden_cells[game->rows * game->cols][2];
    int hidden_count = 0;
    for (int r = 0; r < game->rows; ++r) {
        for (int c = 0; c < game->cols; ++c) {
            if (game->displayBoard[r][c] == '#') {
                hidden_cells[hidden_count][0] = r;
                hidden_cells[hidden_count][1] = c;
                hidden_count++;
            }
        }
    }

    if (hidden_count > 0) {
        int random_index = rand() % hidden_count;
        move.type = AI_MOVE_GUESS;
        move.row = hidden_cells[random_index][0];
        move.col = hidden_cells[random_index][1];
        return move;
    }

    // 如果没有任何未揭开的格子，说明AI卡住了（通常是胜利）
    move.type = AI_MOVE_STUCK;
    return move;
}


// --- 内部(static)函数实现 ---

//用默认值填充申请的内存空间
static void initializeBoards(Game* game) {
    for (int r = 0; r < game->rows; ++r) {
        for (int c = 0; c < game->cols; ++c) {
            game->mineBoard[r][c] = 0;
            game->displayBoard[r][c] = '#';
        }
    }
}

static void placeMines(Game* game, int firstRow, int firstCol) {
    int mines_placed = 0;
    while (mines_placed < game->mines) {
        //获得随机的行列坐标
        int r = rand() % game->rows;
        int c = rand() % game->cols;

        //保证了地雷不会出现在玩家首次点击的安全区内
        int isSafeZone = (abs(r - firstRow) <= 1 && abs(c - firstCol) <= 1);
        if (game->mineBoard[r][c] != -1 && !isSafeZone) {
            game->mineBoard[r][c] = -1;
            mines_placed++;
        }
    }
}

//对每个格子遍历，检查周围8个格子并累加地雷数量
static void calculateNumbers(Game* game) {
    for (int r = 0; r < game->rows; ++r) {
        for (int c = 0; c < game->cols; ++c) {
            if (game->mineBoard[r][c] == -1) continue;
            int mine_count = 0;
            for (int dr = -1; dr <= 1; ++dr) {
                for (int dc = -1; dc <= 1; ++dc) {
                    if (dr == 0 && dc == 0) continue;
                    if (isValid(game, r + dr, c + dc) && game->mineBoard[r + dr][c + dc] == -1) {
                        mine_count++;
                    }
                }
            }
            game->mineBoard[r][c] = mine_count;
        }
    }
}

//
static void revealCell(Game* game, int row, int col) {
    if (!isValid(game, row, col) || (game->displayBoard[row][col] != '#')) {
        return;
    }
    if (game->mineBoard[row][col] == -1) {
        game->gameOver = 1;
        // 游戏结束时，揭示所有地雷和错误标记
        for(int r = 0; r < game->rows; ++r) {
            for(int c = 0; c < game->cols; ++c) {
                if(game->mineBoard[r][c] == -1) {
                    if(game->displayBoard[r][c] != '@')
                        game->displayBoard[r][c] = '*';
                } else if (game->displayBoard[r][c] == '@') {
                     game->displayBoard[r][c] = 'X'; // 标记错误
                }
            }
        }
        game->displayBoard[row][col] = '!'; // 被引爆的地雷
        return;
    }

    game->displayBoard[row][col] = (game->mineBoard[row][col] == 0) ? ' ' : (game->mineBoard[row][col] + '0');

    if (game->mineBoard[row][col] == 0) {
        for (int dr = -1; dr <= 1; ++dr) {
            for (int dc = -1; dc <= 1; ++dc) {
                if (dr == 0 && dc == 0) continue;
                revealCell(game, row + dr, col + dc);
            }
        }
    }
}

//
static void toggleFlag(Game* game, int row, int col) {
    char state = game->displayBoard[row][col];
    if (state == '#') {
        game->displayBoard[row][col] = '@';
        game->flagsPlaced++;
    } else if (state == '@') {
        game->displayBoard[row][col] = '#';
        game->flagsPlaced--;
    }
}

//防止程序因访问数组越界而崩溃
static int isValid(Game* game, int row, int col) {
    return row >= 0 && row < game->rows && col >= 0 && col < game->cols;
}

