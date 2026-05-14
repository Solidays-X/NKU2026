#pragma once

#include <QString>
#include <QSet>
#include <QStringList>
#include <QHash>

enum class TimeSlot {
    Morning,   // 早晨
    Forenoon,  // 上午
    Noon,      // 中午
    Afternoon, // 下午
    Evening,   // 傍晚
    Night      // 夜晚
};

enum class Day {
    Saturday, // 周六
    Sunday    // 周日
};

class GameState {
public:
    GameState();

    // ===== Core variables =====
    int E = 100;   // 体力
    int S = 80;    // 精神稳定
    int K = 0;     // 知识
    int Q = 0;     // 身体素质
    int L = 0;     // 循环次数
    int M = 0;     // 沉沦值
    int N = 0;     // 做作业次数 (跨循环累加)

    // ===== Time =====
    Day day = Day::Saturday;
    TimeSlot slot = TimeSlot::Morning;

    // ===== Inventory / items =====
    QSet<QString> items;                  // current loop or persistent items
    bool carryingStudentCard = false;     // 是否随身携带学子卡 (跨循环但需要每次进入宿舍重新选择)

    // ===== Tasks =====
    QSet<QString> completedTasks;         // 已完成任务 (跨循环)
    QSet<QString> tasksCompletedThisLoop; // 本循环完成的任务

    // ===== World cracks =====
    QSet<QString> worldCracks;            // 收集到的世界裂缝 (跨循环)

    // ===== Endings unlocked (entries) =====
    QSet<QString> unlockedEndings;        // 解锁的结局入口 (仅 T1/T2)
    QSet<QString> seenEndings;            // 已经游玩到过的全部结局 (含 B1/B2/Hidden, 永久, 图鉴展示用)

    // ===== Persistent flags (across loops) =====
    bool firstHaitangBoxOpened = false;   // 是否曾经打开过海棠树下铁盒
    bool haitangBoxTakenCard = false;     // 是否带走过心愿卡(获得过)
    bool everCompletedHandkerchief = false;// 是否曾经完成过手帕任务
    bool everPublishedRecording = false;  // 是否曾经发帖
    bool everReceivedReply = false;       // 是否收到过私信

    // ===== Per-loop flags (reset each loop) =====
    bool tryProbeRoommateDone = false;    // 试探循环本循环已用
    bool teachingAa = false;              // 公教楼剧情aa触发过
    bool teachingBbDone = false;          // 公教楼剧情bb是否完成过(永久)
    bool libraryAa = false;               // 图书馆剧情aa触发过
    bool libraryBbDone = false;           // 图书馆剧情bb是否完成过(永久)
    bool libraryTestDone = false;         // 图书馆阅览桌测试记录是否触发过(永久)
    bool canteenA = false;                // 食堂剧情A触发过(本循环)
    bool canteenBDone = false;            // 剧情B是否完成过(永久)
    bool canteenCDone = false;            // 剧情C是否完成过(永久)
    bool canteenAuntUnlocked = false;     // 食堂阿姨选项激活 (跨循环持久, 一次触发永久解锁)
    bool canteenAuntStory111 = false;     // 本循环已触发剧情111
    bool canteenAuntTalkedThisLoop = false; // 本循环已经和阿姨说过话 (L<8 时第一次/非第一次区分)
    bool ladyHandkerchiefFoundThisLoop = false; // 本循环已在操场捡到手帕
    bool catTeachingSat = false;          // 公教楼橘猫坐过去(本循环)
    bool catLibrarySat = false;           // 图书馆橘猫坐过去(本循环)
    bool catPlaygroundSat = false;        // 操场橘猫坐过去(本循环)
    bool catMissedThisLoop = false;       // 本循环放弃过橘猫(后续不再遇见)
    bool haitangVisitedThisLoop = false;  // 本循环海棠树事件触发过
    bool corridorShadowDone = false;      // 公教楼连廊人影是否触发过(永久, 仅一次)
    bool southGateTriggeredThisLoop = false; // 本循环南门触发过
    bool catTeachingMet = false;          // 本循环公教楼橘猫遇到过(为防止重复触发)
    bool catLibraryMet = false;
    bool catPlaygroundMet = false;
    bool randomFoundTeaching = false;
    bool randomFoundLibrary = false;
    bool randomFoundPlayground = false;
    bool randomFoundWander = false;
    bool inDormTonight = true;            // 今晚是否在宿舍 (默认true)
    bool sleptOutsidePenalty = false;     // 在外过夜需要扣S(待结算)
    bool nightChoiceMadeThisLoop = false; // 本晚是否已经选过 "回/不回" 宿舍
    bool flippedWindowOut = false;        // 翻窗外出
    bool publishedRecording = false;      // 本循环已发帖
    bool gotReplyMessage = false;         // 本循环已收到私信
    bool blankPaperTaken = false;         // 本循环空白纸已查看(永久, 一次)

    // ===== Random encounters per loop =====
    bool canteenAuntApron = false;        // 本循环触发了食堂阿姨概率事件(L<8)
    bool storyAAA_done = false;           // 是否已经触发过剧情AAA

    // ===== 存档恢复点 =====
    // 每次进入一个 "主场景" (node1/campusHub/dormScene/canteenScene...) 时
    // SceneEngine 会把场景 id 写到这里. 读档时按这个 id 重新进入对应场景.
    // 由于场景内部的多步 narration 用 lambda 链接, 无法从中间恢复;
    // 读档恢复粒度 = 最近的主场景入口.
    QString sceneCheckpoint = QStringLiteral("intro");

    // ===== 消耗品定义 =====
    // 消耗品名 -> (dE, dS, dK, dQ, 使用文案)
    struct ConsumableEffect {
        int dE = 0; int dS = 0; int dK = 0; int dQ = 0;
        QString useText;
    };
    static const QHash<QString, ConsumableEffect>& consumableTable();
    static bool isConsumable(const QString& itemName);

    // ===== Methods =====
    void clampStats();
    void resetLoopFlags();                // L+1 时调用
    QString dayName() const;
    QString slotName() const;
    QString timeDisplay() const;
    void advanceTime();                   // 推进一个时段
    bool isDaytime() const;               // 是否白天
    bool sanityScrambled() const { return S <= 0; }

    // Helpers
    void addItem(const QString& name);
    bool hasItem(const QString& name) const;
    void removeItem(const QString& name);
    void completeTask(const QString& name);
    void unlockEnding(const QString& name);
};
