#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QDateTime>

// 星期枚举（游戏只涉及周六与周日）
enum class DayOfWeek {
    Saturday,
    Sunday
};

// 时段枚举
enum class TimeSlot {
    Morning,    // 早晨
    Forenoon,   // 上午
    Noon,       // 中午
    Afternoon,  // 下午
    Evening,    // 傍晚
    Night       // 夜晚
};

// 道具枚举
enum class Item {
    None = 0,
    StudentCard,          // 学子卡
    AudioRecording,       // 一段钢琴录音
    WishCard,             // 心愿卡
    AuntHandkerchief,     // 阿姨的手帕
    OldAssignment,        // 陈旧的作业纸
    SciFiBookExcerpt,     // 科幻小说书摘
    PearCandy,            // 一小袋梨膏糖
    TornNote,             // 一张撕下来的笔记
    HalfChalk,            // 半截粉笔
    OneYuanCoin,          // 一枚一元硬币
    MapleBookmark,        // 一张遗留的枫叶书签
    OldDiaryPage,         // 一页泛黄的日记
    HandDrawnMap,         // 一张手绘津南校区地图
    InstantCoffee,        // 一包速溶咖啡
    LargeSycamoreLeaf,    // 一片很大的梧桐叶
    DarkBlueButton,       // 一枚深蓝色的纽扣
    KeychainWithBell,     // 一个钥匙扣（挂着小铃铛）
    SportsDrink,          // 一瓶运动饮料（未拆封）
    Pinecone,             // 一颗松果
    DoodlePaper,          // 一张被涂鸦过的草稿纸
    BrokenEarphone,       // 一截断掉的耳机线
    SmoothPebble          // 一块圆润的鹅卵石
};

// 世界裂缝枚举
enum class WorldCrack {
    None = 0,
    CorridorShadow,       // 公教楼连廊的人影
    BlankPaper,           // 一张空白的纸
    TestRecordOnDesk      // 阅览桌上的测试记录
};

// 任务枚举
enum class Quest {
    Handkerchief = 0,     // 食堂阿姨的手帕
    PianoWish,            // 琴房的心愿卡
    CatSunset             // 陪猫猫看日落
};

// 结局枚举
enum class Ending {
    None = 0,
    BondSpark,            // 羁绊·星火 (T1)
    TodayUntie,           // 今日·解铃 (T2)
    CocoonButterfly_Butterfly, // 茧与蝶·成蝶
    CocoonButterfly_Resonance, // 茧与蝶·共振
    BoundlessTomorrow,    // 困缚·明日何其多 (B1)
    DrowningSleep         // 沉溺·长眠 (B2)
};

class GameState : public QObject
{
    Q_OBJECT

public:
    explicit GameState(QObject *parent = nullptr);

    // 基础变量读取与设置
    int energy() const;           // E: 体力值
    int sanity() const;           // S: 精神稳定值
    int knowledge() const;        // K: 知识
    int physical() const;         // Q: 身体素质
    int loopCount() const;        // L: 循环次数
    int sinking() const;          // M: 沉沦值
    int homeworkCount() const;    // N: 做作业次数

    void setEnergy(int val);
    void setSanity(int val);
    void setKnowledge(int val);
    void setPhysical(int val);
    void setLoopCount(int val);
    void setSinking(int val);
    void setHomeworkCount(int val);

    // 时间系统
    DayOfWeek currentDay() const;
    TimeSlot currentSlot() const;
    void setDay(DayOfWeek day);
    void setSlot(TimeSlot slot);
    bool isWeekendNight() const;  // 是否周六或周日夜晚
    void advanceTime();           // 推进一个时段，若从夜晚跨天则处理循环

    // 结局解锁
    Ending unlockedEnding() const;  // 当前选中的结局（if any）
    void setUnlockedEnding(Ending e);
    bool isEndingAvailable(Ending e) const;
    void permanentlyUnlockEnding(Ending e); // 永久解锁入口
    bool isEndingUnlocked(Ending e) const;

