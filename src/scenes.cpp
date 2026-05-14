#include "scenes.h"
#include "gameview.h"
#include "gamestate.h"

#include <QRandomGenerator>
#include <QStringList>

static int randPercent() {
    return QRandomGenerator::global()->bounded(100);
}
static bool chance(int percent) {
    return randPercent() < percent;
}
static int pickWeighted(const QList<int>& weights) {
    int total = 0;
    for (int w : weights) total += w;
    if (total <= 0) return 0;
    int r = QRandomGenerator::global()->bounded(total);
    int acc = 0;
    for (int i = 0; i < weights.size(); ++i) {
        acc += weights[i];
        if (r < acc) return i;
    }
    return weights.size() - 1;
}

#define MW m_mw
#define ST m_st

SceneEngine::SceneEngine(IGameView* view, GameState* st, QObject* parent)
    : QObject(parent), m_mw(view), m_st(st) {}

void SceneEngine::refreshHud() { MW->refreshHud(); }

QString SceneEngine::endingsHint() const {
    if (ST->unlockedEndings.isEmpty()) return QString();
    QStringList list;
    for (const QString& s : ST->unlockedEndings) list << QStringLiteral("【%1】").arg(s);
    return QStringLiteral("<br/><span style='color:#ffd97a;font-size:13px;'>已解锁结局入口：%1</span>")
        .arg(list.join('/'));
}

// =====================================================================
// 开场
// =====================================================================
void SceneEngine::startIntro() {
    ST->clampStats();
    refreshHud();
    const QString p1 = QStringLiteral(
        "你是南开大学计算机专业的一名学生，在津南校区度过了将近两学期风平浪静的大学生活。<br/>"
        "你习惯了理科组团到文科快递站的距离，习惯了夜晚公教楼的风声，<br/>"
        "原本，你和所有人一样，都觉得日子会像马蹄湖的湖水一样，波澜不惊地流淌下去。<br/>"
        "直到那个早晨——");

    const QString p2 = QStringLiteral(
        "「你今天这么早就起来了？」<br/><br/>"
        "刚坐起身的你恍惚了一瞬，看向对面。【室友A】还躺在床上，"
        "拉开床帘一角，揉了揉眼睛，看着你。<br/>"
        "「今天不是有早八吗？」你疑惑地回答。<br/>"
        "「你起太早迷糊了吧？今天是周六啊。」【室友A】说完，便又躺了回去。<br/>"
        "你睡意全无。");

    const QString p3 = QStringLiteral(
        "今天不是周一吗？刚刚过去的整个周末，你起早贪黑，"
        "在宿舍完成了 C++ 课程大作业的代码和视频的剪辑。<br/>"
        "做是做完了，但你迟迟没有点下提交键。"
        "你总觉得哪里还差点，又说不上来。<br/>"
        "你对自己说，周一早上头脑最清醒，最后检查一遍就交。<br/>"
        "想到这里，你飞快地下床，坐在桌前，怀着一丝侥幸，打开了电脑……");

    const QString p4 = QStringLiteral(
        "大作业文件夹里空空如也。<br/>"
        "你反复刷新了几次，文件夹仍然空着，干净得就像这个周末从未存在过一样。<br/>"
        "你的视线僵硬地下移，落在屏幕右下角显示时间的位置——<b>周六，06:33。</b><br/>"
        "一股凉意从脊椎爬升。");

    const QString p5 = QStringLiteral(
        "你盯着那行文字看了许久，直到屏幕的光刺得眼睛发酸。<br/>"
        "但你仍旧是一个相信科学的人。<br/>"
        "你试图说服自己，那只是一场逼真的梦，如今梦醒了，"
        "也该抓紧这个真正的「周末」，把作业重新赶出来。<br/>"
        "——只是，那两天的情景与细节还历历在目，如此真实，真的是梦吗？");

    const QString p6 = QStringLiteral(
        "你又起早贪黑地做了一整个周末。这次，你准备提交了再去睡觉。<br/>"
        "周日深夜，你盯着提交按钮，手指悬在触摸板上方，"
        "最后确认了一遍，然后点了提交。<br/>"
        "网页转了几圈，显示提交成功。<br/>"
        "你长出一口气，精疲力竭地爬上床，沉沉睡去……");

    const QString p7 = QStringLiteral(
        "闹铃声再度响起。<br/>"
        "你坐起身，拉开床帘。<br/><br/>"
        "「你今天这么早就起来了？」<br/><br/>"
        "你缓缓低下头，按亮手机屏幕，那上面赫然写着——<br/>"
        "<b>周六，06:33。</b>");

    // 之后是 7 段开场剧情;
    // resumeFromHub() 用于读档时跳过开场, 直接进入 hub 探索
    MW->showNarration(p1, [=]() {
    MW->showNarration(p2, [=]() {
    MW->showNarration(p3, [=]() {
    MW->showNarration(p4, [=]() {
    MW->showNarration(p5, [=]() {
    MW->showNarration(p6, [=]() {
    MW->showNarration(p7, [=]() {
        node1();
    });});});});});});});
}

void SceneEngine::resumeFromHub() {
    ST->clampStats();
    refreshHud();
    if (checkBadEndings()) return;
    campusHub();
}

void SceneEngine::resumeByCheckpoint() {
    ST->clampStats();
    refreshHud();
    if (checkBadEndings()) return;
    const QString& cp = ST->sceneCheckpoint;
    if      (cp == QStringLiteral("node1"))           node1();
    else if (cp == QStringLiteral("node3"))           node3();
    else if (cp == QStringLiteral("dormScene"))       dormScene();
    else if (cp == QStringLiteral("canteenScene"))    canteenScene();
    else if (cp == QStringLiteral("teachingScene"))   teachingScene();
    else if (cp == QStringLiteral("libraryScene"))    libraryScene();
    else if (cp == QStringLiteral("playgroundScene")) playgroundScene();
    else if (cp == QStringLiteral("wanderScene"))     wanderScene();
    else if (cp == QStringLiteral("pianoScene"))      pianoScene();
    else                                              campusHub();
}

// =====================================================================
// 节点 1: 初始选择
// =====================================================================
void SceneEngine::node1() {
    ST->sceneCheckpoint = QStringLiteral("node1");
    ST->day = Day::Saturday;
    ST->slot = TimeSlot::Morning;
    refreshHud();

    QVector<Choice> choices;
    choices.append({QStringLiteral("再睡一会儿"), [=]() {
        QString t = QStringLiteral(
            "你闭上眼，躺在柔软的枕头上，只当一切都没有发生，"
            "内心盼望着，下一次醒来，见到的是真实的世界。<br/>"
            "<i>（体力值 +5，精神稳定值 -5）</i>");
        ST->E += 5; ST->S -= 5; ST->M += 1;
        ST->clampStats();
        refreshHud();
        // 给玩家一点提示, 避免不知情触发 B2
        if (ST->M == 2) {
            t += QStringLiteral("<br/><br/><span style='color:#ffaa88;'>"
                                "（你感到心里有什么在慢慢沉下去……）</span>");
        } else if (ST->M == 3) {
            t += QStringLiteral("<br/><br/><span style='color:#ff8866;'>"
                                "（再这样下去，你怕是真的爬不起来了。）</span>");
        }
        MW->showNarration(t, [=]() {
            if (ST->M > 3) {
                triggerEndingB2();
            } else {
                node1();
            }
        });
    }});

    choices.append({QStringLiteral("下床"), [=]() {
        if (ST->L == 0) {
            QString t = QStringLiteral(
                "你掀开被子，踩着楼梯下床。<br/>"
                "往日不知走了多少次的楼梯，这次，你扶着身旁栏杆，"
                "才勉强稳住脚步，却仍止不住细微的颤抖。<br/>"
                "你走到桌前坐下，呼吸急促。<br/>"
                "到底发生了什么？这还是原本的世界吗？<br/>"
                "你不知道。你不知道该做什么。<br/>"
                "脑子里像有一团浆糊。<br/>"
                "你只知道，你唯一确定的，就是该做作业。");
            MW->showNarration(t, [=]() { node2(); });
        } else {
            QString t = QStringLiteral(
                "你掀开被子，踩着楼梯下床。脚步还是有点飘，但不抖了。<br/>"
                "你走到桌前坐下，却没有按下电脑开机键。<br/>"
                "手机上的时间你没再看，你知道它会显示什么。<br/>"
                "这不是第一次了。<br/>"
                "你深深吸了一口气，开始回想前几次循环——"
                "每次都在周六早晨醒来，每次作业都从头开始，"
                "每次周日晚上睡着，醒来又是周六早晨。<br/>"
                "你盯着漆黑的电脑屏幕。<br/>"
                "你知道作业就算能写完，也会重来。<br/>"
                "但你不知道该做什么别的。");
            MW->showNarration(t, [=]() { node3(); });
        }
    }});

    MW->showChoices(QStringLiteral("<b>要做什么？</b>") + endingsHint(), choices);
}

// =====================================================================
// 节点 2: 首次循环 (L=0)
// =====================================================================
void SceneEngine::node2() {
    refreshHud();
    QVector<Choice> choices;
    choices.append({QStringLiteral("做作业"), [=]() {
        ST->S -= 2; ST->N += 1; ST->clampStats(); refreshHud();
        QString t;
        if (ST->N == 1) {
            t = QStringLiteral("你从头开始做大作业。<br/><i>（精神稳定值 -2）</i>");
        } else {
            t = QStringLiteral("作业进度增加。<br/><i>（精神稳定值 -2）</i>");
        }
        MW->showNarration(t, [=]() {
            if (ST->N <= 3) {
                node2();
            } else {
                ST->N = 0;
                QString tail = QStringLiteral(
                    "不知过了多久，你已经注意不到周围的任何事物，"
                    "只盯着眼前的屏幕。<br/>"
                    "终于，你觉得眼皮沉重得让你睁不开眼睛，趴在桌上睡着了。<br/><br/>"
                    "闹铃声再次响起，你发现你躺在床上。<br/>"
                    "「你今天这么早就起来了？」");
                ST->L += 1;
                ST->day = Day::Saturday;
                ST->slot = TimeSlot::Morning;
                ST->resetLoopFlags();
                refreshHud();
                if (checkBadEndings()) return;
                MW->showNarration(tail, [=]() { node1(); });
            }
        });
    }});
    MW->showChoices(QStringLiteral("<b>要做什么？</b>"), choices);
}

// =====================================================================
// 节点 3: 意识循环 (L >= 1)
// =====================================================================
void SceneEngine::node3() {
    ST->sceneCheckpoint = QStringLiteral("node3");
    refreshHud();
    QVector<Choice> choices;

    choices.append({QStringLiteral("继续做作业"), [=]() {
        QString t = QStringLiteral(
            "你按下开机键。<br/>"
            "熟悉的桌面，熟悉的空白文件夹。<br/>"
            "你把手放上键盘。写到一半停了一下——有些内容，和上一次一模一样。<br/>"
            "你继续写。写完，保存，提交，合上电脑。<br/>"
            "你坐在椅子上，等到眼皮沉了，趴在桌上。<br/>"
            "闹铃声再次响起。<br/>"
            "「你今天这么早就起来了？」<br/>"
            "周六，06:33。<br/>"
            "你看着手机，觉得这个数字变得很轻，轻得像一句无关紧要的话。<br/>"
            "<i>（精神稳定值 -2）</i>");
        ST->S -= 2; ST->N += 1; ST->clampStats(); refreshHud();
        MW->showNarration(t, [=]() {
            if (ST->N > 19) {
                triggerEndingB1();
                return;
            }
            ST->L += 1;
            ST->day = Day::Saturday;
            ST->slot = TimeSlot::Morning;
            ST->resetLoopFlags();
            refreshHud();
            if (checkBadEndings()) return;
            node3();
        });
    }});

    choices.append({QStringLiteral("探索真相"), [=]() {
        QString t = QStringLiteral(
            "如果这真是一个循环——有没有可能，清醒的只有你一个人？"
            "有没有可能，循环本身，是可以被打破的？<br/>"
            "你决定探索真相，去看看这个循环中的校园。<br/>"
            "<i>（解锁全部校园场景：宿舍、食堂、公教楼、图书馆、操场、到处看看）</i>");
        MW->showNarration(t, [=]() { campusHub(); });
    }});

    MW->showChoices(QStringLiteral("<b>要做什么？</b>"), choices);
}

