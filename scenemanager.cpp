#include "scenemanager.h"
#include "mainwindow.h"
#include "storydisplay.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRandomGenerator>
#include <QCoreApplication>

SceneManager::SceneManager(MainWindow *window, QObject *parent)
    : QObject(parent), m_window(window), m_state(window->gameState())
{
    m_lastDay = m_state->currentDay();
    m_currentLoop = m_state->loopCount();
    m_isEnding = false;
    QString path = QCoreApplication::applicationDirPath() + "/scenario_data.json";
    loadScenarioData(path);
    connect(m_window, &MainWindow::optionSelected, this, &SceneManager::onOptionSelected);
    // 连接文本播放完毕信号：显示完所有段落后，处理选项或自动跳转
    connect(m_window->storyDisplay(), &StoryDisplay::allParagraphsShown,
            this, &SceneManager::onStoryFinished);
}

void SceneManager::loadScenarioData(const QString &filePath)
{
    qDebug() << "尝试加载 JSON：" << filePath;
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "失败：" << file.errorString();
        return;
    }
    QByteArray raw = file.readAll();
    // 去除 UTF-8 BOM（如果存在）
    if (raw.startsWith("\xEF\xBB\xBF"))
        raw.remove(0, 3);
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(raw, &error);
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON 解析错误：" << error.errorString();
        qDebug() << "错误位置（字节偏移）：" << error.offset;
        int start = qMax(0, error.offset - 30);
        int len = qMin(60, raw.size() - start);
        qDebug() << "出错附近内容：" << raw.mid(start, len);
        return;
    }
    m_data = doc.object();
    qDebug() << "JSON 加载成功，包含节点数：" << m_data.keys().size();
    file.close();
}

void SceneManager::startGame()
{
    m_isEnding = false;
    // 节点1：显示开场文本和初始选项
    processNode("node1_intro");
}

