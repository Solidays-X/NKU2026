#include "gamestate.h"
#include <QJsonObject>
#include <QJsonArray>
#include <algorithm>

GameState::GameState(QObject *parent)
    : QObject(parent)
{
    // 初始值已在成员初始化列表中设置，无需额外操作
}

// ---------- 基础变量访问器 ----------
int GameState::energy() const { return m_energy; }
int GameState::sanity() const { return m_sanity; }
int GameState::knowledge() const { return m_knowledge; }
int GameState::physical() const { return m_physical; }
int GameState::loopCount() const { return m_loopCount; }
int GameState::sinking() const { return m_sinking; }
int GameState::homeworkCount() const { return m_homeworkCount; }

void GameState::setEnergy(int val) {
    val = std::clamp(val, 0, 100);
    if (m_energy != val) {
        m_energy = val;
        emit energyChanged(m_energy);
        if (m_energy <= 0) {
            // 体力耗尽，触发结局 B2（在游戏流程中处理）
        }
    }
}

void GameState::setSanity(int val) {
    val = std::clamp(val, 0, 100);
    if (m_sanity != val) {
        m_sanity = val;
        emit sanityChanged(m_sanity);
    }
}

void GameState::setKnowledge(int val) {
    if (val < 0) val = 0; // 无上限
    if (m_knowledge != val) {
        m_knowledge = val;
        emit knowledgeChanged(m_knowledge);
    }
}

void GameState::setPhysical(int val) {
    if (val < 0) val = 0;
    if (m_physical != val) {
        m_physical = val;
        emit physicalChanged(m_physical);
    }
}

void GameState::setLoopCount(int val) {
    if (m_loopCount != val) {
        m_loopCount = val;
        emit loopCountChanged(m_loopCount);
        // 每循环开始时精神稳定值 -3 （由场景逻辑调用）
    }
}

void GameState::setSinking(int val) {
    val = std::clamp(val, 0, 7);
    if (m_sinking != val) {
        m_sinking = val;
        emit sinkingChanged(m_sinking);
        if (m_sinking > 3) {
            // 触发结局 B2（由流程处理）
        }
    }
}

void GameState::setHomeworkCount(int val) {
    if (m_homeworkCount != val) {
        m_homeworkCount = val;
        emit homeworkCountChanged(m_homeworkCount);
    }
}

// ---------- 时间系统 ----------
DayOfWeek GameState::currentDay() const { return m_day; }
TimeSlot GameState::currentSlot() const { return m_slot; }

void GameState::setDay(DayOfWeek day) {
    if (m_day != day) {
        m_day = day;
        emit dayChanged(m_day);
    }
}

void GameState::setSlot(TimeSlot slot) {
    if (m_slot != slot) {
        m_slot = slot;
        emit slotChanged(m_slot);
    }
}

bool GameState::isWeekendNight() const {
    return (m_day == DayOfWeek::Saturday || m_day == DayOfWeek::Sunday) && m_slot == TimeSlot::Night;
}

void GameState::advanceTime() {
    // 推进一个时段
    switch (m_slot) {
    case TimeSlot::Morning:   setSlot(TimeSlot::Forenoon); break;
    case TimeSlot::Forenoon:  setSlot(TimeSlot::Noon); break;
    case TimeSlot::Noon:      setSlot(TimeSlot::Afternoon); break;
    case TimeSlot::Afternoon: setSlot(TimeSlot::Evening); break;
    case TimeSlot::Evening:   setSlot(TimeSlot::Night); break;
    case TimeSlot::Night:
        // 夜晚结束后进入次日早晨
        if (m_day == DayOfWeek::Saturday) {
            setDay(DayOfWeek::Sunday);
            setSlot(TimeSlot::Morning);
        } else { // Sunday
            // 周日夜晚结束 -> 循环结束，周六早晨并 L+1
            setDay(DayOfWeek::Saturday);
            setSlot(TimeSlot::Morning);
            setLoopCount(m_loopCount + 1);
            // 每次循环开始，精神稳定值 -3
            setSanity(m_sanity - 3);
            // 重置一些每个循环独立的状态（如橘猫相遇标记）
            // 橘猫相遇是一次循环内可触发的，但文档中橘猫剧情要求跨循环记忆？文档描述橘猫剧情1、2、3触发条件并未明确说跨循环是否重置。
            // 由场景管理器根据 L 和循环内触发情况控制，这里暂不自动重置。
            // 背包、知识、身体素质、世界裂缝等均保留。
        }
        break;
    }
}