// =====================================================================
// 校园探索主界面
// =====================================================================
void SceneEngine::mainCampusPrompt(const QString& extraText) {
    QString head = QStringLiteral("<b>要去哪里？</b>");
    if (!extraText.isEmpty()) head = extraText + QStringLiteral("<br/><br/>") + head;
    head += endingsHint();
    QVector<Choice> choices;
    choices.append({QStringLiteral("1. 宿舍"),     [=]() { dormScene(); }});
    choices.append({QStringLiteral("2. 食堂"),     [=]() { canteenScene(); }});
    choices.append({QStringLiteral("3. 公教楼"),   [=]() { teachingScene(); }});
    choices.append({QStringLiteral("4. 图书馆"),   [=]() { libraryScene(); }});
    choices.append({QStringLiteral("5. 操场"),     [=]() { playgroundScene(); }});
    choices.append({QStringLiteral("6. 到处看看"), [=]() { wanderScene(); }});
    if (ST->hasItem(QStringLiteral("心愿卡"))) {
        choices.append({QStringLiteral("7. 大通琴房"), [=]() { pianoScene(); }});
    }
    MW->showChoices(head, choices);
}

void SceneEngine::campusHub() {
    ST->sceneCheckpoint = QStringLiteral("campusHub");
    refreshHud();
    if (checkBadEndings()) return;
    // 夜晚不在宿舍 - 询问回宿舍 (仅当本晚还没做过选择)
    if (ST->slot == TimeSlot::Night && !ST->inDormTonight
        && !ST->nightChoiceMadeThisLoop) {
        askReturnToDormIfNight([this](){ campusHub(); });
        return;
    }
    mainCampusPrompt();
}

// =====================================================================
// 时间/夜晚处理
// =====================================================================
void SceneEngine::askReturnToDormIfNight(std::function<void()> next) {
    // 如果本晚已经做过选择(选了"否"继续在外), 不再询问, 直接进入下一步
    if (ST->nightChoiceMadeThisLoop && !ST->inDormTonight) {
        if (next) next();
        return;
    }
    // 如果当前已经在宿舍里(inDormTonight=true), 也不再询问
    if (ST->inDormTonight) {
        if (next) next();
        return;
    }
    QVector<Choice> choices;
    choices.append({QStringLiteral("是，回宿舍"), [=]() {
        ST->inDormTonight = true;
        ST->nightChoiceMadeThisLoop = true;
        dormScene();
    }});
    choices.append({QStringLiteral("否，继续在外"), [=]() {
        ST->inDormTonight = false;
        ST->nightChoiceMadeThisLoop = true;
        // 立即结算: 体力 -10, 精神稳定 -5;
        // 不再依赖 sleptOutsidePenalty 在次日早晨扣分, 避免漏提示
        ST->E -= 10;
        ST->S -= 5;
        ST->sleptOutsidePenalty = false;
        ST->clampStats();
        refreshHud();
        MW->showNarration(
            QStringLiteral("<i>（体力值 -10，精神稳定值 -5）</i>"),
            [=]() { if (next) next(); });
    }});
    if (ST->Q > 15) {
        choices.append({QStringLiteral("（翻窗）我自己想办法过来"), [=]() {
            ST->inDormTonight = true;
            ST->nightChoiceMadeThisLoop = true;
            ST->flippedWindowOut = true;
            MW->showNarration(QStringLiteral("夜风穿过窗缝。你扒住窗台借力，一翻而过——"
                                             "脚步落地的声音被夜色一并吞没了。"),
                              [=]() { dormScene(); });
        }});
    }
    MW->showChoices(QStringLiteral("【是否选择回宿舍？】"), choices);
}

void SceneEngine::postDaytimeAction(std::function<void()> next) {
    if (ST->slot == TimeSlot::Night) {
        // 已经是夜晚还做了一项活动 -> 在外面熬过去 (体力 -10, 精神 -5; 不再显示长段叙述)
        ST->E -= 10; ST->S -= 5; ST->clampStats(); refreshHud();
        enterNextDayMorning();
        return;
    }
    ST->advanceTime();
    refreshHud();
    if (ST->slot == TimeSlot::Night) {
        // 刚刚进入夜晚 —— 询问回宿舍 (无论 inDormTonight 标志)
        // 因为玩家显然在校园场景而不是宿舍里
        ST->inDormTonight = false; // 重置，需要重新选择
        askReturnToDormIfNight([this, next]() {
            if (next) next();
        });
        return;
    }
    if (next) next();
}

void SceneEngine::enterNextDayMorning() {
    bool wasSunday = (ST->day == Day::Sunday);
    // 注: 选 "否, 继续在外" 时已经立刻扣除 E-10 / S-5 并提示玩家,
    // 不再在这里二次扣分。 sleptOutsidePenalty 保留作为状态标记。
    ST->sleptOutsidePenalty = false;
    if (wasSunday) {
        endLoopAndRespawn();
    } else {
        ST->day = Day::Sunday;
        ST->slot = TimeSlot::Morning;
        ST->inDormTonight = true;
        ST->nightChoiceMadeThisLoop = false;
        ST->clampStats();
        refreshHud();
        if (checkBadEndings()) return;
        campusHub();
    }
}

void SceneEngine::endLoopAndRespawn() {
    ST->L += 1;
    ST->day = Day::Saturday;
    ST->slot = TimeSlot::Morning;
    ST->resetLoopFlags();
    ST->tasksCompletedThisLoop.clear();
    refreshHud();
    if (checkBadEndings()) return;
    // 跳过"闹铃响了... 周六 06:33"的提示, 已经循环过的玩家直接进入 探索真相 hub
    // (L=0 走不到这里, 此时 L>=1; node3 第二项就是 "探索真相", 我们直接进入校园 hub)
    campusHub();
}

// =====================================================================
// 结束条件检查
// =====================================================================
bool SceneEngine::checkBadEndings() {
    if (ST->E <= 0 || ST->M > 7) {
        triggerEndingB2();
        return true;
    }
    if (ST->L > 52) {
        triggerEndingB2();
        return true;
    }
    return false;
}

// =====================================================================
// 宿舍
// =====================================================================
void SceneEngine::dormScene() {
    ST->sceneCheckpoint = QStringLiteral("dormScene");
    refreshHud();
    if (checkBadEndings()) return;

    const QString sceneText = QStringLiteral(
        "<b>宿舍</b><br/>你的宿舍。窗帘半掩，桌前是你熟悉的电脑。") + endingsHint();

    QVector<Choice> choices;
    choices.append({QStringLiteral("0. 桌前"),  [=]() { dormDesk(); }});
    choices.append({QStringLiteral("1. 做作业"),[=]() {
        ST->S -= 2; ST->N += 1; ST->clampStats(); refreshHud();
        MW->showNarration(QStringLiteral("作业进度增加。<br/><i>（精神稳定值 -2）</i>"),
                          [=]() {
            if (ST->N > 19) { triggerEndingB1(); return; }
            postDaytimeAction([this](){ dormScene(); });
        });
    }});
    choices.append({QStringLiteral("2. 睡觉"),  [=]() { dormSleep(); }});
    choices.append({QStringLiteral("3. 室友A"), [=]() { dormRoommate(1); }});
    choices.append({QStringLiteral("4. 室友B"), [=]() { dormRoommate(2); }});
    choices.append({QStringLiteral("5. 室友C"), [=]() { dormRoommate(3); }});

    // 离开宿舍
    if (!(ST->slot == TimeSlot::Night && ST->inDormTonight && ST->Q <= 15 && !ST->flippedWindowOut)) {
        choices.append({QStringLiteral("← 离开宿舍"), [=]() {
            if (ST->slot == TimeSlot::Night && ST->Q > 15) {
                ST->flippedWindowOut = true;
                ST->inDormTonight = false;
                MW->showNarration(QStringLiteral("你撑住窗框翻身出去。夜风冷得让人清醒。"),
                                  [=]() { mainCampusPrompt(); });
            } else if (ST->slot == TimeSlot::Night) {
                MW->showNarration(QStringLiteral("夜已深了，外面静悄悄的。<br/>"
                                                 "你不太想再出去——除非身体素质允许你翻窗。"),
                                  [=]() { dormScene(); });
            } else {
                mainCampusPrompt();
            }
        }});
    } else {
        choices.append({QStringLiteral("（夜晚，已无法离开宿舍）"), [=]() { dormScene(); }});
    }

    MW->showChoices(sceneText, choices);
}

void SceneEngine::dormDesk() {
    refreshHud();
    QVector<Choice> choices;

    choices.append({QStringLiteral("（1）学子卡"), [=]() {
        QString t = ST->carryingStudentCard
            ? QStringLiteral("学子卡此刻就在你口袋里。<br/>"
                             "<b>是否携带学子卡？</b>")
            : QStringLiteral("桌上摆着你的学子卡。<br/><b>是否携带学子卡？</b>");
        QVector<Choice> sub;
        sub.append({QStringLiteral("是"), [=]() {
            ST->carryingStudentCard = true;
            ST->addItem(QStringLiteral("学子卡"));
            refreshHud();
            MW->showNarration(QStringLiteral("你把学子卡放进口袋。"),
                              [=]() { dormDesk(); });
        }});
        sub.append({QStringLiteral("否"), [=]() {
            ST->carryingStudentCard = false;
            ST->removeItem(QStringLiteral("学子卡"));
            refreshHud();
            MW->showNarration(QStringLiteral("你把学子卡留在桌上。"),
                              [=]() { dormDesk(); });
        }});
        MW->showChoices(t, sub);
    }});

    // 发帖 (拥有钢琴录音后)
    if (ST->hasItem(QStringLiteral("一段钢琴录音")) && !ST->publishedRecording) {
        choices.append({QStringLiteral("（2）在社交媒体上发帖"), [=]() {
            ST->removeItem(QStringLiteral("一段钢琴录音"));
            ST->publishedRecording = true;
            ST->everPublishedRecording = true;
            QString t = QStringLiteral(
                "你打开平台，把录音传了上去。配文来来回回修改了几遍，最后只留了一句——<br/>"
                "「海棠树下捡到一张心愿卡。卡片上说想听一次琴房的声音，替你弹过了。」<br/>"
                "你将定位设置在学校，发送。<br/>"
                "<i>【道具：一段钢琴录音 移除】</i>");
            MW->showNarration(t, [=]() { dormDesk(); });
        }});
    }

    // 手机 (发帖之后)
    if (ST->publishedRecording && !ST->gotReplyMessage) {
        choices.append({QStringLiteral("（3）手机"), [=]() {
            ST->gotReplyMessage = true;
            ST->everReceivedReply = true;
            QString t = QStringLiteral(
                "手机亮了，是一条私信。<br/>"
                "「你好，我就是那张心愿卡的主人。学妹把帖子转给我了，录音我听了好几遍。」<br/>"
                "「以前总觉得还有时间，想着忙完就去琴房看看。后来毕业了，"
                "以为这件事早就过去了。没想到隔了这么久，有人替我在里面弹了一首曲子。」<br/>"
                "「谢谢你。那间琴房的声音，原来是这样。」<br/>"
                "你低头，回复她：<br/>"
                "「也谢谢你，那首曲子，也是弹给我自己的。」<br/>"
                "你想起那些等了很久的事，有些已经过去了，有些还在。<br/>"
                "但这次，至少有一件事，没有再等下去。<br/>"
                "<i>（精神稳定值 +10）</i><br/>"
                "<i>（任务完成：琴房的心愿卡）</i>");
            ST->S += 10; ST->clampStats();
            ST->completeTask(QStringLiteral("琴房的心愿卡"));
            refreshHud();
            MW->showNarration(t, [=]() { dormDesk(); });
        }});
    }

    // 空白纸 (永久, 只看一次)
    if (!ST->blankPaperTaken) {
        choices.append({QStringLiteral("（4）一张空白的纸"), [=]() {
            ST->blankPaperTaken = true;
            ST->worldCracks.insert(QStringLiteral("一张空白的纸"));
            QString t = QStringLiteral(
                "你坐到桌前，桌角的书压着一张空白的纸，但你不记得是什么时候放在这里的。<br/>"
                "你将它拿出来。<br/>"
                "不，不是完全空白。在纸张最底端，有一行被橡皮擦得快要看不见的铅笔字：<br/>"
                "「我在做一个项目，但我写不到结局。」<br/>"
                "你心跳快了一拍，不是因为害怕——是因为笔迹。<br/>"
                "这句话的笔迹和你的一模一样。<br/>"
                "你没有写过这句话，但你觉得那不是别人模仿你的笔迹写来骗你的——"
                "你就是觉得，那是你自己写下的，只是你不记得写过。<br/>"
                "你放下纸，手指有点凉。<br/>"
                "你把纸对折，想收起来。在折叠的一瞬间，纸面上浮现出一行新的铅笔字，"
                "像有人正坐在你看不见的地方写着：<br/>"
                "「你会怎么写？」<br/>"
                "字迹仍然是你的。<br/>"
                "你还来不及反应，那行字已经淡去，纸面恢复了空白。<br/>"
                "<i>【获得世界裂缝：一张空白的纸】</i>");
            MW->showNarration(t, [=]() { dormDesk(); });
        }});
    }

    // 已解锁的结局入口 (夜晚禁止进入, 与顶部按钮策略一致)
    if (!ST->unlockedEndings.isEmpty() && ST->slot != TimeSlot::Night) {
        const QStringList endings = ST->unlockedEndings.values();
        for (const QString& name : endings) {
            const QString endingName = name;
            choices.append({QStringLiteral("进入结局：【%1】").arg(endingName), [=]() {
                QVector<Choice> sub;
                sub.append({QStringLiteral("是 - 进入结局：%1").arg(endingName), [=]() {
                    enterEndingByName(endingName);
                }});
                sub.append({QStringLiteral("再想想"), [=]() { dormDesk(); }});
                MW->showChoices(
                    QStringLiteral("你坐在桌前，目光落在那张写过又被你折起的纸上。<br/>"
                                   "<b>是否进入结局：%1？</b>").arg(endingName),
                    sub);
            }});
        }
    }

    choices.append({QStringLiteral("← 返回宿舍"), [=]() { dormScene(); }});

    QString deskHeader = QStringLiteral("<b>桌前</b><br/>桌上放着你的一些物品。");
    if (!ST->unlockedEndings.isEmpty() && ST->slot != TimeSlot::Night) {
        deskHeader += QStringLiteral(
            "<br/><span style='color:#ffd97a;'>"
            "（你感到桌前的空气有些异样——已解锁的结局入口似乎可以从这里进入。）"
            "</span>");
    } else if (!ST->unlockedEndings.isEmpty()) {
        deskHeader += QStringLiteral(
            "<br/><span style='color:#cfcfcf;'>"
            "（夜里没办法静下心来做决定，等明天白天再来桌前吧。）"
            "</span>");
    }
    MW->showChoices(deskHeader, choices);
}