void SceneManager::processNode(const QString &nodeKey)
{
    qDebug() << "当前节点:" << nodeKey << " 沉沦值:" << m_state->sinking();
    if (!m_data.contains(nodeKey)) return;
    // 新增：检测结局节点
    if (nodeKey.startsWith("ending_")) {
        m_isEnding = true;
    }
    // 原有代码继续...
    QJsonObject node = m_data[nodeKey].toObject();
    currentNode = nodeKey;

    // 重置每日标记（如果日期改变）
    if (m_state->currentDay() != m_lastDay) {
        m_dailyMarkers.clear();
        m_lastDay = m_state->currentDay();
    }
    // 重置循环标记（如果循环次数变化）
    if (m_state->loopCount() != m_currentLoop) {
        // 循环递增
        // 将当前循环的任务完成标记转移为“前循环完成”
        if (m_markers.contains("aunt_task_done")) {
            m_markers.remove("aunt_task_done");
            m_markers.insert("aunt_task_prev_loop");
        }
        // 重置其他循环标记
        m_loopMarkers.clear();
        m_markers.remove("refused_to_return");
        m_markers.remove("morning_penalty_applied");
        m_markers.remove("locked_in_dorm_until_morning");
        m_currentLoop = m_state->loopCount();
    }

    if (!m_data.contains(nodeKey)) return;
    currentNode = nodeKey;

    if (node.contains("random_chance") && node.contains("random_next")) {
        int chance = node["random_chance"].toInt();
        if (QRandomGenerator::global()->bounded(100) < chance) {
            goToNode(node["random_next"].toString());
        } else if (node.contains("default_next")) {
            goToNode(node["default_next"].toString());
        } else {
            // 没有默认时，继续处理当前节点（可能还有选项等）
        }
        return; // 已跳转，不处理后续
    }

    // 0. 检查特殊动作：推进时间
    if (node.contains("advance_time") && node["advance_time"].toBool()) {
        // 使用封装函数以检查循环超限和惩罚
        // 但此时我们可能没有 nextNode，所以需要存储后续跳转
        // 更好的做法：在 advanceTimeAndGoTo 中推进时间后继续处理，但这里我们直接调用并检查
        m_state->advanceTime();
        if (m_state->loopCount() > 52 && !m_isEnding) {
            m_isEnding = true;
            goToNode("ending_B2");
            return;
        }
        // 早晨惩罚检查
        if (m_markers.contains("refused_to_return") && !m_markers.contains("morning_penalty_applied") &&
            m_state->currentSlot() == TimeSlot::Morning) {
            m_state->setSanity(m_state->sanity() - 5);
            m_markers.insert("morning_penalty_applied");
            // 注意：这里直接显示文本而不通过故事节点，可能会打断后续流程。
            // 但由于 morning penalty 触发应该是独立通知，我们可以像上面那样展示。
            // 需要确保在显示惩罚文本后能继续处理当前节点。
            // 我们暂保持简单，先不显示额外文本，仅扣数值，或留到后续优化。
        }
    }

    // 1. 显示段落
    if (node.contains("paragraphs")) {
        QJsonArray arr = node["paragraphs"].toArray();
        QStringList paragraphs;
        for (const auto &p : arr) {
            paragraphs.append(p.toString());
        }
        m_window->showStoryParagraphs(paragraphs);
    }

    // 2. 应用效果
    if (node.contains("effect")) {
        applyEffect(node["effect"].toObject());
    }

    // 3. 复杂条件检查
    if (node.contains("complex_condition")) {
        QString cond = node["complex_condition"].toString();
        if (evaluateComplexCondition(cond)) {
            if (node.contains("true_next")) {
                goToNode(node["true_next"].toString());
                return;
            }
        } else {
            if (node.contains("false_next")) {
                goToNode(node["false_next"].toString());
                return;
            }
        }
    }

    // 4. 简单条件检查（如 N > 3 等）
    if (node.contains("condition")) {
        QString cond = node["condition"].toString();
        if (checkCondition(cond)) {
            if (node.contains("true_next")) {
                goToNode(node["true_next"].toString());
                return;
            }
        } else {
            if (node.contains("false_next")) {
                goToNode(node["false_next"].toString());
                return;
            }
        }
    }

    // 5. 处理选项
    if (node.contains("options")) {
        QJsonArray opts = node["options"].toArray();
        m_optionNextNodes.clear();
        QStringList labels;
        for (const auto &opt : opts) {
            QJsonObject optObj = opt.toObject();
            // 可选的条件显示选项
            if (optObj.contains("condition")) {
                QString condStr = optObj["condition"].toString();
                if (!evaluateComplexCondition(condStr) && !checkCondition(condStr))
                    continue; // 不满足条件则不显示该选项
            }
            labels.append(optObj["label"].toString());
            m_optionNextNodes.append(optObj["next"].toString());
        }
        if (!labels.isEmpty()) {
            m_pendingOptions = labels;
            m_waitingForOption = true;
            if (!node.contains("paragraphs")) {
                m_window->setOptions(labels);
                m_pendingOptions.clear();
            }
        }
    }

    // 在 processNode 函数中，于条件处理部分之后添加：
    if (node.contains("cases")) {
        QJsonObject cases = node["cases"].toObject();
        for (auto it = cases.begin(); it != cases.end(); ++it) {
            QJsonObject caseObj = it.value().toObject();
            QString cond = caseObj["condition"].toString();
            if (evaluateComplexCondition(cond)) {
                goToNode(caseObj["next"].toString());
                return;
            }
        }
        // 若无匹配，可定义默认 next
        if (node.contains("default_next"))
            goToNode(node["default_next"].toString());
        return;
    }

    if (node.contains("random_weighted")) {
        QJsonObject weighted = node["random_weighted"].toObject();
        if (weighted.contains("once_per_loop") && weighted["once_per_loop"].toBool()) {
            if (m_loopMarkers.contains("random_discovery_done")) {
                // 已触发过，直接跳转到 default_next 或 next
                if (node.contains("next")) goToNode(node["next"].toString());
                return;
            }
        }
        QJsonArray items = weighted["items"].toArray();
        int totalWeight = 0;
        QVector<QPair<int, QJsonObject>> probList;
        for (const auto &val : items) {
            QJsonObject obj = val.toObject();
            int w = obj["chance"].toInt();
            totalWeight += w;
            probList.append({w, obj});
        }
        int rand = QRandomGenerator::global()->bounded(totalWeight);
        int accum = 0;
        for (const auto &pair : probList) {
            accum += pair.first;
            if (rand < accum) {
                QJsonObject chosen = pair.second;
                if (chosen.contains("paragraph")) {
                    // 显示获得物品的段落
                    QStringList paras;
                    paras.append(chosen["paragraph"].toString());
                    m_window->showStoryParagraphs(paras);
                }
                if (chosen.contains("item")) {
                    QString itemName = chosen["item"].toString();
                    applyEffect(QJsonObject{{"add_item", itemName}});
                    // 特殊效果（如体力恢复）可以在这里手动调用
                    if (itemName == "PearCandy") m_state->setEnergy(m_state->energy()+5);
                    else if (itemName == "TornNote") m_state->setKnowledge(m_state->knowledge()+2);
                }
                if (weighted.contains("once_per_loop") && weighted["once_per_loop"].toBool()) {
                    m_loopMarkers.insert("random_discovery_done");
                }
                if (node.contains("next")) goToNode(node["next"].toString());
                return;
            }
        }
    }

    if (node.contains("random_events")) {
        QJsonObject events = node["random_events"].toObject();
        QJsonArray eventList = events["events"].toArray();
        // 根据权重选择
        int totalChance = 0;
        QVector<QPair<int, QJsonObject>> choices;
        for (const auto &val : eventList) {
            QJsonObject ev = val.toObject();
            if (ev.contains("condition")) {
                if (!evaluateComplexCondition(ev["condition"].toString()))
                    continue;
            }
            int c = ev["chance"].toInt();
            totalChance += c;
            choices.append({c, ev});
        }
        if (totalChance == 0) {
            // 无符合条件的事件，走默认
            if (node.contains("next")) goToNode(node["next"].toString());
            return;
        }
        int rand = QRandomGenerator::global()->bounded(totalChance);
        int accum = 0;
        for (const auto &pair : choices) {
            accum += pair.first;
            if (rand < accum) {
                goToNode(pair.second["next"].toString());
                return;
            }
        }
    }

    // 6. 自动跳转
    else if (node.contains("next")) {
        m_autoNext = node["next"].toString();
        if (!node.contains("paragraphs")) {
            goToNode(m_autoNext);
            m_autoNext.clear();
        }
    }
}

