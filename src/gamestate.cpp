#include "gamestate.h"

GameState::GameState() {}

const QHash<QString, GameState::ConsumableEffect>& GameState::consumableTable() {
    static const QHash<QString, ConsumableEffect> tbl = {
        {QStringLiteral("一包速溶咖啡"),
         {15, 0, 0, 0, QStringLiteral("你撕开速溶咖啡，用温水冲了一杯，"
                                       "焦香的热气在指尖打转。<br/>"
                                       "<i>（体力值 +15）</i>")}},
        {QStringLiteral("一小袋梨膏糖"),
         {10, 0, 0, 0, QStringLiteral("你含了一颗梨膏糖。"
                                       "甜中微凉，喉咙舒服了不少。<br/>"
                                       "<i>（体力值 +10）</i>")}},
        {QStringLiteral("一瓶运动饮料"),
         {20, 0, 0, 0, QStringLiteral("你拧开瓶盖一饮而尽，"
                                       "电解质让脚步轻快了几分。<br/>"
                                       "<i>（体力值 +20）</i>")}},
        {QStringLiteral("一张撕下来的笔记"),
         {0, 0, 3, 0, QStringLiteral("你把那张笔记摊开认真读了一遍，"
                                      "里面是某位前人留下的解题思路。<br/>"
                                      "<i>（知识 +3）</i>")}},
        {QStringLiteral("一张遗留的枫叶书签"),
         {0, 8, 0, 0, QStringLiteral("你把书签夹回手边的书里。"
                                      "细密的脉络让心也安静了下来。<br/>"
                                      "<i>（精神稳定值 +8）</i>")}},
        {QStringLiteral("一块圆润的鹅卵石"),
         {0, 0, 0, 5, QStringLiteral("你握紧那块鹅卵石，掌心被磨得发热。"
                                      "感觉手腕都跟着结实起来。<br/>"
                                      "<i>（身体素质 +5）</i>")}},
    };
    return tbl;
}

bool GameState::isConsumable(const QString& name) {
    return consumableTable().contains(name);
}

void GameState::clampStats() {
    if (E > 100) E = 100;
    if (E < 0) E = 0;
    if (S > 100) S = 100;
    if (S < 0) S = 0;
    if (K < 0) K = 0;
    if (Q < 0) Q = 0;
    if (M < 0) M = 0;
    if (L < 0) L = 0;
    if (N < 0) N = 0;
}

void GameState::resetLoopFlags() {
    tryProbeRoommateDone = false;
    teachingAa = false;
    libraryAa = false;
    canteenA = false;
    // 注: canteenAuntUnlocked 改为持久(跨循环). 一旦某次循环偶然遇到食堂阿姨,
    // 之后所有循环都能直接选 [食堂阿姨] 选项, 不再依赖 30% 概率事件.
    canteenAuntStory111 = false;
    canteenAuntTalkedThisLoop = false;
    ladyHandkerchiefFoundThisLoop = false;
    // catTeachingSat / catLibrarySat / catPlaygroundSat 改为持久(跨循环累计),
    // 任务【陪猫猫看日落】需要三个地点都"坐过去"
    // catTeachingMet / catLibraryMet / catPlaygroundMet 保持单循环, 用于"一天只触发一次"
    catTeachingMet = false;
    catLibraryMet = false;
    catPlaygroundMet = false;
    catMissedThisLoop = false;
    haitangVisitedThisLoop = false;
    southGateTriggeredThisLoop = false;
    randomFoundTeaching = false;
    randomFoundLibrary = false;
    randomFoundPlayground = false;
    randomFoundWander = false;
    inDormTonight = true;
    sleptOutsidePenalty = false;
    nightChoiceMadeThisLoop = false;
    flippedWindowOut = false;
    publishedRecording = false;
    gotReplyMessage = false;
    canteenAuntApron = false;

    // 每次循环开始时 S 自动 -3
    S -= 3;
    clampStats();
}

QString GameState::dayName() const {
    return day == Day::Saturday ? QStringLiteral("周六") : QStringLiteral("周日");
}

QString GameState::slotName() const {
    switch (slot) {
        case TimeSlot::Morning:   return QStringLiteral("早晨");
        case TimeSlot::Forenoon:  return QStringLiteral("上午");
        case TimeSlot::Noon:      return QStringLiteral("中午");
        case TimeSlot::Afternoon: return QStringLiteral("下午");
        case TimeSlot::Evening:   return QStringLiteral("傍晚");
        case TimeSlot::Night:     return QStringLiteral("夜晚");
    }
    return QString();
}

QString GameState::timeDisplay() const {
    return dayName() + QStringLiteral(" ") + slotName();
}

void GameState::advanceTime() {
    switch (slot) {
        case TimeSlot::Morning:   slot = TimeSlot::Forenoon;  break;
        case TimeSlot::Forenoon:  slot = TimeSlot::Noon;      break;
        case TimeSlot::Noon:      slot = TimeSlot::Afternoon; break;
        case TimeSlot::Afternoon: slot = TimeSlot::Evening;   break;
        case TimeSlot::Evening:   slot = TimeSlot::Night;     break;
        case TimeSlot::Night:
            // 夜晚之后通常会进入睡觉处理（不在这里直接跳天），保持夜晚
            // 由专门的"次日早晨"逻辑处理
            slot = TimeSlot::Night;
            break;
    }
}

bool GameState::isDaytime() const {
    return slot != TimeSlot::Night;
}

void GameState::addItem(const QString& name) {
    items.insert(name);
}

bool GameState::hasItem(const QString& name) const {
    return items.contains(name);
}

void GameState::removeItem(const QString& name) {
    items.remove(name);
}

void GameState::completeTask(const QString& name) {
    completedTasks.insert(name);
    tasksCompletedThisLoop.insert(name);
}

void GameState::unlockEnding(const QString& name) {
    unlockedEndings.insert(name);
}