void SceneEngine::dormSleep() {
    if (ST->slot != TimeSlot::Night) {
        QString t = QStringLiteral(
            "你躺在柔软的枕头上，闭上眼。<br/><b>要小憩一会儿吗？</b>");
        QVector<Choice> choices;
        choices.append({QStringLiteral("（1）小憩片刻"), [=]() {
            ST->E += 10; ST->S += 5; ST->clampStats();
            QString t2 = QStringLiteral(
                "你小憩了一会儿，闭眼像是只过了一瞬，醒来却觉得精神了不少。<br/>"
                "<i>（体力值 +10  精神稳定值 +5）</i>");
            refreshHud();
            MW->showNarration(t2, [=]() {
                postDaytimeAction([this](){ dormScene(); });
            });
        }});
        choices.append({QStringLiteral("（2）算了，不睡了"), [=]() {
            MW->showNarration(QStringLiteral("你躺了几分钟，又坐了起来。还是不睡了，做点别的吧。"),
                              [=]() { dormScene(); });
        }});
        MW->showChoices(t, choices);
    } else {
        dormSleepNight();
    }
}

void SceneEngine::dormSleepNight() {
    QString t = QStringLiteral("你躺在柔软的枕头上，闭上眼。");
    QVector<Choice> choices;
    choices.append({QStringLiteral("（1）好好睡一觉"), [=]() {
        // 检查 AAA 触发条件: 有 陈旧的作业纸 和 科幻小说书摘 且 周日夜晚
        // 注意: AAA 仅触发一次, 之后(已解锁 今日·解铃)直接走 hiddenCond 检查,
        // 否则 hidden 结局永远被 AAA 拦截
        bool aaaCond = !ST->storyAAA_done
            && ST->hasItem(QStringLiteral("陈旧的作业纸"))
            && ST->hasItem(QStringLiteral("科幻小说书摘"))
            && ST->day == Day::Sunday;
        if (aaaCond) {
            QString t2 = QStringLiteral(
                "你梦到了小时候的自己。<br/>"
                "旧书桌上，摊着一份只差最后一段的作文。"
                "小小的你握着笔，笔尖悬在纸面上，就是落不下去。<br/>"
                "你问她：「为什么不写完呢？」<br/>"
                "她没抬头，声音闷闷的：「我怕写不好。"
                "总觉得哪里还差一点，我想尽可能好地写完。」<br/>"
                "「可你已经写了很久了，万一过了提交的时间呢？」<br/>"
                "她没说话，只是用笔在草稿纸上反复描着同一行字，描到纸快要破了。<br/>"
                "你看着她，忽然分不清那究竟是认真，还是害怕。<br/>"
                "「如果不交的话，」你蹲下来，平视她的眼睛，"
                "「前面的时间就白费了啊。」<br/>"
                "她终于停下笔，抬起头，用和你一样的眼睛看着你。<br/>"
                "「所以，你现在明白了吗？」<br/>"
                "你猛然睁开眼。");
            ST->storyAAA_done = true;
            MW->showNarration(t2, [=]() {
                // 是否进入结局 今日·解铃
                QVector<Choice> ec;
                ec.append({QStringLiteral("是 - 进入结局：今日·解铃"), [=]() {
                    triggerEndingT2();
                }});
                ec.append({QStringLiteral("否 - 继续"), [=]() {
                    ST->unlockEnding(QStringLiteral("今日·解铃"));
                    refreshHud();
                    // 结算
                    ST->E += 20; ST->S += 5; ST->clampStats();
                    QString notes;
                    if (ST->E == 100) notes += QStringLiteral("<br/>体力值已满");
                    if (ST->S == 100) notes += QStringLiteral("<br/>精神稳定值已满");
                    MW->showNarration(QStringLiteral(
                                          "你不想这样结束。你想再走走。<br/>"
                                          "<i>（体力值 +20，精神稳定值 +5）</i>%1<br/>"
                                          "<i>（结局入口【今日·解铃】已永久解锁。"
                                          "之后想进入这一结局时，"
                                          "可在【宿舍 → 桌前】点击进入。）</i>")
                                          .arg(notes),
                                      [=]() { enterNextDayMorning(); });
                }});
                MW->showChoices(QStringLiteral("【是否进入结局：今日·解铃】"), ec);
            });
            return;
        }
        // 检查隐藏结局触发条件
        const bool hiddenCond =
            ST->day == Day::Sunday &&
            ST->unlockedEndings.contains(QStringLiteral("羁绊·星火")) &&
            ST->unlockedEndings.contains(QStringLiteral("今日·解铃")) &&
            ST->worldCracks.size() >= 3 &&
            ST->worldCracks.contains(QStringLiteral("公教楼连廊的人影")) &&
            ST->worldCracks.contains(QStringLiteral("一张空白的纸")) &&
            ST->worldCracks.contains(QStringLiteral("阅览桌上的测试记录"));
        if (hiddenCond) {
            QString hint = QStringLiteral(
                "三个碎片在你脑海里轻轻碰撞，拼出一条模糊的线。<br/>"
                "你隐约感觉到——有什么在等你，在一个你还没去过的地方。");
            QVector<Choice> cc;
            cc.append({QStringLiteral("去马蹄湖"), [=]() {
                // 隐藏结局序章
                QString t3 = QStringLiteral(
                    "你来到马蹄湖边。<br/>"
                    "夜景褪了色。柳树、湖水、路灯，都像一幅只勾了线的草图。"
                    "灰白的轮廓，没有深浅。<br/>"
                    "只有你是有颜色的。<br/>"
                    "湖边亭下，一台笔记本电脑亮着，屏幕光照亮了周围一小圈地面。<br/>"
                    "你走过去，屏幕上是打开的项目文档，标题是：<br/>"
                    "<b>《以南开为背景的交互叙事程序——献给在这里活过的我们》</b><br/>"
                    "署名是你的名字。<br/>"
                    "你往下读。<br/>"
                    "是她写的。海棠树下的红笺，窗台的作业纸，橘猫，手帕，"
                    "都在里面。每一个你遇见过、帮助过的人，都被她写成了这个世界里的光。<br/>"
                    "进度条卡在 90%。最后一段下面，批注密密麻麻：<br/>"
                    "「这里的结局怎么办。」<br/>"
                    "「我不想让它结束。」<br/>"
                    "「海棠、琴房、橘猫……我把能想起来的东西都写进去了，"
                    "可越写越多，好像永远写不完。」<br/>"
                    "「室友说，毕业而已，又不是不会回来。"
                    "但等真的离开那天——这些日常就不再是「我的日常」了。」<br/>"
                    "「如果这个程序完成了，就是承认，我的大学真的结束了。」<br/>"
                    "最后一句话，光标在它末尾一闪一闪：<br/>"
                    "「如果这个世界里的我能活过来……她会比我更知道，该怎么告别吗？」<br/>"
                    "你伸手，指尖碰到键盘。");
                MW->showNarration(t3, [=]() {
                    QVector<Choice> hc;
                    hc.append({QStringLiteral("1. 敲下结局"), [=]() { triggerHiddenEnding(1); }});
                    hc.append({QStringLiteral("2. 写下留言"), [=]() { triggerHiddenEnding(2); }});
                    MW->showChoices(QStringLiteral("<b>你打开了一个空白的对话框……</b>"), hc);
                });
            }});
            MW->showNarration(hint, [=]() {
                MW->showChoices(QStringLiteral("你站在宿舍门口，似乎被什么牵引着……"), cc);
            });
            return;
        }
        // 普通睡眠
        ST->E += 20; ST->S += 5; ST->clampStats();
        QString notes;
        if (ST->E == 100) notes += QStringLiteral("<br/>体力值已满");
        if (ST->S == 100) notes += QStringLiteral("<br/>精神稳定值已满");
        refreshHud();
        QString t2 = QStringLiteral(
            "你沉沉睡去，醒来觉得被窝暖暖的。<br/>"
            "<i>（体力值 +20，精神稳定值 +5）</i>%1").arg(notes);
        MW->showNarration(t2, [=]() { enterNextDayMorning(); });
    }});
    choices.append({QStringLiteral("（2）再撑一会儿"), [=]() {
        MW->showNarration(QStringLiteral(
                              "你还不想睡。还有些事情想做，还有些思绪没理清。<br/>"
                              "你坐起身，打开台灯。灯光在桌面上铺出一小圈暖黄。"),
                          [=]() { dormScene(); });
    }});
    MW->showChoices(t, choices);
}

void SceneEngine::dormRoommate(int which) {
    QString name;
    switch (which) {
        case 1: name = QStringLiteral("室友A"); break;
        case 2: name = QStringLiteral("室友B"); break;
        default: name = QStringLiteral("室友C"); break;
    }
    // 早晨或夜晚：在睡觉
    if (ST->slot == TimeSlot::Morning || ST->slot == TimeSlot::Night) {
        MW->showNarration(QStringLiteral("【%1】她在睡觉。").arg(name),
                          [=]() { dormScene(); });
        return;
    }
    QVector<Choice> choices;
    choices.append({QStringLiteral("（1）闲聊"), [=]() {
        ST->E -= 5; ST->S += 5; ST->clampStats(); refreshHud();
        MW->showNarration(QStringLiteral("你和【%1】愉快地聊了一会儿。<br/>"
                                         "<i>（体力值 -5，精神稳定值 +5）</i>").arg(name),
                          [=]() {
                              postDaytimeAction([this](){ dormScene(); });
                          });
    }});
    if (which == 1 && !ST->tryProbeRoommateDone) {
        choices.append({QStringLiteral("（2）试探循环"), [=]() {
            ST->tryProbeRoommateDone = true;
            if (ST->K > 10 && ST->L >= 4) {
                QString t = QStringLiteral(
                    "你看着她，犹豫了一下，还是开口了。<br/>"
                    "「你有没有觉得……这个周末一直在重复？」<br/>"
                    "她停下手里的事，看着你。没有笑，没有说你在开玩笑。<br/>"
                    "「我做过那种梦，」她说，「同一个周末，过了又过。"
                    "但你这么一问……好像不只是梦。」<br/>"
                    "她皱起眉，像在努力抓住什么，又抓不住。<br/>"
                    "「说起来，今天早上你说那句话的时候，我就觉得在哪里听过。"
                    "一模一样的语气。」<br/>"
                    "你们沉默了几秒。<br/>"
                    "「如果你真的被困住了，」她最后说，「也许不该一个人待着。"
                    "出去走走，或者找别人问问，也许有人会记得点什么。」<br/>"
                    "她说完低下头，好像刚才那段话花掉了她很大的力气。<br/>"
                    "<i>（体力值 -5，知识 +3）</i>");
                ST->E -= 5; ST->K += 3; ST->clampStats(); refreshHud();
                MW->showNarration(t, [=]() { postDaytimeAction([this](){ dormScene(); }); });
            } else {
                QString t = QStringLiteral(
                    "「你有没有觉得……最近有点奇怪？」<br/>"
                    "她抬头看你：「什么？」<br/>"
                    "你张了张嘴，又把话咽了回去。<br/>"
                    "「没什么。可能是我没睡好。」<br/>"
                    "她点点头，没追问什么。但你总觉得她多看了你一眼。<br/>"
                    "<i>（体力值 -5，知识 +1）</i>");
                ST->E -= 5; ST->K += 1; ST->clampStats(); refreshHud();
                MW->showNarration(t, [=]() { postDaytimeAction([this](){ dormScene(); }); });
            }
        }});
    }
    choices.append({QStringLiteral("← 返回宿舍"), [=]() { dormScene(); }});
    MW->showChoices(QStringLiteral("<b>%1</b> 抬头看了你一眼。").arg(name), choices);
}