// ---------- 结局解锁 ----------
Ending GameState::unlockedEnding() const {
    // 返回最近满足条件的结局（如同时满足 T1 和 T2，需在 UI 中提供选择），此处仅作存储
    if (isEndingAvailable(Ending::BondSpark))
        return Ending::BondSpark;
    if (isEndingAvailable(Ending::TodayUntie))
        return Ending::TodayUntie;
    // 其它结局由场景直接调用进入，不在此列
    return Ending::None;
}

void GameState::setUnlockedEnding(Ending e) {
    // 适用于当前有可进入结局时存储
    if (e != Ending::None && !isEndingUnlocked(e))
        permanentlyUnlockEnding(e);
}

bool GameState::isEndingAvailable(Ending e) const {
    // 这里仅检查条件是否满足（如任务完成、道具持有等）
    // 实际判断在场景管理器中，GameState 只负责存储永久解锁状态
    return isEndingUnlocked(e);
}

void GameState::permanentlyUnlockEnding(Ending e) {
    if (!m_unlockedEndings.contains(e)) {
        m_unlockedEndings.append(e);
        emit endingUnlocked(e);
    }
}

bool GameState::isEndingUnlocked(Ending e) const {
    return m_unlockedEndings.contains(e);
}

// ---------- 世界裂缝 ----------
bool GameState::hasCrack(WorldCrack crack) const {
    return m_cracks.contains(crack);
}

void GameState::addCrack(WorldCrack crack) {
    if (!m_cracks.contains(crack)) {
        m_cracks.append(crack);
    }
}

QList<WorldCrack> GameState::cracks() const {
    return m_cracks;
}

// ---------- 任务完成状态 ----------
bool GameState::isQuestCompleted(Quest quest) const {
    return m_completedQuests.contains(quest);
}

void GameState::completeQuest(Quest quest) {
    if (!m_completedQuests.contains(quest)) {
        m_completedQuests.append(quest);
    }
}

// ---------- 背包 ----------
bool GameState::hasItem(Item item) const {
    return m_inventory.contains(item);
}

void GameState::addItem(Item item) {
    if (!m_inventory.contains(item)) {
        m_inventory.append(item);
        emit inventoryChanged();
    }
}

void GameState::removeItem(Item item) {
    if (m_inventory.removeAll(item) > 0) {
        emit inventoryChanged();
    }
}

QList<Item> GameState::inventoryItems() const {
    return m_inventory;
}

// ---------- 特殊标记 ----------
bool GameState::hasDoneInitialLoop() const { return m_loopCount >= 1; }

bool GameState::hasTalkedToCatScene1() const { return m_catScene1; }
bool GameState::hasTalkedToCatScene2() const { return m_catScene2; }
bool GameState::hasTalkedToCatScene3() const { return m_catScene3; }

void GameState::setCatScene1Done(bool done) { m_catScene1 = done; }
void GameState::setCatScene2Done(bool done) { m_catScene2 = done; }
void GameState::setCatScene3Done(bool done) { m_catScene3 = done; }

bool GameState::hasTriggeredWindowPaper() const { return m_windowPaper; }
bool GameState::hasTriggeredWindowPaperBB() const { return m_windowPaperBB; }
bool GameState::hasTriggeredLibraryBookAA() const { return m_libraryBookAA; }
bool GameState::hasTriggeredLibraryBookBB() const { return m_libraryBookBB; }
bool GameState::hasTriggeredAuntSaw() const { return m_auntSaw; }
bool GameState::hasTriggeredAuntDialectA() const { return m_auntDialectA; }
bool GameState::hasTriggeredAuntDialectB() const { return m_auntDialectB; }
bool GameState::hasTriggeredAuntDialectC() const { return m_auntDialectC; }
bool GameState::hasTriggeredAuntLost() const { return m_auntLost; }
bool GameState::hasFoundHandkerchiefInField() const { return m_foundHandkerchief; }

void GameState::setWindowPaperTrigg(bool v) { m_windowPaper = v; }
void GameState::setWindowPaperBBTrigg(bool v) { m_windowPaperBB = v; }
void GameState::setLibraryBookAATrigg(bool v) { m_libraryBookAA = v; }
void GameState::setLibraryBookBBTrigg(bool v) { m_libraryBookBB = v; }
void GameState::setAuntSaw(bool v) { m_auntSaw = v; }
void GameState::setAuntDialectA(bool v) { m_auntDialectA = v; }
void GameState::setAuntDialectB(bool v) { m_auntDialectB = v; }
void GameState::setAuntDialectC(bool v) { m_auntDialectC = v; }
void GameState::setAuntLost(bool v) { m_auntLost = v; }
void GameState::setFoundHandkerchiefInField(bool v) { m_foundHandkerchief = v; }