bool SceneManager::evaluateComplexCondition(const QString &cond)
{
    // 道具持有检查
    if (cond.startsWith("has_item:")) {
        QString itemName = cond.mid(9);
        if (itemName == "AudioRecording") return m_state->hasItem(Item::AudioRecording);
        if (itemName == "StudentCard") return m_state->hasItem(Item::StudentCard);
        if (itemName == "AuntHandkerchief") return m_state->hasItem(Item::AuntHandkerchief);
        if (itemName == "OldAssignment") return m_state->hasItem(Item::OldAssignment);
        if (itemName == "SciFiBookExcerpt") return m_state->hasItem(Item::SciFiBookExcerpt);
        return false;
    }

    // 标记检查
    if (cond.startsWith("marker:")) {
        QString marker = cond.mid(7);
        return m_markers.contains(marker);
    }

    // 特殊复合条件
    if (cond == "can_trigger_AAA") {
        return m_state->hasItem(Item::OldAssignment) &&
               m_state->hasItem(Item::SciFiBookExcerpt) &&
               m_state->currentDay() == DayOfWeek::Sunday &&
               m_state->currentSlot() == TimeSlot::Night;
    }

    if (cond == "is_weekend_night") {
        return m_state->isWeekendNight();
    }
    if (cond == "is_sunday_night") {
        return m_state->currentDay() == DayOfWeek::Sunday && m_state->currentSlot() == TimeSlot::Night;
    }
    if (cond == "is_saturday_night") {
        return m_state->currentDay() == DayOfWeek::Saturday && m_state->currentSlot() == TimeSlot::Night;
    }

    // ----- 新增四个条件判断 -----
    if (cond == "morning_or_night") {
        TimeSlot slot = m_state->currentSlot();
        return slot == TimeSlot::Morning || slot == TimeSlot::Night;
    }

    if (cond == "probe_possible") {
        return m_state->knowledge() > 10 && m_state->loopCount() >= 4;
    }

    if (cond == "dorm_post_done") {
        return m_markers.contains("dorm_post_done");
    }

    if (cond.startsWith("not ")) {
        QString inner = cond.mid(4);
        return !evaluateComplexCondition(inner);
    }
    // ---------------------------

    if (cond.startsWith("marker:")) {
        return m_markers.contains(cond.mid(7));
    }

    // 数值比较
    if (cond.startsWith("L_ge:")) {
        int val = cond.mid(5).toInt();
        return m_state->loopCount() >= val;
    }
    if (cond.startsWith("L_lt:")) {
        int val = cond.mid(5).toInt();
        return m_state->loopCount() < val;
    }
    // 标记检查
    if (cond.startsWith("marker:")) {
        return m_markers.contains(cond.mid(7));
    }
    if (cond.startsWith("!marker:")) {
        return !m_markers.contains(cond.mid(8));
    }
    // 时间相关
    if (cond == "is_meal_time") {
        TimeSlot slot = m_state->currentSlot();
        return slot == TimeSlot::Noon || slot == TimeSlot::Evening;
    }
    if (cond == "canteen_aunt_visible") {
        return m_markers.contains("aunt_seen");
    }
    // 任务完成检查
    if (cond == "quest_handkerchief_done") {
        return m_state->isQuestCompleted(Quest::Handkerchief);
    }
    // 组合：L_ge:8 && has_item:AuntHandkerchief 等已在 cases 条件中拆分为简单条件，由单个 evaluateComplexCondition 不支持逻辑与，因此我们在 cases 结构中直接使用组合的简单条件表达式，我们的 evaluateComplexCondition 需能解析 &&。为了简便，可专门解析 "&&"。
    if (cond.contains("&&")) {
        QStringList parts = cond.split("&&");
        for (const QString& part : parts) {
            if (!evaluateComplexCondition(part.trimmed()))
                return false;
        }
        return true;
    }

    if (cond.startsWith("K_gt:")) {
        int val = cond.mid(5).toInt();
        return m_state->knowledge() > val;
    }
    if (cond == "quest_piano_done") {
        return m_state->isQuestCompleted(Quest::PianoWish);
    }
    if (cond == "is_evening") {
        return m_state->currentSlot() == TimeSlot::Evening;
    }
    if (cond.startsWith("daily_marker:")) {
        QString marker = cond.mid(13);
        return m_dailyMarkers.contains(marker);
    }
    if (cond.startsWith("!daily_marker:")) {
        return !m_dailyMarkers.contains(cond.mid(14));
    }
    if (cond.startsWith("loop_marker:")) {
        return m_loopMarkers.contains(cond.mid(12));
    }
    if (cond.startsWith("!loop_marker:")) {
        return !m_loopMarkers.contains(cond.mid(13));
    }
    if (cond.contains("&&")) {
        QStringList parts = cond.split("&&");
        for (const QString &part : parts)
            if (!evaluateComplexCondition(part.trimmed()))
                return false;
        return true;
    }

    if (cond == "quest_handkerchief_done") {
        return m_state->isQuestCompleted(Quest::Handkerchief);
    }
    if (cond == "marker:cat_teaching_done") {
        return m_markers.contains("cat_teaching_done");
    }

    if (cond == "marker:cat_library_done") {
        return m_markers.contains("cat_library_done");
    }
    if (cond == "marker:aunt_lost_triggered") {
        return m_markers.contains("aunt_lost_triggered");
    }
    if (cond.startsWith("!has_item:")) {
        QString itemName = cond.mid(10);
        if (itemName == "AuntHandkerchief") return !m_state->hasItem(Item::AuntHandkerchief);
        // 可扩展
        return true;
    }

    if (cond == "sanity_lt:60") {
        return m_state->sanity() < 60;
    }
    if (cond == "K_gt:10") {
        return m_state->knowledge() > 10;
    }
    if (cond == "K_gt:20") {
        return m_state->knowledge() > 20;
    }
    if (cond == "Q_gt:20") {
        return m_state->physical() > 20;
    }
    if (cond.startsWith("first_time:")) {
        QString flag = cond.mid(11);
        if (!m_firstTimeFlags.contains(flag)) {
            m_firstTimeFlags.insert(flag);
            return true;
        }
        return false;
    }

    if (cond == "can_trigger_hidden_ending") {
        // 满足T1和T2条件（即对应任务完成和道具持有）且收集了三个世界裂缝
        bool t1Ready = m_state->isQuestCompleted(Quest::Handkerchief) &&
                       m_state->isQuestCompleted(Quest::PianoWish) &&
                       m_state->isQuestCompleted(Quest::CatSunset);
        bool t2Ready = m_state->hasItem(Item::OldAssignment) && m_state->hasItem(Item::SciFiBookExcerpt);
        bool cracksReady = m_state->hasCrack(WorldCrack::CorridorShadow) &&
                           m_state->hasCrack(WorldCrack::BlankPaper) &&
                           m_state->hasCrack(WorldCrack::TestRecordOnDesk);
        return t1Ready && t2Ready && cracksReady;
    }

    if (cond == "is_locked_out_of_dorm") {
        // 如果玩家曾拒绝回宿舍且未翻窗进入，或夜晚被锁在外面
        if (m_markers.contains("refused_to_return") && !m_markers.contains("entered_via_window"))
            return true;
        if (m_state->isWeekendNight() && !m_markers.contains("returned_to_dorm"))
            return true;
        return false;
    }
    if (cond == "can_leave_normally") {
        if (m_state->isWeekendNight() && m_markers.contains("locked_in_dorm_until_morning"))
            return false;
        return true;
    }
    if (cond == "can_leave_by_window") {
        if (m_markers.contains("locked_in_dorm_until_morning") && m_state->physical() > 15)
            return true;
        return false;
    }
    if (cond == "Q_gt:15") {
        return m_state->physical() > 15;
    }

    return false;
}