// =====================================================================
// 食堂
// =====================================================================
void SceneEngine::canteenScene() {
    ST->sceneCheckpoint = QStringLiteral("canteenScene");
    refreshHud();
    if (checkBadEndings()) return;
    if (ST->slot == TimeSlot::Night && !ST->inDormTonight
        && !ST->nightChoiceMadeThisLoop) {
        askReturnToDormIfNight([this](){ canteenScene(); });
        return;
    }
    QVector<Choice> choices;
    bool canEat = (ST->slot == TimeSlot::Noon || ST->slot == TimeSlot::Evening);
    if (canEat) choices.append({QStringLiteral("1. 吃饭"), [=]() { canteenEat(); }});
    if (ST->canteenAuntUnlocked) {
        choices.append({QStringLiteral("2. 食堂阿姨"), [=]() { canteenAunt(); }});
    }
    choices.append({QStringLiteral("← 离开食堂"), [=]() { campusHub(); }});
    QString hint = canEat
        ? QString()
        : QStringLiteral("<br/><i>（食堂只在中午、傍晚开放窗口。）</i>");
    MW->showChoices(
        QStringLiteral("<b>食堂</b><br/>理科食堂。蒸汽腾腾，人声鼎沸。%1").arg(hint) + endingsHint(),
        choices);
}

void SceneEngine::canteenEat() {
    // 概率激活食堂阿姨
    bool apronTrigger = chance(30) && !ST->canteenAuntUnlocked;
    if (apronTrigger) {
        ST->canteenAuntUnlocked = true;
    }

    // 剧情 A/B/C
    QString flow;
    if (ST->L >= 5 && ST->canteenBDone && !ST->canteenCDone && chance(30)) {
        ST->canteenCDone = true;
        flow = QStringLiteral(
            "你再次走到那个窗口，大叔仍说着你的家乡话。<br/>"
            "「同学，我们是老乡，你又来了这么多次，今天我请你吃个煎蛋。」<br/>"
            "你愣住了，手悬在半空，忘了去接那碗面。<br/>"
            "你清晰地记得——你只在前面的某一次循环中，对他说过方言。<br/>"
            "他是怎么记得的？<br/>脑子里很乱。<br/>"
            "直到后面的同学出声提醒，你才回过神来，"
            "接过那碗热气腾腾的面。上面放着一个煎蛋，边缘煎得微焦。<br/>");
    } else if (ST->L >= 5 && ST->canteenA && !ST->canteenBDone && chance(30)) {
        ST->canteenBDone = true;
        flow = QStringLiteral(
            "你像之前一样走到那个窗口。大叔仍说着你的家乡话。<br/>"
            "这次，你也不由自主地用方言回应了他。<br/>"
            "他听到后笑了笑，没有多说什么。"
            "但你接过面的时候，觉得那碗比平时的满了一点。<br/>");
    } else if (chance(30) && !ST->canteenA) {
        ST->canteenA = true;
        flow = QStringLiteral(
            "你走到一个窗口前，点了一碗面。<br/>"
            "窗口后面的大叔说着你的家乡话，将热气腾腾的面递给了你。<br/>");
    }
    QString prefix = QStringLiteral(
        "你在食堂三层楼之间闲逛，想找到一些好吃的窗口。<br/>");
    if (apronTrigger) {
        prefix += QStringLiteral(
            "你看到一位正在擦拭台面的食堂阿姨，"
            "衣服口袋里露出一截浅蓝色的布料。<br/>");
    }
    QString suffix = QStringLiteral(
        "你找到座位坐下，开始吃饭。<br/>"
        "<i>（体力值 +5，精神稳定值 +5）</i>");
    ST->E += 5; ST->S += 5; ST->clampStats(); refreshHud();
    QString full = prefix + flow + QStringLiteral("<br/>") + suffix;
    MW->showNarration(full, [=]() {
        postDaytimeAction([this](){ canteenScene(); });
    });
}

void SceneEngine::canteenAunt() {
    if (ST->L < 8) {
        // 情况一/二 (L<8): 本循环第一次/非第一次
        if (!ST->canteenAuntTalkedThisLoop) {
            ST->canteenAuntTalkedThisLoop = true;
            QString t = QStringLiteral(
                "你走到食堂阿姨附近拿餐具，无意间看到，"
                "那浅蓝色的布料是手帕的一角，针脚略显笨拙。<br/>"
                "你开口问了一句。<br/>"
                "阿姨低头看看，把手帕拿出来，笑了。<br/>"
                "「我女儿去年给我绣的生日礼物，手艺挺好的，"
                "不过更重要的是她的心意。」<br/>"
                "她把手帕叠好放回口袋，动作很轻。"
                "你看到手帕边角绣了个小小的爱心。<br/>"
                "<i>（体力值 -5，知识 +2）</i><br/>"
                "<i>【获得信息：阿姨的手帕】</i>");
            ST->E -= 5; ST->K += 2; ST->clampStats();
            refreshHud();
            MW->showNarration(t, [=]() {
                postDaytimeAction([this](){ canteenScene(); });
            });
        } else {
            QString t = QStringLiteral(
                "阿姨正忙着擦拭台面，口袋里仍好好放着那条浅蓝色的手帕。<br/>"
                "<i>（体力值 -5）</i>");
            ST->E -= 5; ST->clampStats(); refreshHud();
            MW->showNarration(t, [=]() {
                postDaytimeAction([this](){ canteenScene(); });
            });
        }
        return;
    }
    // L >= 8
    bool hasHandkerchief = ST->hasItem(QStringLiteral("阿姨的手帕"));
    bool task111ThisLoop = ST->canteenAuntStory111;
    bool taskCompletedEver = ST->everCompletedHandkerchief;
    bool taskCompletedThisLoop =
        ST->tasksCompletedThisLoop.contains(QStringLiteral("食堂阿姨的手帕"));

    // 情况七: 该循环内完成过任务
    if (taskCompletedThisLoop) {
        QString t = QStringLiteral(
            "你走进食堂时，阿姨就远远看见了你。"
            "她放下手里的抹布，朝你招了招手。<br/>"
            "等你走近了，她从口袋里掏出一个苹果，递给你。"
            "苹果皮上还带着后厨水槽里洗过的凉意。<br/>"
            "「干净的，给你。」<br/>"
            "你接过来。她笑了笑，拿起抹布继续忙了。"
            "那条浅蓝色手帕放在她另一边口袋里，叠得整整齐齐，露出边角那颗小小的爱心。<br/>"
            "<i>（体力值 +5，精神稳定值 +5）</i>");
        ST->E += 5; ST->S += 5; ST->clampStats(); refreshHud();
        MW->showNarration(t, [=]() {
            postDaytimeAction([this](){ canteenScene(); });
        });
        return;
    }
    // 情况八: 之前的循环完成过
    if (taskCompletedEver && !taskCompletedThisLoop && !hasHandkerchief) {
        QString t = QStringLiteral(
            "你走过去拿餐具，阿姨正低头擦拭台面。<br/>"
            "那条浅蓝色手帕放在她的口袋里，叠得整整齐齐，露出边角那颗小小的爱心。<br/>"
            "她抬起头看见你，笑了笑，点了点头——就像对每一个来食堂的学生那样。<br/>"
            "她没认出你。你也没说什么。<br/>"
            "但你路过她身边时，她叫住了你。<br/>"
            "「哎，同学。」<br/>"
            "你回头。她从另一边口袋里掏了掏，什么也没掏出来。"
            "她愣了愣，自己也觉得奇怪，摆摆手说没事。<br/>"
            "你不确定她还记不记得什么。但她叫住你的那个动作，"
            "很像那个口袋里曾经放过什么东西。<br/>"
            "<i>（体力值 -5，精神稳定值 +5）</i>");
        ST->E -= 5; ST->S += 5; ST->clampStats(); refreshHud();
        MW->showNarration(t, [=]() {
            postDaytimeAction([this](){ canteenScene(); });
        });
        return;
    }
    // 情况五/六: 持有手帕 -> 完成任务
    if (hasHandkerchief) {
        QString prefix = task111ThisLoop
            ? QStringLiteral(
                "你走到阿姨面前，将那条浅蓝色的手帕交给她。<br/>"
                "她愣住了，手悬在半空，像是怕碰碎什么东西。<br/>"
                "「同学，你在哪儿找到的？」她的声音有点发颤。<br/>"
                "你说了地点。她接过手帕，指腹慢慢抚过那颗小小的爱心。")
            : QStringLiteral(
                "你走到阿姨面前，将那条浅蓝色的手帕交给她。<br/>"
                "她愣了愣，没有马上接。<br/>"
                "「这是……」她低头看了好一会儿，才伸手接过去，"
                "「同学，你怎么知道是我的？」<br/>"
                "你说，之前在食堂吃饭时见过她口袋里露出来一截。<br/>"
                "她恍然地点点头，没再追问。<br/>"
                "她接过手帕，指腹慢慢抚过那颗小小的爱心。");
        QString tail = QStringLiteral(
            "线有点松了，颜色也洗淡了，但她看着它的眼神，"
            "像在看一个走了很远的路、终于回到家的人。<br/>"
            "「我女儿去外地上学那年绣的。」她低着头说，"
            "「她怕我想她。怕我一个人在食堂太累。」<br/>"
            "她用手背很快地擦了一下眼角。<br/>"
            "「这几天找不到它，我连觉都睡不好。」<br/>"
            "她把你的手拉过来，使劲握了一下。"
            "她的手上有一层老茧，那是生活磨出的痕迹。<br/>"
            "「孩子，谢谢你。真的。」她看着你，眼圈微红，笑了，"
            "「往后想家了就来，阿姨给你做顿热乎的。」<br/>"
            "<i>（体力值 -5，精神稳定值 +15）</i><br/>"
            "<i>（任务完成：食堂阿姨的手帕。你好像也不那么想家了。）</i>");
        ST->E -= 5; ST->S += 15; ST->clampStats();
        ST->removeItem(QStringLiteral("阿姨的手帕"));
        ST->completeTask(QStringLiteral("食堂阿姨的手帕"));
        ST->everCompletedHandkerchief = true;
        refreshHud();
        MW->showNarration(prefix + QStringLiteral("<br/>") + tail, [=]() {
            postDaytimeAction([this](){ canteenScene(); });
        });
        return;
    }
    // 情况三: 第一次选择(无手帕, 第一次)
    if (!task111ThisLoop) {
        ST->canteenAuntStory111 = true;
        QString t = QStringLiteral(
            "这次，你没有看到阿姨口袋里那条浅蓝色手帕。<br/>"
            "你心下好奇，等到阿姨忙完后，走到她身边。<br/>"
            "听到你问她，阿姨愣了一下，手不自觉摸了摸口袋。<br/>"
            "「手帕……」她想了想，「那条浅蓝色的，昨天回去就找不到了。」<br/>"
            "她继续擦拭台面，但动作慢了下来。<br/>"
            "「也不记得放在哪了，可能是丢了吧。」<br/>"
            "你看着阿姨的背影，察觉到她语气里的落寞。<br/>"
            "<i>（体力值 -5，知识 +2，精神稳定值 -5）</i>");
        ST->E -= 5; ST->K += 2; ST->S -= 5; ST->clampStats(); refreshHud();
        MW->showNarration(t, [=]() {
            postDaytimeAction([this](){ canteenScene(); });
        });
        return;
    }
    // 情况四
    QString t = QStringLiteral(
        "阿姨还在忙，口袋里仍是空的。<br/>"
        "食堂的蒸汽升起来，模糊了她的脸。<br/>"
        "<i>（体力值 -5）</i>");
    ST->E -= 5; ST->clampStats(); refreshHud();
    MW->showNarration(t, [=]() {
        postDaytimeAction([this](){ canteenScene(); });
    });
}

// =====================================================================
// 公教楼
// =====================================================================
void SceneEngine::teachingScene() {
    ST->sceneCheckpoint = QStringLiteral("teachingScene");
    refreshHud();
    if (checkBadEndings()) return;
    if (ST->slot == TimeSlot::Night && !ST->inDormTonight
        && !ST->nightChoiceMadeThisLoop) {
        askReturnToDormIfNight([this](){ teachingScene(); });
        return;
    }
    QVector<Choice> choices;
    choices.append({QStringLiteral("1. 学习"), [=]() { teachingStudy(); }});
    choices.append({QStringLiteral("2. 探索"), [=]() { teachingExplore(); }});
    choices.append({QStringLiteral("← 离开公教楼"), [=]() { campusHub(); }});
    MW->showChoices(QStringLiteral("<b>公教楼</b><br/>公共教学楼。走廊里光斜斜地铺着。") + endingsHint(),
                    choices);
}

