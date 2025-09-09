#include "scoreboard.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// --- 私有辅助函数声明 ---
// 注意：我们不再需要私有的load_scores了
static void save_scores(const PlayerScore scores[], int count);
static int compare_scores(const void* a, const void* b);

// --- 公共函数实现 ---

void scoreboard_add_score(const char* name, int time) {
    PlayerScore scores[MAX_SCORES + 1];
    int count = scoreboard_load(scores, MAX_SCORES); // 现在直接调用公共的加载函数

    // 添加新成绩
    strncpy(scores[count].name, name, 19);
    scores[count].name[19] = '\0';
    scores[count].time = time;
    count++;

    // 排序
    qsort(scores, count, sizeof(PlayerScore), compare_scores);

    // 如果超出最大数量，则只保留前MAX_SCORES个
    if (count > MAX_SCORES) {
        count = MAX_SCORES;
    }

    save_scores(scores, count);
}

void scoreboard_display() {
    PlayerScore scores[MAX_SCORES];
    int count = scoreboard_load(scores, MAX_SCORES); // 同样调用公共加载函数

    printf("\n--- 排行榜 ---\n");
    if (count == 0) {
        printf("暂无记录。\n");
    } else {
        printf("排名\t玩家\t\t时间(秒)\n");
        for (int i = 0; i < count; ++i) {
            printf("%d\t%-15s\t%d\n", i + 1, scores[i].name, scores[i].time);
        }
    }
    printf("--------------\n");
}


/**
 * @brief 【修改后的函数】从文件加载得分记录
 * 我们将原来私有的 'load_scores' 函数改为这个公共函数
 */
int scoreboard_load(PlayerScore scores[], int max_scores) {
    FILE* file = fopen(SCOREBOARD_FILE, "rb"); // "rb" for reading binary
    if (file == NULL) {
        return 0; // 文件不存在或无法打开，返回0个记录
    }

    // 从文件读取数据到数组
    int count = fread(scores, sizeof(PlayerScore), max_scores, file);
    fclose(file);
    return count;
}


// --- 私有辅助函数实现 ---

/**
 * @brief 将得分记录保存到文件
 */
static void save_scores(const PlayerScore scores[], int count) {
    FILE* file = fopen(SCOREBOARD_FILE, "wb"); // "wb" for writing binary
    if (file == NULL) {
        perror("无法打开排行榜文件进行写入");
        return;
    }
    fwrite(scores, sizeof(PlayerScore), count, file);
    fclose(file);
}

/**
 * @brief qsort所需的比较函数，按时间升序排列
 */
static int compare_scores(const void* a, const void* b) {
    const PlayerScore* scoreA = (const PlayerScore*)a;
    const PlayerScore* scoreB = (const PlayerScore*)b;
    return scoreA->time - scoreB->time;
}

