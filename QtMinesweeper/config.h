#ifndef CONFIG_H
#define CONFIG_H

// --- 排行榜配置 ---
#define MAX_SCORES 5                       // 排行榜记录的最大数量
#define SCOREBOARD_FILE "scoreboard.dat"   // 【新增】排行榜数据文件的名称

// --- 游戏难度配置 ---

// 初级 (Easy)
#define EASY_ROWS 9
#define EASY_COLS 9
#define EASY_MINES 10

// 中级 (Medium)
#define MEDIUM_ROWS 16
#define MEDIUM_COLS 16
#define MEDIUM_MINES 40

// 高级 (Hard)
#define HARD_ROWS 16
#define HARD_COLS 30
#define HARD_MINES 99


#endif // CONFIG_H