void SceneEngine::teachingStudy() {
    ST->K += 3; ST->clampStats(); refreshHud();
    QString base = QStringLiteral("你在公教楼认真地学习了一会儿。<br/><i>（知识 +3）</i>");
    bool bbAvailable = ST->teachingAa && ST->K > 10 && !ST->teachingBbDone;
    if (bbAvailable && chance(30)) {
        ST->teachingBbDone = true;
        ST->addItem(QStringLiteral("陈旧的作业纸"));
        ST->K += 2; ST->S -= 5; ST->clampStats();
        QString t = base + QStringLiteral(
            "<br/><br/>你学习累了，在窗户边眺望远方。<br/>"
            "忽然，一阵风吹来，又将窗台上的那张纸吹落在地。<br/>"
            "你再次将它拾起，发现它竟然是一张作业纸。<br/>"
            "纸张边缘微微卷起，字迹工整地有些刻板。"
            "在内容的最下方，有一行被用力划掉，但勉强还能辨认的字："
            "「我还是没有交，感觉它还是不够好。明天再看看吧。」<br/>"
            "旁边有人用红笔在旁边补了一句，字迹不一样，温温和和的："
            "「已经很好了，不用害怕。」<br/>"
            "像是谁写了这句话，犹豫了，划掉了。"
            "而另一个人看见了它，替它补了一个回答。<br/>"
            "不知为何，你看着这张纸，一股说不清的熟悉感涌上心头。<br/>"
            "<i>【获得道具：陈旧的作业纸】</i><br/>"
            "<i>（知识 +2，精神稳定值 -5）</i>");
        refreshHud();
        MW->showNarration(t, [=]() {
            postDaytimeAction([this](){ teachingScene(); });
        });
        return;
    }
    if (!ST->teachingAa && chance(30)) {
        ST->teachingAa = true;
        QString t = base + QStringLiteral(
            "<br/><br/>你学习累了，在窗户边眺望远方。<br/>"
            "忽然，一阵风吹来，将窗台上的一张纸吹落在地。<br/>"
            "你将它拾起，放回窗台上。");
        MW->showNarration(t, [=]() {
            postDaytimeAction([this](){ teachingScene(); });
        });
        return;
    }
    MW->showNarration(base, [=]() {
        postDaytimeAction([this](){ teachingScene(); });
    });
}

void SceneEngine::teachingExplore() {
    QString narrative = QStringLiteral(
        "你绕着公教楼周围走，留意被掩在树影里的细节。");

    // 橘猫剧情1
    bool catCond = ST->completedTasks.contains(QStringLiteral("琴房的心愿卡"))
        && !ST->catTeachingSat              // 仅在第一次 sat 之前触发
        && ST->slot == TimeSlot::Evening
        && !ST->catTeachingMet
        && !ST->catMissedThisLoop;
    if (catCond && chance(30)) {
        ST->catTeachingMet = true;
        QString t = QStringLiteral(
            "公教楼外的草地边，一只橘猫蹲在那儿。<br/>"
            "面朝西，尾巴圈着前爪。<br/>"
            "你顺着它的目光看——太阳正往砖红色的楼后面沉。");
        QVector<Choice> cc;
        cc.append({QStringLiteral("1. 坐过去"), [=]() {
            ST->catTeachingSat = true;
            ST->E -= 5; ST->S += 5; ST->clampStats(); refreshHud();
            QString t2 = QStringLiteral(
                "你在草地上坐下，它没理你。太阳往下沉了一截，风从远方吹过来。<br/>"
                "最后一线光消失的时候，它站起来，伸了个懒腰，踩着草走了。<br/>"
                "<i>（体力值 -5，精神稳定值 +5）</i>");
            MW->showNarration(t2, [=]() {
                postDaytimeAction([this](){ teachingScene(); });
            });
        }});
        cc.append({QStringLiteral("2. 离开"), [=]() {
            ST->catMissedThisLoop = true;
            ST->E -= 5; ST->clampStats(); refreshHud();
            MW->showNarration(QStringLiteral("你看了它一眼，走了。<br/>"
                                             "<i>（体力值 -5）</i>"),
                              [=]() {
                                  postDaytimeAction([this](){ teachingScene(); });
                              });
        }});
        MW->showChoices(t, cc);
        return;
    }
    // 随机发现
    if (!ST->randomFoundTeaching && chance(25)) {
        ST->randomFoundTeaching = true;
        int idx = pickWeighted({30, 30, 25, 15});
        QString itemName; QString tail;
        switch (idx) {
            case 0: itemName = QStringLiteral("一小袋梨膏糖"); tail = QStringLiteral("（可在背包中使用：体力 +10）"); break;
            case 1: itemName = QStringLiteral("一张撕下来的笔记"); tail = QStringLiteral("（可在背包中使用：知识 +3）"); break;
            case 2: itemName = QStringLiteral("半截粉笔"); tail = QStringLiteral("（无特殊效果）"); break;
            case 3: itemName = QStringLiteral("一枚一元硬币"); tail = QStringLiteral("（无特殊效果）"); break;
        }
        ST->addItem(itemName);
        refreshHud();
        MW->showNarration(narrative + QStringLiteral("<br/>"
                              "你在草地边的台阶缝里发现了一件小东西。<br/>"
                              "<i>【获得：%1】%2</i>")
                              .arg(itemName, tail),
                          [=]() {
                              postDaytimeAction([this](){ teachingScene(); });
                          });
        return;
    }
    MW->showNarration(narrative + QStringLiteral("<br/>你转了一圈，没什么特别的发现。"),
                      [=]() {
                          postDaytimeAction([this](){ teachingScene(); });
                      });
}

// =====================================================================
// 图书馆
// =====================================================================
void SceneEngine::libraryScene() {
    ST->sceneCheckpoint = QStringLiteral("libraryScene");
    refreshHud();
    if (checkBadEndings()) return;
    if (ST->slot == TimeSlot::Night && !ST->inDormTonight
        && !ST->nightChoiceMadeThisLoop) {
        askReturnToDormIfNight([this](){ libraryScene(); });
        return;
    }
    QVector<Choice> choices;
    choices.append({QStringLiteral("1. 学习"), [=]() { libraryStudy(); }});
    choices.append({QStringLiteral("2. 探索"), [=]() { libraryExplore(); }});
    choices.append({QStringLiteral("← 离开图书馆"), [=]() { campusHub(); }});
    MW->showChoices(QStringLiteral("<b>图书馆</b><br/>津南图书馆。书页翻动的声音轻轻浮在空气里。") + endingsHint(),
                    choices);
}

void SceneEngine::libraryStudy() {
    // 学习随机事件
    int roll = QRandomGenerator::global()->bounded(100);
    QString text;
    if (roll < 30) {
        text = QStringLiteral(
            "你翻开书本，原本只是打算看一会儿，但不知不觉就被内容吸引了进去。<br/>"
            "等你抬起头，窗外已经换了一番光景。"
            "你揉了揉酸胀的脖颈，却发现心里某个一直模糊的地方变得清晰了不少。<br/>"
            "<i>（知识 +5）</i>");
        ST->K += 5;
    } else if (roll < 50) {
        if (!ST->libraryTestDone) {
            ST->libraryTestDone = true; // 暂借用作"不安书架本循环触发"
            text = QStringLiteral(
                "你在书架间穿行，手指划过一排排书脊。"
                "在某个冷门的角落里，你看到一本关于时间循环的理论书。"
                "封皮很旧，像是很多年没人碰过了。<br/>"
                "你翻开其中一页，有一行被反复圈画：<br/>"
                "「如果每一天都是一模一样的重复，你怎么知道今天是哪一天？"
                "你又怎么知道——你还是你？」<br/>"
                "你合上书，把那本书推回了原处。<br/>"
                "书脊和旁边那本严丝合缝地贴在一起，像什么都没发生过。<br/>"
                "<i>（知识 +4，精神稳定值 -3）</i>");
            ST->K += 4; ST->S -= 3;
        } else {
            text = QStringLiteral(
                "你摊开书本，笔尖在纸面上沙沙地响。<br/>"
                "你认认真真地学了一会儿。<br/>"
                "<i>（知识 +4）</i>");
            ST->K += 4;
        }
    } else {
        text = QStringLiteral(
            "你摊开书本，笔尖在纸面上沙沙地响。<br/>"
            "你认认真真地学了一会儿。<br/>"
            "<i>（知识 +4）</i>");
        ST->K += 4;
    }
    ST->clampStats();

    // 剧情 aa / bb
    bool bbAvail = ST->libraryAa && ST->K > 20 && !ST->libraryBbDone;
    if (bbAvail && chance(30)) {
        ST->libraryBbDone = true;
        ST->addItem(QStringLiteral("科幻小说书摘"));
        ST->K += 5; ST->S -= 5;
        QString xday = (ST->day == Day::Saturday)
                           ? QStringLiteral("六")
                           : QStringLiteral("日");
        text += QStringLiteral(
                    "<br/><br/>你又看到了那本科幻小说。<br/>"
                    "它还在老地方，书脊微微褪色，像已经等了你很久。"
                    "你把它抽出来，随手翻开。<br/>"
                    "书页间掉出一张借阅记录，轻飘飘落在桌上。"
                    "你拿起来，上面一行日期被反复借还的印戳盖得有些模糊，"
                    "但你还是看清了——某年某月某日，星期%1。<br/>"
                    "夹着借阅记录的那一页，有一段文字：<br/>"
                    "「他一直在等一个最适合出发的时机。等天气再好一点，"
                    "等自己再准备充分一点。后来有人问他：你在怕什么？"
                    "他说，我不是怕出发，我是怕走完。"
                    "怕走完之后发现，原来自己只能走到这里。」<br/>"
                    "那几行字下面，有人用铅笔轻轻划了一道，在旁边写了两个字：「是我。」<br/>"
                    "书从你手中滑落，掉在地上。你呆呆地站着，没有立刻去捡。<br/>"
                    "<i>【获得道具：科幻小说书摘】</i><br/>"
                    "<i>（知识 +5，精神稳定值 -5）</i>")
                    .arg(xday);
    } else if (!ST->libraryAa && chance(30)) {
        ST->libraryAa = true;
        text += QStringLiteral(
            "<br/><br/>你在书架间穿行时，偶然发现一本科幻小说。"
            "你看了看它，没有拿出来。");
    }
    ST->clampStats(); refreshHud();
    MW->showNarration(text, [=]() {
        postDaytimeAction([this](){ libraryScene(); });
    });
}

