#ifndef SCOREBOARD_H
#define SCOREBOARD_H

#include "config.h"

#ifdef __cplusplus
extern "C" {
#endif

// 定义玩家得分的结构体
typedef struct {
    char name[20]; // 玩家名字
    int time;      // 完成时间（秒）
} PlayerScore;

// --- 函数声明 ---

/**
 * @brief 添加一个新的得分记录到排行榜
 * @param name 玩家的名字
 * @param time 玩家的时间
 */
void scoreboard_add_score(const char* name, int time);

/**
 * @brief 在控制台显示排行榜 (保留给C语言版本使用)
 */
void scoreboard_display();

/**
 * @brief 【新增的公共函数】从文件加载得分到数组中
 * @param scores 用于存储得分的PlayerScore数组
 * @param max_scores 数组的最大容量
 * @return 实际加载的得分数量
 */
int scoreboard_load(PlayerScore scores[], int max_scores); // <--- 在这里添加这一行

#ifdef __cplusplus
}
#endif

#endif // SCOREBOARD_H