// ---------- 存档 / 读档 ----------
QJsonObject GameState::toJson() const {
    QJsonObject obj;
    obj["energy"] = m_energy;
    obj["sanity"] = m_sanity;
    obj["knowledge"] = m_knowledge;
    obj["physical"] = m_physical;
    obj["loopCount"] = m_loopCount;
    obj["sinking"] = m_sinking;
    obj["homeworkCount"] = m_homeworkCount;

    obj["day"] = static_cast<int>(m_day);
    obj["slot"] = static_cast<int>(m_slot);

    QJsonArray endingsArr;
    for (Ending e : m_unlockedEndings) {
        endingsArr.append(static_cast<int>(e));
    }
    obj["unlockedEndings"] = endingsArr;

    QJsonArray cracksArr;
    for (WorldCrack c : m_cracks) {
        cracksArr.append(static_cast<int>(c));
    }
    obj["cracks"] = cracksArr;

    QJsonArray questsArr;
    for (Quest q : m_completedQuests) {
        questsArr.append(static_cast<int>(q));
    }
    obj["completedQuests"] = questsArr;

    QJsonArray itemsArr;
    for (Item it : m_inventory) {
        itemsArr.append(static_cast<int>(it));
    }
    obj["inventory"] = itemsArr;

    // 标记
    obj["catScene1"] = m_catScene1;
    obj["catScene2"] = m_catScene2;
    obj["catScene3"] = m_catScene3;
    obj["windowPaper"] = m_windowPaper;
    obj["windowPaperBB"] = m_windowPaperBB;
    obj["libraryBookAA"] = m_libraryBookAA;
    obj["libraryBookBB"] = m_libraryBookBB;
    obj["auntSaw"] = m_auntSaw;
    obj["auntDialectA"] = m_auntDialectA;
    obj["auntDialectB"] = m_auntDialectB;
    obj["auntDialectC"] = m_auntDialectC;
    obj["auntLost"] = m_auntLost;
    obj["foundHandkerchief"] = m_foundHandkerchief;

    return obj;
}

void GameState::fromJson(const QJsonObject &obj) {
    m_energy = obj.value("energy").toInt(100);
    m_sanity = obj.value("sanity").toInt(80);
    m_knowledge = obj.value("knowledge").toInt(0);
    m_physical = obj.value("physical").toInt(0);
    m_loopCount = obj.value("loopCount").toInt(0);
    m_sinking = obj.value("sinking").toInt(0);
    m_homeworkCount = obj.value("homeworkCount").toInt(0);

    m_day = static_cast<DayOfWeek>(obj.value("day").toInt(0));
    m_slot = static_cast<TimeSlot>(obj.value("slot").toInt(0));

    m_unlockedEndings.clear();
    for (auto val : obj.value("unlockedEndings").toArray()) {
        m_unlockedEndings.append(static_cast<Ending>(val.toInt()));
    }

    m_cracks.clear();
    for (auto val : obj.value("cracks").toArray()) {
        m_cracks.append(static_cast<WorldCrack>(val.toInt()));
    }

    m_completedQuests.clear();
    for (auto val : obj.value("completedQuests").toArray()) {
        m_completedQuests.append(static_cast<Quest>(val.toInt()));
    }

    m_inventory.clear();
    for (auto val : obj.value("inventory").toArray()) {
        m_inventory.append(static_cast<Item>(val.toInt()));
    }

    m_catScene1 = obj.value("catScene1").toBool(false);
    m_catScene2 = obj.value("catScene2").toBool(false);
    m_catScene3 = obj.value("catScene3").toBool(false);
    m_windowPaper = obj.value("windowPaper").toBool(false);
    m_windowPaperBB = obj.value("windowPaperBB").toBool(false);
    m_libraryBookAA = obj.value("libraryBookAA").toBool(false);
    m_libraryBookBB = obj.value("libraryBookBB").toBool(false);
    m_auntSaw = obj.value("auntSaw").toBool(false);
    m_auntDialectA = obj.value("auntDialectA").toBool(false);
    m_auntDialectB = obj.value("auntDialectB").toBool(false);
    m_auntDialectC = obj.value("auntDialectC").toBool(false);
    m_auntLost = obj.value("auntLost").toBool(false);
    m_foundHandkerchief = obj.value("foundHandkerchief").toBool(false);

    // 发出所有变更信号，让 UI 刷新
    emit energyChanged(m_energy);
    emit sanityChanged(m_sanity);
    emit knowledgeChanged(m_knowledge);
    emit physicalChanged(m_physical);
    emit loopCountChanged(m_loopCount);
    emit sinkingChanged(m_sinking);
    emit homeworkCountChanged(m_homeworkCount);
    emit dayChanged(m_day);
    emit slotChanged(m_slot);
    emit inventoryChanged();
    // 结局解锁信号不逐个发出，UI 可查询
}