void SceneEngine::libraryExplore() {
    // 剧情一：阅览桌测试记录 (世界裂缝)
    if (ST->K > 30 &&
        ST->completedTasks.contains(QStringLiteral("食堂阿姨的手帕")) &&
        !ST->worldCracks.contains(QStringLiteral("阅览桌上的测试记录")) &&
        chance(30)) {
        ST->worldCracks.insert(QStringLiteral("阅览桌上的测试记录"));
        QString t = QStringLiteral(
            "你在书架间找位置，经过一张靠窗的阅览桌时，"
            "发现桌上摊着一份打印文件，封面朝上。"
            "旁边的椅子是空的，但桌上还放着一支铅笔，笔身有被久握的痕迹。<br/>"
            "你低头看了一眼封面。标题是「理科食堂NPC对话测试」，"
            "下面表格列着测试项目、触发条件、预期反馈。"
            "每一项都打了勾，最下面一栏是手写的：<br/>"
            "「往后想家了就来，阿姨给你做顿热乎的。——测试通过。"
            "备注：其实这句话，也是我想听到有人对我说的。」<br/>"
            "你盯着那行字看了很久，不由自主地拿起那份文件，翻到下一页。<br/>"
            "还是一段手写的备注，写在打印表格的背面：<br/>"
            "「写这段对话时，我忽然想起上次回家，妈妈做饭的时候，她说了一句差不多的话。」<br/>"
            "下面还有一行，字迹更潦草：「这个NPC可能不会记得，但我不想让她忘。」<br/>"
            "你放下文件。窗外是图书馆的阶梯，远处理科食堂的灯亮着。<br/>"
            "灯光在文件封面上映出一小片白光。那把空椅子还在原处。<br/>"
            "你没有继续往下翻。<br/>"
            "<i>【获得世界裂缝：阅览桌上的测试记录】</i>");
        MW->showNarration(t, [=]() {
            postDaytimeAction([this](){ libraryScene(); });
        });
        return;
    }
    // 橘猫剧情2
    bool catCond = ST->catTeachingSat
        && !ST->catLibrarySat               // 仅在第一次 sat 之前触发
        && ST->slot == TimeSlot::Evening
        && !ST->catLibraryMet
        && !ST->catMissedThisLoop;
    if (catCond && chance(30)) {
        ST->catLibraryMet = true;
        QString t = QStringLiteral(
            "你走出图书馆时，在台阶上又看见了它。<br/>"
            "还是那只橘猫，朝西坐着，尾巴尖轻轻晃了一下。");
        QVector<Choice> cc;
        cc.append({QStringLiteral("1. 坐过去"), [=]() {
            ST->catLibrarySat = true;
            ST->E -= 5; ST->S += 5; ST->clampStats(); refreshHud();
            MW->showNarration(
                QStringLiteral(
                    "你走过去，在它不远处坐下。它看了你一眼。<br/>"
                    "你觉得它认出了你。今天的日落比上次慢一点，云被烧成薄薄的橘色。<br/>"
                    "天暗下来的时候，它站起来，蹭了一下你的手腕，跳下台阶走了。<br/>"
                    "<i>（体力值 -5，精神稳定值 +5）</i>"),
                [=]() { postDaytimeAction([this](){ libraryScene(); }); });
        }});
        cc.append({QStringLiteral("2. 离开"), [=]() {
            ST->catMissedThisLoop = true;
            ST->E -= 5; ST->clampStats(); refreshHud();
            MW->showNarration(QStringLiteral("你看了它一眼，走了。<br/><i>（体力值 -5）</i>"),
                              [=]() { postDaytimeAction([this](){ libraryScene(); }); });
        }});
        MW->showChoices(t, cc);
        return;
    }
    // 随机发现
    if (!ST->randomFoundLibrary && chance(25)) {
        ST->randomFoundLibrary = true;
        int idx = pickWeighted({40, 20, 20, 20});
        QString itemName; QString tail;
        switch (idx) {
            case 0: itemName = QStringLiteral("一张遗留的枫叶书签"); tail = QStringLiteral("（可在背包中使用：精神稳定 +8）"); break;
            case 1: itemName = QStringLiteral("一页泛黄的日记"); tail = QStringLiteral("（无特殊效果）"); break;
            case 2: itemName = QStringLiteral("一张手绘津南校区地图"); tail = QStringLiteral("（无特殊效果）"); break;
            case 3: itemName = QStringLiteral("一包速溶咖啡"); tail = QStringLiteral("（可在背包中使用：体力 +15）"); break;
        }
        ST->addItem(itemName);
        refreshHud();
        MW->showNarration(QStringLiteral(
                              "你在书架的间隙里发现了一件小东西。<br/>"
                              "<i>【获得：%1】%2</i>")
                              .arg(itemName, tail),
                          [=]() { postDaytimeAction([this](){ libraryScene(); }); });
        return;
    }
    MW->showNarration(QStringLiteral(
                          "你慢慢踱过书架间的过道，几个安静的同学抬头看了你一眼。"
                          "你没找到什么特别的。"),
                      [=]() { postDaytimeAction([this](){ libraryScene(); }); });
}

// =====================================================================
// 操场
// =====================================================================
void SceneEngine::playgroundScene() {
    ST->sceneCheckpoint = QStringLiteral("playgroundScene");
    refreshHud();
    if (checkBadEndings()) return;
    if (ST->slot == TimeSlot::Night && !ST->inDormTonight
        && !ST->nightChoiceMadeThisLoop) {
        askReturnToDormIfNight([this](){ playgroundScene(); });
        return;
    }
    QVector<Choice> choices;
    choices.append({QStringLiteral("1. 运动"), [=]() { playgroundSport(); }});
    choices.append({QStringLiteral("2. 探索"), [=]() { playgroundExplore(); }});
    choices.append({QStringLiteral("← 离开操场"), [=]() { campusHub(); }});
    MW->showChoices(QStringLiteral("<b>操场</b><br/>理科操场。塑胶跑道在阳光下散着淡淡热气。") + endingsHint(),
                    choices);
}

void SceneEngine::playgroundSport() {
    ST->Q += 5; ST->clampStats(); refreshHud();
    MW->showNarration(QStringLiteral("你在操场上运动了一会儿。<br/><i>（身体素质 +5）</i>"),
                      [=]() {
                          postDaytimeAction([this](){ playgroundScene(); });
                      });
}

void SceneEngine::playgroundExplore() {
    // 操场剧情一: 手帕
    if (ST->canteenAuntStory111
        && !ST->ladyHandkerchiefFoundThisLoop
        && !ST->hasItem(QStringLiteral("阿姨的手帕"))
        && chance(50)) {
        ST->ladyHandkerchiefFoundThisLoop = true;
        ST->addItem(QStringLiteral("阿姨的手帕"));
        ST->E -= 5; ST->clampStats(); refreshHud();
        MW->showNarration(QStringLiteral(
                              "你沿着跑道慢慢走。夕阳把整片操场染成金色，"
                              "路灯的影子斜斜铺在草地上。<br/>"
                              "走到弯道处，你看见跑道边缘的草丛里有一小块浅蓝色，被草叶半遮着。<br/>"
                              "你弯腰捡起来。是一条手帕，边角绣着一颗小小的爱心。<br/>"
                              "大概是风把它带到了这里。<br/>"
                              "你把它叠好，放进口袋。<br/>"
                              "<i>【获得道具：阿姨的手帕】（体力值 -5）</i>"),
                          [=]() { postDaytimeAction([this](){ playgroundScene(); }); });
        return;
    }
    // 橘猫剧情3
    bool catCond = ST->catTeachingSat && ST->catLibrarySat
        && !ST->catPlaygroundSat            // 仅在第一次 sat 之前触发
        && ST->slot == TimeSlot::Evening
        && !ST->catPlaygroundMet
        && !ST->catMissedThisLoop;
    if (catCond && chance(30)) {
        ST->catPlaygroundMet = true;
        ST->catPlaygroundSat = true;
        ST->E -= 5; ST->S += 10; ST->clampStats();
        ST->completeTask(QStringLiteral("陪猫猫看日落"));
        refreshHud();
        QString t = QStringLiteral(
            "你走到操场弯道旁，那只橘猫蹲在角落。<br/>"
            "你在它旁边坐下，太阳正往树林后面落。"
            "跑道上有几个跑步的人，影子拉得很长。它眯着眼，尾巴搭在你鞋面上。<br/>"
            "光一层一层褪掉，天空从橘色过渡到深蓝。<br/>"
            "它站起来，回头看你。你伸手，它让你摸了一下，"
            "尾巴竖成个小弯钩，然后转身沿着跑道边缘走远了。<br/>"
            "你又坐了一会儿，风很轻。"
            "那些一直悬在你心里的事情，好像也跟着落下来了。<br/>"
            "你忽然想起，这是你很久以来第一次什么都不想，"
            "只是坐着，陪一只猫看完了日落。<br/>"
            "<i>（体力值 -5，精神稳定值 +10）</i><br/>"
            "<i>（任务完成：陪猫猫看日落）</i>");
        MW->showNarration(t, [=]() {
            QVector<Choice> cc;
            cc.append({QStringLiteral("是 - 进入结局：羁绊·星火"), [=]() {
                triggerEndingT1();
            }});
            cc.append({QStringLiteral("否 - 继续剧情"), [=]() {
                ST->unlockEnding(QStringLiteral("羁绊·星火"));
                refreshHud();
                MW->showNarration(QStringLiteral(
                                      "你站起身，看着橘猫远去的方向。<br/>"
                                      "<i>（结局入口【羁绊·星火】已永久解锁。"
                                      "之后想进入这一结局时，"
                                      "可在【宿舍 → 桌前】点击进入。）</i>"),
                                  [=]() { postDaytimeAction([this](){ playgroundScene(); }); });
            }});
            MW->showChoices(QStringLiteral("【是否进入结局：羁绊·星火】"), cc);
        });
        return;
    }
    // 随机发现
    if (!ST->randomFoundPlayground && chance(25)) {
        ST->randomFoundPlayground = true;
        int idx = pickWeighted({20, 30, 40, 10});
        QString itemName; QString tail; int dS = 0;
        switch (idx) {
            case 0: itemName = QStringLiteral("一片很大的梧桐叶"); tail = QStringLiteral("（精神稳定值 +5）"); dS = 5; break;
            case 1: itemName = QStringLiteral("一枚深蓝色的纽扣"); tail = QStringLiteral("（无特殊效果）"); break;
            case 2: itemName = QStringLiteral("一个钥匙扣（上面挂着小铃铛）"); tail = QStringLiteral("（无特殊效果）"); break;
            case 3: itemName = QStringLiteral("一瓶运动饮料"); tail = QStringLiteral("（可在背包中使用：体力 +20）"); break;
        }
        ST->addItem(itemName);
        if (dS) { ST->S += dS; ST->clampStats(); }
        refreshHud();
        MW->showNarration(QStringLiteral(
                              "你在跑道边的草地上发现了一件小东西。<br/>"
                              "<i>【获得：%1】%2</i>")
                              .arg(itemName, tail),
                          [=]() { postDaytimeAction([this](){ playgroundScene(); }); });
        return;
    }
    MW->showNarration(QStringLiteral("你慢慢走着，操场被风吹得很安静。"),
                      [=]() { postDaytimeAction([this](){ playgroundScene(); }); });
}

// =====================================================================
// 到处看看
// =====================================================================
void SceneEngine::wanderScene() {
    ST->sceneCheckpoint = QStringLiteral("wanderScene");
    refreshHud();
    if (checkBadEndings()) return;
    if (ST->slot == TimeSlot::Night && !ST->inDormTonight
        && !ST->nightChoiceMadeThisLoop) {
        askReturnToDormIfNight([this](){ wanderScene(); });
        return;
    }
    QVector<Choice> choices;
    choices.append({QStringLiteral("A. 海棠树下"), [=]() { wanderHaitang(); }});
    choices.append({QStringLiteral("B. 公教楼连廊"), [=]() { wanderCorridor(); }});
    choices.append({QStringLiteral("C. 闲逛"), [=]() { wanderStroll(); }});
    choices.append({QStringLiteral("← 返回"), [=]() { campusHub(); }});
    MW->showChoices(QStringLiteral("<b>到处看看</b><br/>校园的角落总比想象中更多。") + endingsHint(),
                    choices);
}