    // 世界裂缝
    bool hasCrack(WorldCrack crack) const;
    void addCrack(WorldCrack crack);
    QList<WorldCrack> cracks() const;

    // 任务完成状态
    bool isQuestCompleted(Quest quest) const;
    void completeQuest(Quest quest);

    // 背包（道具）
    bool hasItem(Item item) const;
    void addItem(Item item);
    void removeItem(Item item);
    QList<Item> inventoryItems() const;

    // 特殊标记
    bool hasDoneInitialLoop() const;   // L>=1
    bool hasTalkedToCatScene1() const; // 公教楼橘猫剧情1已完成
    bool hasTalkedToCatScene2() const;
    bool hasTalkedToCatScene3() const;
    void setCatScene1Done(bool done);
    void setCatScene2Done(bool done);
    void setCatScene3Done(bool done);

    // 其他一次性触发标记 (为了剧情分支)
    bool hasTriggeredWindowPaper() const;      // 公教楼 aa
    bool hasTriggeredWindowPaperBB() const;    // 公教楼 bb
    bool hasTriggeredLibraryBookAA() const;
    bool hasTriggeredLibraryBookBB() const;
    bool hasTriggeredAuntSaw() const;          // 食堂阿姨初次见面
    bool hasTriggeredAuntDialectA() const;     // 剧情A
    bool hasTriggeredAuntDialectB() const;     // 剧情B
    bool hasTriggeredAuntDialectC() const;     // 剧情C
    bool hasTriggeredAuntLost() const;         // 剧情111 已触发
    bool hasFoundHandkerchiefInField() const;  // 操场已捡到手帕
    void setWindowPaperTrigg(bool v);
    void setWindowPaperBBTrigg(bool v);
    void setLibraryBookAATrigg(bool v);
    void setLibraryBookBBTrigg(bool v);
    void setAuntSaw(bool v);
    void setAuntDialectA(bool v);
    void setAuntDialectB(bool v);
    void setAuntDialectC(bool v);
    void setAuntLost(bool v);
    void setFoundHandkerchiefInField(bool v);

    const QList<Ending>& unlockedEndings() const { return m_unlockedEndings; }

    // 存档 / 读档
    QJsonObject toJson() const;
    void fromJson(const QJsonObject &json);

signals:
    void energyChanged(int);
    void sanityChanged(int);
    void knowledgeChanged(int);
    void physicalChanged(int);
    void loopCountChanged(int);
    void sinkingChanged(int);
    void homeworkCountChanged(int);
    void dayChanged(DayOfWeek);
    void slotChanged(TimeSlot);
    void inventoryChanged();
    void endingUnlocked(Ending);

private:
    // 基础变量
    int m_energy = 100;
    int m_sanity = 80;
    int m_knowledge = 0;
    int m_physical = 0;
    int m_loopCount = 0;
    int m_sinking = 0;
    int m_homeworkCount = 0;

    // 时间
    DayOfWeek m_day = DayOfWeek::Saturday;
    TimeSlot m_slot = TimeSlot::Morning;

    // 结局永久解锁列表
    QList<Ending> m_unlockedEndings;

    // 世界裂缝
    QList<WorldCrack> m_cracks;

    // 已完成任务
    QList<Quest> m_completedQuests;

    // 背包物品
    QList<Item> m_inventory;

    // 各种一次性触发标记
    bool m_catScene1 = false;
    bool m_catScene2 = false;
    bool m_catScene3 = false;
    bool m_windowPaper = false;
    bool m_windowPaperBB = false;
    bool m_libraryBookAA = false;
    bool m_libraryBookBB = false;
    bool m_auntSaw = false;
    bool m_auntDialectA = false;
    bool m_auntDialectB = false;
    bool m_auntDialectC = false;
    bool m_auntLost = false;
    bool m_foundHandkerchief = false;
};

#endif // GAMESTATE_H