void SceneManager::advanceTimeAndGoTo(const QString &nextNode)
{
    m_state->advanceTime();

    // 检查循环次数超限
    if (m_state->loopCount() > 52 && !m_isEnding) {
        m_isEnding = true;
        goToNode("ending_B2");
        return;
    }

    // 检查早晨惩罚
    if (m_markers.contains("refused_to_return") && !m_markers.contains("morning_penalty_applied") &&
        m_state->currentSlot() == TimeSlot::Morning) {
        m_state->setSanity(m_state->sanity() - 5);
        m_markers.insert("morning_penalty_applied");
        m_window->showStoryParagraphs({
            "天亮了。你整夜未归宿舍，感觉精神有些疲惫。",
            "精神稳定值-5。"
        });
    }
    goToNode(nextNode);
}

void SceneManager::returnToCampusHub()
{
    m_previousLocation = "campus_hub";
    goToNode("campus_hub");
}

void SceneManager::onStoryFinished()
{
    if (m_isEnding) return;
    // 文本显示完毕
    if (m_waitingForOption && !m_pendingOptions.isEmpty()) {
        m_window->setOptions(m_pendingOptions);
        m_pendingOptions.clear();
    }
    else if (!m_autoNext.isEmpty()) {
        goToNode(m_autoNext);
        m_autoNext.clear();
    }
}