void SceneEngine::wanderHaitang() {
    QString xxx = (ST->S < 60) ? QStringLiteral("前程似锦") : QStringLiteral("平安喜乐");
    QString base = QStringLiteral(
        "你来到了木斋图书馆外，一棵海棠树，树下挂着几排红笺，"
        "风过时被轻轻吹起，簌簌地响。<br/>"
        "你走近看，其中一张上，黑笔端端正正写着四个字——「%1」。<br/>"
        "你在树旁站了一会儿，风又吹过来，红笺在你身旁轻轻打了个转。<br/>"
        "你的心情变好了。<br/>"
        "<i>（体力值 -5，精神稳定值 +5）</i>").arg(xxx);
    ST->E -= 5; ST->S += 5; ST->clampStats(); refreshHud();

    // 隐藏剧情铁盒
    bool boxCond = ST->K > 10 && !ST->haitangVisitedThisLoop && !ST->haitangBoxTakenCard;
    if (boxCond && chance(30)) {
        ST->haitangVisitedThisLoop = true;
        QString t = base + QStringLiteral(
            "<br/><br/>这次你注意到，树根附近的泥土有一小块不太一样——"
            "颜色比周围的土深，微微隆起，像是被人翻开又重新覆上。<br/>"
            "你蹲下身，用手轻轻拨开表层松动的泥土，指尖碰到一个硬物。<br/>"
            "那是一只铁皮盒子，巴掌大，密封得很好。"
            "盒子正面贴着一张透明塑封的便签，封得严严实实。<br/>"
            "你抹掉表面的浮土，看清了上面的字：<br/>"
            "「如果你看到了这个盒子，请打开它。"
            "里面有一件我没能完成的事。期待有缘人能替我实现。」<br/>"
            "风从海棠树间穿过来，红笺簌簌地响，你捏着那张便签，"
            "忽然觉得指尖有点发烫。<br/><br/><b>【是否打开铁盒？】</b>");
        QVector<Choice> cc;
        cc.append({QStringLiteral("否"), [=]() {
            QString t2 = QStringLiteral(
                "你的手指在盒盖上停了一会儿。<br/>"
                "便签上那行字还在土痕里微微反光——「期待有缘人能替我实现。」<br/>"
                "你不知道里面是什么。也许是一句话，一个名字，一件过期许久的期待。<br/>"
                "但你还没有准备好，去做那个「有缘人」。<br/>"
                "你把铁盒轻轻放回原处，重新覆上泥土，盖得和之前一样平整。<br/>"
                "下次吧。如果还有下次。<br/>"
                "你站起来，拍了拍手上的土。风吹过来，红笺簌簌地响了几声，又安静下去。");
            MW->showNarration(t2, [=]() { postDaytimeAction([this](){ wanderScene(); }); });
        }});
        cc.append({QStringLiteral("是"), [=]() {
            // 三个分支
            auto branch1Or2Open = [=]() {
                QString cardText = QStringLiteral(
                    "盒盖打开了。<br/>"
                    "里面躺着一张对折的卡片，用透明密封袋仔细封着，干燥完好。<br/>"
                    "你拿出卡片。钢笔字迹洇着年深日久的淡蓝，但每个字都清晰：<br/>"
                    "「一直想去琴房坐坐，但课表太满，作业太多。"
                    "想着，等考完试再去，等心情好的时候再去，等哪天有空了再去。"
                    "等着等着，就要毕业了。」<br/>"
                    "「以前常练琴，大学忙起来就没怎么碰过了。"
                    "每次路过琴房，听见里面有人在弹，都会想起以前坐在琴凳上的自己。"
                    "说不上遗憾，就是有点怀念。」<br/>"
                    "「如果你发现了这张卡片，可以替我弹一首吗——什么都好。"
                    "我想听听，这间琴房的声音。」<br/>"
                    "卡片一角画了一架小钢琴，琴键描得很认真。<br/>"
                    "你盯着这张卡片看了一会儿。<br/>"
                    "你也等过。等一个合适的时机，等自己再准备好一点。"
                    "有些事等着等着，就搁置了很久。<br/><br/>"
                    "<b>【是否带走卡片？】</b>");
                QVector<Choice> sub;
                sub.append({QStringLiteral("否"), [=]() {
                    MW->showNarration(
                        QStringLiteral(
                            "你把卡片放回铁盒，盖上盒盖。<br/>"
                            "不是不想替她做，只是这一次，你自己心里还有些东西没想清楚。<br/>"
                            "你把铁盒放回土坑，覆上泥土，拍得和之前一样平整。<br/>"
                            "等准备好了再来吧。如果你准备好了，她也还在这里。<br/>"
                            "<i>（体力值 -5；本循环海棠树场景不再触发此事件。）</i>"),
                        [=]() { postDaytimeAction([this](){ wanderScene(); }); });
                }});
                sub.append({QStringLiteral("是"), [=]() {
                    ST->haitangBoxTakenCard = true;
                    ST->addItem(QStringLiteral("心愿卡"));
                    MW->showNarration(QStringLiteral(
                                          "你将铁盒埋回原处，带走了卡片。<br/>"
                                          "<i>【获得道具：心愿卡】【解锁场景：大通琴房】</i>"),
                                      [=]() { postDaytimeAction([this](){ wanderScene(); }); });
                }});
                MW->showChoices(cardText, sub);
            };
            if (ST->K > 20) {
                MW->showNarration(QStringLiteral(
                                      "你把铁盒翻过来，发现盒盖合口处有一枚小小的挂锁。<br/>"
                                      "你掰了掰锁，纹丝不动。<br/>"
                                      "忽然，你想到了什么。<br/>"
                                      "你试着把手伸进刚才挖出铁盒的土坑里，指尖在松软的泥土中摸索——<br/>"
                                      "碰到了一截冰凉的金属。<br/>"
                                      "是一把很小的钥匙。原来它一直在铁盒下面，被土埋住了。<br/>"
                                      "你把钥匙插进锁孔，轻轻一转。锁舌弹开，发出一声细细的脆响，"
                                      "像隔着好几年终于被听到的一句回应。"),
                                  branch1Or2Open);
            } else if (ST->Q > 20) {
                MW->showNarration(QStringLiteral(
                                      "你把铁盒翻过来，发现盒盖合口处有一枚小小的挂锁。<br/>"
                                      "你掰了掰锁，竟然将锁打开了。"),
                                  branch1Or2Open);
            } else {
                MW->showNarration(QStringLiteral(
                                      "你把铁盒翻过来，发现盒盖合口处有一枚小小的挂锁。<br/>"
                                      "你掰了掰锁，纹丝不动。<br/>"
                                      "你把铁盒翻来覆去检查了一遍，没找到钥匙在哪里。<br/>"
                                      "下次吧。也许下一次，你能找到打开它的办法。"),
                                  [=]() { postDaytimeAction([this](){ wanderScene(); }); });
            }
        }});
        MW->showChoices(t, cc);
        return;
    }
    MW->showNarration(base, [=]() {
        postDaytimeAction([this](){ wanderScene(); });
    });
}

void SceneEngine::wanderCorridor() {
    QString base = QStringLiteral(
        "你来到公教 B，没有找到合适的教室，便从 5 楼的连廊往公教 C 走。<br/>"
        "连廊中段靠窗处摆放着三把椅子，阳光从玻璃窗外倾泻下来，明晃晃铺了一地。<br/>"
        "你走过去坐下，阳光落在身上，暖暖的。<br/>"
        "你的心情变好了。<br/>"
        "<i>（体力值 -5，精神稳定值 +5）</i>");
    ST->E -= 5; ST->S += 5; ST->clampStats(); refreshHud();

    if (!ST->corridorShadowDone) {
        ST->corridorShadowDone = true;
        ST->worldCracks.insert(QStringLiteral("公教楼连廊的人影"));
        ST->S -= 5; ST->clampStats(); refreshHud();
        QString t = base + QStringLiteral(
            "<br/><br/>你站起身，将垂下来的头发别到耳后。<br/>"
            "准备离开时，目光无意间扫过墙壁——"
            "阳光里映着一个人影，就坐在你刚刚坐过的地方，正低头翻看一本厚厚的书。<br/>"
            "你回头。<br/>"
            "三把椅子都是空的。阳光明晃晃地铺在地上，连脚印也没有。<br/>"
            "再看墙上，人影也消失了。"
            "但那一瞬间你记得很清楚——她也抬起手，把垂下来的头发别到了耳后，"
            "动作几乎和你刚才一模一样。<br/>"
            "你站了一会儿，不确定那是不是自己看错了。<br/>"
            "只是那种感觉说不上来，就像照镜子时，"
            "镜子里的那个人，比你慢了半拍。<br/>"
            "<i>（精神稳定值 -5）</i><br/>"
            "<i>【获得世界裂缝：公教楼连廊的人影】</i>");
        MW->showNarration(t, [=]() { postDaytimeAction([this](){ wanderScene(); }); });
        return;
    }
    MW->showNarration(base, [=]() { postDaytimeAction([this](){ wanderScene(); }); });
}

void SceneEngine::wanderStroll() {
    QString base = QStringLiteral(
        "你在校园里闲逛，心情变好了。<br/>"
        "<i>（精神稳定值 +5）</i>");
    ST->S += 5; ST->clampStats(); refreshHud();

    // 南门隐藏剧情
    if (ST->slot == TimeSlot::Evening && !ST->southGateTriggeredThisLoop && chance(10)) {
        ST->southGateTriggeredThisLoop = true;
        ST->S += 20; ST->clampStats(); refreshHud();
        QString t = base + QStringLiteral(
            "<br/><br/>你来到南门前，那条小路上没什么人。"
            "路灯把橘黄色的光铺在地砖上，不算很亮，刚好够看清脚下的影子。<br/>"
            "前面有两个人，正低头看着地上。走近了，才发现他们在拍影子。"
            "他们的影子被拉长了些，落在砖缝和落叶中间。<br/>"
            "他们凑在一起看屏幕，轻轻笑了。<br/>"
            "你站了一会儿，没有走过去。那两道影子靠在一起，"
            "轮廓清晰，像被光特意描过一遍。<br/>"
            "风从南门那边吹过来，你抬头看了一眼路灯，走了。<br/>"
            "<i>（精神稳定值 +20）</i>");
        MW->showNarration(t, [=]() { postDaytimeAction([this](){ wanderScene(); }); });
        return;
    }

    // 随机发现
    if (!ST->randomFoundWander && chance(25)) {
        ST->randomFoundWander = true;
        int idx = pickWeighted({30, 30, 25, 15});
        QString itemName; QString tail;
        switch (idx) {
            case 0: itemName = QStringLiteral("一颗松果"); tail = QStringLiteral("（无特殊效果）"); break;
            case 1: itemName = QStringLiteral("一张被涂鸦过的草稿纸"); tail = QStringLiteral("（无特殊效果）"); break;
            case 2: itemName = QStringLiteral("一截断掉的耳机线"); tail = QStringLiteral("（无特殊效果）"); break;
            case 3: itemName = QStringLiteral("一块圆润的鹅卵石"); tail = QStringLiteral("（可在背包中使用：身体素质 +5）"); break;
        }
        ST->addItem(itemName);
        refreshHud();
        MW->showNarration(base + QStringLiteral(
                              "<br/>路过校园里某个角落，你捡到了一件小东西。<br/>"
                              "<i>【获得：%1】%2</i>")
                              .arg(itemName, tail),
                          [=]() { postDaytimeAction([this](){ wanderScene(); }); });
        return;
    }
    MW->showNarration(base, [=]() { postDaytimeAction([this](){ wanderScene(); }); });
}

// =====================================================================
// 大通琴房
// =====================================================================
void SceneEngine::pianoScene() {
    ST->sceneCheckpoint = QStringLiteral("pianoScene");
    refreshHud();
    if (checkBadEndings()) return;
    if (ST->slot == TimeSlot::Night && !ST->inDormTonight
        && !ST->nightChoiceMadeThisLoop) {
        askReturnToDormIfNight([this](){ pianoScene(); });
        return;
    }
    QVector<Choice> choices;
    choices.append({QStringLiteral("1. 进入琴房"), [=]() { pianoEnter(); }});
    choices.append({QStringLiteral("2. 不进入"), [=]() {
        MW->showNarration(QStringLiteral(
                              "你在门口站了一会儿。<br/>"
                              "不是没勇气。只是这一刻，你觉得自己还没有准备好，"
                              "去替一个陌生人完成她的心愿。<br/>"
                              "下次吧。走廊的暖光灯亮着，没有催你。"),
                          [=]() { campusHub(); });
    }});
    choices.append({QStringLiteral("← 离开琴房楼层"), [=]() { campusHub(); }});
    MW->showChoices(QStringLiteral("<b>大通琴房</b><br/>琴房在大通学生活动中心一楼。") + endingsHint(),
                    choices);
}

void SceneEngine::pianoEnter() {
    if (!ST->carryingStudentCard) {
        MW->showNarration(QStringLiteral(
                              "你翻了翻口袋和背包，都没找到学子卡。<br/>"
                              "告示上写着：「凭学子卡登记借用。」<br/>"
                              "只能下次带上卡再来了。"),
                          [=]() { pianoScene(); });
        return;
    }
    QString t = QStringLiteral(
        "你将学子卡交给物业人员，做好登记，拿到了一间琴房的钥匙。<br/>"
        "你走到琴房门口，用钥匙打开了门。<br/>"
        "你坐在琴凳上，面前是一架立式钢琴。<br/><br/><b>【是否会弹钢琴】</b>");
    QVector<Choice> choices;
    choices.append({QStringLiteral("是"), [=]() {
        ST->addItem(QStringLiteral("一段钢琴录音"));
        MW->showNarration(QStringLiteral(
                              "你打开录音，手指放在琴键上，弹了一首你以前经常练习的曲子——"
                              "这首曲子，既是替她弹的，也是弹给你自己的。<br/>"
                              "<i>【获得道具：一段钢琴录音】</i>"),
                          [=]() { postDaytimeAction([this](){ pianoScene(); }); });
    }});
    choices.append({QStringLiteral("否"), [=]() {
        ST->addItem(QStringLiteral("一段钢琴录音"));
        MW->showNarration(QStringLiteral(
                              "你打开录音，手指放在琴键上，跟着手机上的教程，"
                              "摸索着，慢慢弹了一首简单的曲子——"
                              "这首曲子，既是替她弹的，也是弹给你自己的。<br/>"
                              "<i>【获得道具：一段钢琴录音】</i>"),
                          [=]() { postDaytimeAction([this](){ pianoScene(); }); });
    }});
    MW->showChoices(t, choices);
}

// =====================================================================
// 结局
// =====================================================================
void SceneEngine::enterEndingByName(const QString& name) {
    if (name == QStringLiteral("羁绊·星火")) triggerEndingT1();
    else if (name == QStringLiteral("今日·解铃")) triggerEndingT2();
}

void SceneEngine::triggerEndingB2() {
    QString body = QStringLiteral(
        "你累了。\n\n"
        "身体沉进床垫里。很软，很深。\n\n"
        "意识像一根线，慢慢松了。\n\n"
        "门外有声音。走廊里的脚步声，远远的说笑声。"
        "你知道外面还有光，还有没做完的事，还有在等的人。\n\n"
        "但你走不动了。\n\n"
        "线断了。\n\n"
        "世界还在转，但你已经找到了最安静的沉寂。\n\n"
        "不需要再努力了。\n\n"
        "不需要再挣扎了。\n\n"
        "只是累的时候，合上眼。\n\n"
        "一次不必再醒来的长眠。");
    MW->showEnding(QStringLiteral("结局 B2：沉溺·长眠"), body);
}

void SceneEngine::triggerEndingB1() {
    QString body = QStringLiteral(
        "闹铃响了。\n\n"
        "你睁开眼。手机屏幕上是一个你已经见过很多次的日期。\n\n"
        "周六，06:33。\n\n"
        "【室友A】从床帘后探出头，揉了揉眼睛说：「你今天这么早就起来了？」\n\n"
        "你没有回答。你只是坐起身，踩着楼梯下床，走到桌前，按下开机键。\n\n"
        "熟悉的桌面，熟悉的空白文件夹。\n\n"
        "你把手放上键盘，开始写。写到一半停了一下——这些内容，好像都和上次一模一样。\n\n"
        "你继续写。写完，提交，合上电脑。\n\n"
        "然后你又写了一遍。\n\n"
        "又一遍。\n\n"
        "你不再看日历了。你不再等周一。你只是写。\n\n"
        "写完有时候会提交，有时候不会。写到手指的关节发酸，"
        "写到屏幕的光刺得眼睛发涩。你眼皮很沉，但你没有停。\n\n"
        "你不知道这是第几遍了。你也不想知道。\n\n"
        "【室友A】有时候会探出头看你，有时候不会。"
        "你觉得她好像问过你「今天这么早」，但你记不清了。\n\n"
        "窗外天亮了几次，暗了几次，你没有数。"
        "你的时间是一行代码一行代码垒起来的，垒到后来，你忘了第一行写的是什么。\n\n"
        "你只是在写。\n\n"
        "因为你知道，只要你还在写，就不用抬头看那个日期。\n\n"
        "就不用承认——你已经被困在这里了。\n\n"
        "——\n\n"
        "「你今天这么早就起来了？」\n\n"
        "你睁开眼。手机屏幕亮着，周六，06:33。\n\n"
        "你坐到桌前，手放在键盘上。你不知道这是第几次了。\n\n"
        "但你觉得，这样也好。\n\n"
        "明天还有很多。");
    MW->showEnding(QStringLiteral("结局 B1：困缚·明日何其多"), body);
}

void SceneEngine::triggerEndingT1() {
    QString body = QStringLiteral(
        "你做了很多事。\n\n"
        "你把浅蓝色的手帕交还给食堂阿姨，看见她指腹慢慢抚过那颗小小的爱心。"
        "她说，女儿去外地上学那年绣的，怕她太想她。\n\n"
        "你坐在琴凳上，替一个素未谋面的人弹了一首曲子。录下来，发在社交媒体上，她听到了。"
        "她说，那间琴房的声音，原来是这样。\n\n"
        "你陪一只橘猫，从公教楼的草地，到图书馆的台阶，再到操场的角落——"
        "陪它看完了三次日落。最后一次，它让你摸了一下，尾巴竖成个小弯钩。\n\n"
        "手帕，心愿，橘猫。这些线索散落在校园各处，像早就等在那里。\n\n"
        "而你每一次循环都在靠近它们。不是巧合——"
        "是你在重复的时间里，终于学会了看向身边的人。\n\n"
        "这一次你没有只顾着做作业，你走出去了。"
        "你找回了食堂阿姨丢失的手帕，你替一个人实现了心愿，你陪一只猫看完了日落。\n\n"
        "你们之间，被一些细线连住了。食堂阿姨握住你的手时，老茧硌着你的掌心；"
        "琴房里第一个音落下去，踏板跟着闷响了一声；橘猫尾巴搭在鞋面上，有点痒。\n\n"
        "这些都不是很大的事。但它们亮着，一颗一颗很小很小的光，"
        "聚在一起，成了你掌心里的星火。\n\n"
        "屏幕微微一烫。\n\n"
        "你掏出手机。屏幕上没有通知，但时间在动。\n\n"
        "周日，23:59。\n\n"
        "你盯着那行数字。\n\n"
        "周一，00:00。\n\n"
        "你屏住呼吸。\n\n"
        "一秒，两秒，三秒。\n\n"
        "屏幕没有闪回周六，时间还在一格一格地走。\n\n"
        "周一，00:01。\n\n"
        "你等了一会儿，又等了一会儿，终于确信——现在是周一凌晨，世界在运转。\n\n"
        "你把手机扣在桌上。屏幕暗下去之前，你仿佛看见那些散落在校园各处的光——"
        "食堂窗口被热气裹住的灯，琴房走廊亮起的日光灯，"
        "操场角落那盏忽明忽暗的路灯——一盏一盏，在你心里亮了一遍。\n\n"
        "你听见一声轻响。很细，很脆。像什么终于松开了——"
        "所有你不确定能不能留住的瞬间，终于落了地。\n\n"
        "你忽然明白了。\n\n"
        "循环不是你一个人打破的。你每靠近一个人，每接住一次目光，"
        "每留下一点痕迹，这个重复的世界就松动一分。"
        "那些细线牵着的人，在你看不见的地方，也拉了你一把。\n\n"
        "是这些羁绊，把循环撬开了一条缝。\n\n"
        "星星之火，可以燎原。\n\n"
        "然后世界亮了。\n\n"
        "你回到桌前，循环中完成的大作业文件夹还在，你点了提交。\n\n"
        "网页转了几圈，显示提交成功。\n\n"
        "你合上电脑。窗外有风，树影在窗帘上轻轻晃了一下。\n\n"
        "你躺在舒适的枕头上，闭上眼。\n\n"
        "——\n\n"
        "闹铃响了。\n\n"
        "你睁开眼，心跳猛地快了一拍——然后你按掉手机，屏幕上的时间是周一。\n\n"
        "你坐起身。阳光从窗帘缝隙漏进来，暖暖的。\n\n"
        "像食堂窗口被热气裹住的灯，像琴房走廊亮起的日光灯，"
        "像操场角落那盏忽明忽暗的路灯——所有你曾靠近的光，"
        "都汇聚到这一束里，轻轻落在你的掌心。\n\n"
        "你没有摊开手去看，你只是拢起手指，推开门。\n\n"
        "走进那片光里。");
    MW->showEnding(QStringLiteral("结局 T1：羁绊·星火"), body);
}

void SceneEngine::triggerEndingT2() {
    QString body = QStringLiteral(
        "你躺在床上，盯着天花板。\n\n"
        "旧书桌前，小小的你看着你。你不是不明白——你一直都明白。\n\n"
        "公教楼窗台上那张被划掉的作业纸，图书馆那本科幻小说里被标记出来的文字。"
        "字迹不一样，写的是同一句话：怕不够好，怕走到头。\n\n"
        "你下床，坐在电脑前，打开大作业文件夹。"
        "代码写完了，视频剪完了，这是你在循环中完成的。\n\n"
        "你只是没有提交，总觉得哪里还差一点。\n\n"
        "但差的那一点，到底是什么？\n\n"
        "你盯着提交按钮，手指悬在触控板上方。\n\n"
        "窗外的海棠树被风吹动，影子在窗帘上轻轻晃。"
        "这个世界没有在等你——它一直在往前走。"
        "食堂阿姨在擦台面，橘猫在草地上打盹，琴房的灯亮着，有人在里面弹一首简单的曲子。\n\n"
        "只有你还停在提交按钮前面。\n\n"
        "你忽然想起那张作业纸上，红笔补的那句话：\n\n"
        "「已经很好了，不用害怕。」\n\n"
        "你按下了提交。\n\n"
        "网页转了几圈，显示提交成功。\n\n"
        "屏幕上的光映在你脸上，你忽然想起一件事："
        "循环开始前的那个周末，你写完了，却没交。\n\n"
        "你一直在等一个「最好的版本」。等自己再清醒一点，"
        "等代码再简洁一点，等视频的转场再顺一点。"
        "等所有条件都凑齐了，再点那个按钮。\n\n"
        "但条件永远不会凑齐。你等的那个「最好时机」，"
        "像一个系住的铃，越等越紧。可解铃的人，只有今天的自己。\n\n"
        "你躺在床上，闭上眼。这一次，你没有对自己说「明天再看看」。你说的是：今天。\n\n"
        "——\n\n"
        "闹铃响了。\n\n"
        "你睁开眼，心跳猛地快了一拍——然后你按掉手机，屏幕上的时间是周一。\n\n"
        "你下床，坐到桌前。电脑屏幕上的提交页面还开着，"
        "上面显示一行黑色的字：已于 周日 23:59 提交。\n\n"
        "你拿起手机，看到那条很久以前设的备忘——\n\n"
        "「别拖了，交吧。」\n\n"
        "你删掉了那行字，把手机放进口袋。\n\n"
        "你推开门，走廊里有人跟你打招呼。\n\n"
        "你说，早。");
    MW->showEnding(QStringLiteral("结局 T2：今日·解铃"), body);
}

void SceneEngine::triggerHiddenEnding(int branch) {
    if (branch == 1) {
        QString body = QStringLiteral(
            "你接着她的最后一行，写道：\n\n"
            "「不用急着告别。」\n\n"
            "「那些你舍不得的人，在你的世界里都好好的。"
            "食堂阿姨会好好放着手帕，橘猫会出现在校园各处等日落，"
            "海棠树下的红笺会在风中轻摇。」\n\n"
            "「你写过的每一样东西，都会继续好好地存在着。包括你。」\n\n"
            "光标停了一拍。你打下最后一句：\n\n"
            "「谢谢你，把我留在这里。现在，你可以往前走了。」\n\n"
            "屏幕亮了一瞬。世界开始褪色——灰白从柳树漫上你的手指，漫过你的胸口。\n\n"
            "但你不害怕。\n\n"
            "最后消失的是眼睛。你看见路灯亮起来，春天的海棠开了满满一树，"
            "风吹过红笺簌簌响。\n\n"
            "有人在远处朝你挥手。她穿着学士服，站在光里。"
            "你看不清她的脸，但你觉得，她笑了。\n\n"
            "然后一切沉入安静的白色。\n\n"
            "——\n\n"
            "你睁开眼。\n\n"
            "宿舍天花板的纹路，和每一次醒来时一模一样。\n\n"
            "但有什么不同了——手机屏幕亮着，时间是周一。\n\n"
            "你躺了一会儿，慢慢坐起来。然后你下床，打开了电脑。\n\n"
            "大作业文件夹还在。循环中你从头开始写的代码，剪辑好的视频，都在。"
            "然后你打开提交页面，上传文件。网页转了几圈，弹出提示——\n\n"
            "提交成功。截止时间：今天 23:59。\n\n"
            "你盯着「提交成功」这四个字看了几秒，轻轻合上电脑。\n\n"
            "出门时，你在海棠树下停了一会儿。"
            "风吹过来，有轻摇的红笺，也有新绿的叶子。\n\n"
            "你忽然想起那个周六早晨，你第一次发现文件夹是空的时候，心跳很快，手是凉的。\n\n"
            "现在你心跳很稳，手是热的。\n\n"
            "你继续往前走。");
        MW->showEnding(QStringLiteral("隐藏结局：茧与蝶 · 成蝶"), body);
    } else {
        QString body = QStringLiteral(
            "你在批注最下方写了一行：\n\n"
            "「你的大学，我替你记着。」\n\n"
            "屏幕暗下去，倒映出你自己的脸。\n\n"
            "你看见倒影里，身后不远处站了一个人。"
            "学士服的垂布搭在肩上，和你一样的轮廓，一样红的眼眶。\n\n"
            "她没有说话，你也没有回头。\n\n"
            "你只是轻轻点了一下头，伸手把屏幕合上。\n\n"
            "合上的那一刻，你听见身后传来很轻的一句话：\n\n"
            "「那你也替我，好好继续生活。」\n\n"
            "声音从很远的地方来，像隔着四年，隔着一段还没走完的路。\n\n"
            "——\n\n"
            "你睁开眼。\n\n"
            "宿舍天花板的纹路，和每一次醒来时一模一样。\n\n"
            "但有什么不同了——手机屏幕亮着，时间是周一。\n\n"
            "你慢慢坐起来，打开了电脑。\n\n"
            "大作业文件夹还在。循环中你从头开始写的代码，剪辑好的视频，都在。"
            "然后你打开提交页面，上传文件。网页转了几圈，弹出提示——\n\n"
            "提交成功。截止时间：今天 23:59。\n\n"
            "你盯着「提交成功」这四个字看了几秒，轻轻合上电脑。\n\n"
            "出门时，海棠树还是那棵海棠树。"
            "你抬头看了一眼，想起循环里捡到的那张心愿卡，想起琴房的录音，"
            "想起橘猫尾巴搭在鞋面上的重量。\n\n"
            "想起那个穿学士服的人，站在很远的地方。\n\n"
            "你背着书包往前走。身后海棠树下的红笺被风掀起一角，轻轻碰了一声。\n\n"
            "像谁应了一句。");
        MW->showEnding(QStringLiteral("隐藏结局：茧与蝶 · 共振"), body);
    }
}

void SceneEngine::onMidnightCheck(std::function<void()> next) {
    if (next) next();
}

void SceneEngine::afterAction(std::function<void()> next) {
    if (next) next();
}

void SceneEngine::advanceSlot() {
    postDaytimeAction(nullptr);
}