void SceneManager::onOptionSelected(int index)
{
    if (!m_waitingForOption) return;
    m_waitingForOption = false;
    if (index >= 0 && index < m_optionNextNodes.size()) {
        QString next = m_optionNextNodes[index];
        // 处理特殊效果（例如选择“再睡一会儿”增加体力等）
        // 可通过 JSON 中选项附带 effects，这里简化：如果 next 是 "node1_sleep_effect" 等节点处理
        goToNode(next);
    }
}

void SceneManager::checkGameOver()
{
    if (m_isEnding) return;
    if (m_state->energy() <= 0 || m_state->sinking() > 7 || m_state->loopCount() > 52) {
        m_isEnding = true;
        goToNode("ending_B2");
    }
}

void SceneManager::applyEffect(const QJsonObject &effect)
{
    // 基础属性变动
    if (effect.contains("energy"))
        m_state->setEnergy(m_state->energy() + effect["energy"].toInt());
    if (effect.contains("sanity"))
        m_state->setSanity(m_state->sanity() + effect["sanity"].toInt());
    if (effect.contains("knowledge"))
        m_state->setKnowledge(m_state->knowledge() + effect["knowledge"].toInt());
    if (effect.contains("physical"))
        m_state->setPhysical(m_state->physical() + effect["physical"].toInt());
    if (effect.contains("sinking"))
        m_state->setSinking(m_state->sinking() + effect["sinking"].toInt());
    if (effect.contains("homework")) {
        int delta = effect["homework"].toInt();
        m_state->setHomeworkCount(m_state->homeworkCount() + delta);
    }
    if (effect.contains("loop"))
        m_state->setLoopCount(m_state->loopCount() + effect["loop"].toInt());
    if (effect.contains("homework_reset") && effect["homework_reset"].toBool())
        m_state->setHomeworkCount(0);

    // 通用标记
    if (effect.contains("marker"))
        m_markers.insert(effect["marker"].toString());

    if (effect.contains("loop_marker")) {
        m_loopMarkers.insert(effect["loop_marker"].toString());
    }
    if (effect.contains("daily_marker")) {
        m_dailyMarkers.insert(effect["daily_marker"].toString());
    }

    // 完成任务
    if (effect.contains("complete_quest")) {
        QString quest = effect["complete_quest"].toString();
        if (quest == "Handkerchief")
            m_state->completeQuest(Quest::Handkerchief);
        else if (quest == "PianoWish")
            m_state->completeQuest(Quest::PianoWish);
        else if (quest == "CatSunset")
            m_state->completeQuest(Quest::CatSunset);
    }

    // 添加道具
    if (effect.contains("add_item")) {
        QString item = effect["add_item"].toString();
        if (item == "StudentCard")
            m_state->addItem(Item::StudentCard);
        else if (item == "AudioRecording")
            m_state->addItem(Item::AudioRecording);
        else if (item == "WishCard")
            m_state->addItem(Item::WishCard);
        else if (item == "AuntHandkerchief")
            m_state->addItem(Item::AuntHandkerchief);
        else if (item == "OldAssignment")
            m_state->addItem(Item::OldAssignment);
        else if (item == "SciFiBookExcerpt")
            m_state->addItem(Item::SciFiBookExcerpt);
        else if (item == "PearCandy")
            m_state->addItem(Item::PearCandy);
        else if (item == "TornNote")
            m_state->addItem(Item::TornNote);
        else if (item == "HalfChalk")
            m_state->addItem(Item::HalfChalk);
        else if (item == "OneYuanCoin")
            m_state->addItem(Item::OneYuanCoin);
        else if (item == "MapleBookmark")
            m_state->addItem(Item::MapleBookmark);
        else if (item == "OldDiaryPage")
            m_state->addItem(Item::OldDiaryPage);
        else if (item == "HandDrawnMap")
            m_state->addItem(Item::HandDrawnMap);
        else if (item == "InstantCoffee")
            m_state->addItem(Item::InstantCoffee);
        else if (item == "LargeSycamoreLeaf")
            m_state->addItem(Item::LargeSycamoreLeaf);
        else if (item == "DarkBlueButton")
            m_state->addItem(Item::DarkBlueButton);
        else if (item == "KeychainWithBell")
            m_state->addItem(Item::KeychainWithBell);
        else if (item == "SportsDrink")
            m_state->addItem(Item::SportsDrink);
        else if (item == "WishCard")
            m_state->addItem(Item::WishCard);
        else if (item == "Pinecone")
            m_state->addItem(Item::Pinecone);
        else if (item == "DoodlePaper")
            m_state->addItem(Item::DoodlePaper);
        else if (item == "BrokenEarphone")
            m_state->addItem(Item::BrokenEarphone);
        else if (item == "SmoothPebble")
            m_state->addItem(Item::SmoothPebble);
        // 后续可在此扩展其他道具
    }

    if (effect.contains("effect_on_get")) {
        QJsonObject getEff = effect["effect_on_get"].toObject();
        if (getEff.contains("sanity")) m_state->setSanity(m_state->sanity() + getEff["sanity"].toInt());
        if (getEff.contains("energy")) m_state->setEnergy(m_state->energy() + getEff["energy"].toInt());
    }

    // 移除道具
    if (effect.contains("remove_item")) {
        QString item = effect["remove_item"].toString();
        if (item == "AudioRecording")
            m_state->removeItem(Item::AudioRecording);
        else if (item == "StudentCard")
            m_state->removeItem(Item::StudentCard);
        else if (item == "AuntHandkerchief")
            m_state->removeItem(Item::AuntHandkerchief);
        // 后续扩展
    }

    // 添加世界裂缝
    if (effect.contains("add_crack")) {
        QString crack = effect["add_crack"].toString();
        if (crack == "BlankPaper")
            m_state->addCrack(WorldCrack::BlankPaper);
        else if (crack == "CorridorShadow")
            m_state->addCrack(WorldCrack::CorridorShadow);
        else if (crack == "TestRecordOnDesk")
            m_state->addCrack(WorldCrack::TestRecordOnDesk);
    }

    if (effect.contains("permanently_unlock_ending")) {
        QString ending = effect["permanently_unlock_ending"].toString();
        if (ending == "BondSpark") m_state->permanentlyUnlockEnding(Ending::BondSpark);
        // 其他结局
    }

    if (effect.contains("set_flag")) {
        m_markers.insert(effect["set_flag"].toString());
    }

    checkGameOver();
}

bool SceneManager::checkCondition(const QString &conditionExpr)
{
    qDebug() << "checkCondition 收到：" << conditionExpr << " 当前沉沦值：" << m_state->sinking();
    if (conditionExpr == "M > 3")  return m_state->sinking() > 3;
    if (conditionExpr == "L == 0") return m_state->loopCount() == 0;
    if (conditionExpr == "L >= 1") return m_state->loopCount() >= 1;
    if (conditionExpr == "N == 0") return m_state->homeworkCount() == 0;
    if (conditionExpr == "N > 0")  return m_state->homeworkCount() > 0;
    if (conditionExpr == "N > 3") return m_state->homeworkCount() > 3;
    if (conditionExpr == "N > 19") return m_state->homeworkCount() > 19;

    return false;
}

void SceneManager::goToNode(const QString &nodeKey)
{
    if (nodeKey.isEmpty()) return;
    processNode(nodeKey);
}

void SceneManager::enterEnding(const QString &endingNode)
{
    currentNode = endingNode;
    m_isEnding = true;
    processNode(endingNode);
}

void SceneManager::restoreFromLoad(const QString &nodeKey)
{
    // 读档后重置界面并跳转至存储的节点
    m_window->clearOptions();
    currentNode = nodeKey;
    // 重置标记（如 pending 等）
    m_waitingForOption = false;
    m_pendingOptions.clear();
    m_autoNext.clear();
    m_isEnding = false;
    processNode(nodeKey);
}