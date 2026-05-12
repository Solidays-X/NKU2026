// Headless test driver: 通过驱动 SceneEngine 走完一条剧情，
// 用以发现 bug 与覆盖关键剧情节点。
#include "gamestate.h"
#include "scenes.h"
#include "headless.h"

#include <QCoreApplication>
#include <QTextStream>
#include <QString>
#include <QRandomGenerator>
#include <QSet>
#include <QDir>
#include <functional>

#include <QFile>
static QFile* g_logFile = nullptr;
static void logLine(const QString& s) {
    if (g_logFile) {
        g_logFile->write(s.toLocal8Bit());
        g_logFile->write("\n");
        g_logFile->flush();
    }
}

// 简单优先级 picker：根据关键字优先选项。
// goal 是一个有序关键字列表 — 在 choices 中匹配到 contains 即选中。
static int pickByKeywords(const QVector<Choice>& choices,
                          const QStringList& priorities) {
    for (const QString& kw : priorities) {
        for (int i = 0; i < choices.size(); ++i) {
            if (choices[i].text.contains(kw)) return i;
        }
    }
    return -1;
}

struct TestPlan {
    QString name;
    std::function<int(const QString&, const QVector<Choice>&)> picker;
    QString expectedEnding; // substring 匹配
    int maxSteps = 200000;
};

// 工具：寻找包含某关键字的选项 index
static int findChoice(const QVector<Choice>& choices, const QString& kw) {
    for (int i = 0; i < choices.size(); ++i)
        if (choices[i].text.contains(kw)) return i;
    return -1;
}

static int g_pickerDebugCount = 0;
static bool pickerLongRouteShouldDebug() {
    return g_pickerDebugCount++ < 80;
}
static int pickerLongRoute(const QString& text,
                           const QVector<Choice>& choices,
                           const GameState& st) {
    auto find = [&](const QString& kw) { return findChoice(choices, kw); };
    auto needItem = [&](const QString& name) { return !st.hasItem(name); };
    auto needCrack = [&](const QString& n) { return !st.worldCracks.contains(n); };
    auto needTask = [&](const QString& n) { return !st.completedTasks.contains(n); };

    bool needHaitangBox = needItem("心愿卡") && !st.haitangBoxTakenCard;
    bool needPianoPlay = st.hasItem("心愿卡") && !st.hasItem("一段钢琴录音")
                         && needTask("琴房的心愿卡");
    bool needPost = st.hasItem("一段钢琴录音");
    bool needReply = st.publishedRecording && !st.gotReplyMessage;
    bool needCorridor = needCrack("公教楼连廊的人影");
    bool needBlank = needCrack("一张空白的纸");
    bool needTestRec = needCrack("阅览桌上的测试记录");
    bool needHandkerchief = !st.everCompletedHandkerchief;
    bool needOldHomework = needItem("陈旧的作业纸");
    bool needBookExcerpt = needItem("科幻小说书摘");
    bool needCatTeach = !st.catTeachingSat &&
                        st.completedTasks.contains("琴房的心愿卡");
    bool needCatLib = st.catTeachingSat && !st.catLibrarySat;
    bool needCatPlay = st.catLibrarySat && !st.catPlaygroundSat;
    bool needT1 = !st.unlockedEndings.contains("羁绊·星火");
    bool needT2 = !st.unlockedEndings.contains("今日·解铃");
    bool readyHidden =
        st.unlockedEndings.contains("羁绊·星火") &&
        st.unlockedEndings.contains("今日·解铃") &&
        st.worldCracks.size() >= 3;

    // 紧急：体力极低 -> 优先睡觉(避免触发 B2 bad ending)
    bool urgentSleep = st.E <= 15 || st.S < 15;

    // ========== 通用对话框处理 ==========
    if (text.contains("是否打开铁盒")) return find("是");
    if (text.contains("是否带走卡片")) return find("是");
    if (text.contains("是否会弹钢琴")) return find("是");
    if (text.contains("是否携带学子卡")) return find("是");
    if (text.contains("是否选择回宿舍")) return find("是");
    if (text.contains("是否进入结局")) {
        // 仅在准备好隐藏结局时，**不要**进入 T1/T2(那样会终止)；其它情况也选否
        return find("否");
    }
    if (text.contains("要小憩一会儿吗")) {
        return find("小憩");
    }
    if (text.contains("好好睡一觉") || (text.contains("躺在柔软") && text.contains("闭上眼"))) {
        return find("好好睡");
    }

    // ========== 节点 1 / 节点 2 / 节点 3 ==========
    if (find("再睡一会儿") >= 0) {
        // 节点1: 下床
        return find("下床");
    }
    if (find("做作业") >= 0 && st.L == 0 && choices.size() == 1) {
        return find("做作业");
    }
    if (find("继续做作业") >= 0 && find("探索真相") >= 0) {
        return find("探索真相");
    }

    // ========== 校园 Hub ==========
    bool atHub = text.contains("要去哪里");
    if (atHub) {
        // 夜晚 -> 宿舍
        if (st.slot == TimeSlot::Night) return find("宿舍");
        if (urgentSleep) return find("宿舍");

        bool canteenOpen = (st.slot == TimeSlot::Noon ||
                            st.slot == TimeSlot::Evening);
        bool pianoDone = st.completedTasks.contains("琴房的心愿卡");

        // ===== 优先级 1: 即时事件 (一次性世界裂缝/铁盒) =====
        // 空白纸 (一次性, 不消耗时段)
        if (needBlank) return find("宿舍");
        // 公教楼连廊
        if (needCorridor) return find("到处看看");
        // 海棠树下铁盒 (开锁需 K>20 或 Q>20)
        if (needHaitangBox && (st.K > 20 || st.Q > 20)) {
            return find("到处看看");
        }

        // ===== 优先级 2: 琴房任务链 (拿心愿卡 -> 学子卡 -> 琴房 -> 发帖 -> 私信) =====
        if (needPianoPlay && !st.carryingStudentCard) return find("宿舍");
        if (needPianoPlay && st.carryingStudentCard && find("大通琴房") >= 0)
            return find("大通琴房");
        if (needPost) return find("宿舍");
        if (needReply) return find("宿舍");

        // ===== 优先级 3: 傍晚 橘猫三连 (需 piano done) =====
        if (st.slot == TimeSlot::Evening && pianoDone) {
            if (needCatTeach) return find("公教楼");
            if (needCatLib) return find("图书馆");
            if (needCatPlay) return find("操场");
        }

        // ===== 优先级 4: 手帕任务 (L>=8) =====
        if (needHandkerchief && st.L >= 8 && canteenOpen) {
            if (!st.canteenAuntUnlocked) return find("食堂");
            if (!st.canteenAuntStory111) return find("食堂");
        }
        if (needHandkerchief && st.L >= 8 && st.canteenAuntStory111 &&
            !st.hasItem("阿姨的手帕")) {
            return find("操场");
        }
        if (needHandkerchief && st.L >= 8 && st.hasItem("阿姨的手帕") && canteenOpen) {
            return find("食堂");
        }

        // ===== 优先级 5: 道具收集 =====
        // 陈旧的作业纸 (公教楼学习 aa+bb)
        if (needOldHomework) return find("公教楼");
        // 科幻小说书摘 (图书馆学习 aa+bb)
        if (needBookExcerpt) return find("图书馆");
        // 阅览桌测试记录 (K>30 + 完成手帕任务)
        if (needTestRec && st.K > 30 &&
            st.completedTasks.contains("食堂阿姨的手帕")) {
            return find("图书馆");
        }

        // ===== 优先级 6: 中午/傍晚补体力 =====
        if (canteenOpen && st.E < 60) {
            return find("食堂");
        }

        // 兜底
        if (st.K < 30) return find("图书馆");
        if (st.Q < 25) return find("操场");
        return find("图书馆");
    }

    // ========== 宿舍 ==========
    if (text.contains("你的宿舍")) {
        if (st.slot == TimeSlot::Night) return find("睡觉");
        if (urgentSleep) return find("睡觉");

        if (needPost || needReply) return find("桌前");
        if (needPianoPlay && !st.carryingStudentCard) return find("桌前");
        if (needBlank) return find("桌前");
        if (find("离开宿舍") >= 0) return find("离开宿舍");
        return 0;
    }
    if (text.contains("桌前") && text.contains("物品")) {
        if (needPost) return find("发帖");
        if (needReply) return find("手机");
        if (needBlank) return find("空白");
        if (needPianoPlay && !st.carryingStudentCard) return find("学子卡");
        return find("返回宿舍");
    }

    // ========== 食堂 ==========
    if (text.contains("理科食堂")) {
        if (find("食堂阿姨") >= 0) return find("食堂阿姨");
        if ((st.slot == TimeSlot::Noon || st.slot == TimeSlot::Evening) &&
            find("吃饭") >= 0)
            return find("吃饭");
        return find("离开食堂");
    }

    // ========== 公教楼 / 图书馆 / 操场 ==========
    bool pianoDone = st.completedTasks.contains("琴房的心愿卡");
    bool canteenOpenNow = (st.slot == TimeSlot::Noon ||
                            st.slot == TimeSlot::Evening);
    // 手帕任务路径(L>=8)和测试记录裂缝任务需要主动离开学/玩场景去食堂
    bool needLeaveForCanteen = needHandkerchief && st.L >= 8 && canteenOpenNow;
    bool needLeaveForPlayground = needHandkerchief && st.L >= 8 &&
                                    st.canteenAuntStory111 &&
                                    !st.hasItem("阿姨的手帕");

    if (text.contains("公共教学楼")) {
        if (st.slot == TimeSlot::Evening && pianoDone) {
            if (needCatTeach) return find("探索");
            if (needCatLib || needCatPlay) return find("离开公共教学楼");
        }
        if (needLeaveForCanteen || needLeaveForPlayground)
            return find("离开公共教学楼");
        if (needOldHomework) return find("学习");
        return find("学习");
    }
    if (text.contains("津南图书馆")) {
        if (st.slot == TimeSlot::Evening && pianoDone) {
            if (needCatLib) return find("探索");
            if (needCatTeach || needCatPlay) return find("离开图书馆");
        }
        if (needLeaveForCanteen || needLeaveForPlayground)
            return find("离开图书馆");
        // 优先拿 科幻小说书摘 (学习 aa + bb)；之后再去探索拿测试记录
        if (needBookExcerpt) return find("学习");
        if (needTestRec && st.K > 30 &&
            st.completedTasks.contains("食堂阿姨的手帕"))
            return find("探索");
        return find("学习");
    }
    if (text.contains("理科操场")) {
        if (st.slot == TimeSlot::Evening && pianoDone) {
            if (needCatPlay) return find("探索");
            if (needCatTeach || needCatLib) return find("离开操场");
        }
        // 需要手帕：在操场探索拿手帕 - 优先于"去食堂"
        // (避免与 hub 决定循环: hub 决定来操场,操场又决定回 hub)
        if (needHandkerchief && st.canteenAuntStory111 &&
            !st.hasItem("阿姨的手帕")) {
            return find("探索");
        }
        if (needLeaveForCanteen) return find("离开操场");
        if (st.Q < 25) return find("运动");
        return find("运动");
    }

    // ========== 到处看看 ==========
    if (text.contains("到处看看")) {
        if (needCorridor) return find("公教楼连廊");
        if (needHaitangBox && (st.K > 20 || st.Q > 20)) return find("海棠树下");
        return find("闲逛");
    }

    // 橘猫遇见
    if (text.contains("橘猫")) return find("坐过去");

    // 琴房入口
    if (text.contains("琴房在大通")) return find("进入琴房");

    // 隐藏结局相关
    if (find("去马蹄湖") >= 0) return find("去马蹄湖");
    if (find("敲下结局") >= 0) return find("敲下结局");

    return -1;
}

// ----- 回归用 picker: 用于触发 B1 / B2 / T1 / T2 等 -----
// 只追求 N>=19 触发 B1: 每个轮次都做作业
static int pickerB1(const QString& text, const QVector<Choice>& choices,
                    const GameState& st) {
    auto find = [&](const QString& kw) {
        for (int i = 0; i < choices.size(); ++i)
            if (choices[i].text.contains(kw)) return i;
        return -1;
    };
    if (find("再睡一会儿") >= 0) return find("下床");
    if (find("做作业") >= 0 && choices.size() == 1) return find("做作业");
    // 节点2/节点3: 始终继续做作业
    if (find("继续做作业") >= 0) return find("继续做作业");
    if (text.contains("是否选择回宿舍")) return find("是");
    if (text.contains("好好睡一觉")) return find("好好睡");
    if (text.contains("你的宿舍")) return find("做作业");
    if (text.contains("要去哪里")) return find("宿舍");
    if (find("是") >= 0) return find("是");
    return 0;
    (void)st;
}

// 体力消耗到 0 触发 B2: 一直运动到死
static int pickerB2(const QString& text, const QVector<Choice>& choices,
                    const GameState& st) {
    auto find = [&](const QString& kw) {
        for (int i = 0; i < choices.size(); ++i)
            if (choices[i].text.contains(kw)) return i;
        return -1;
    };
    if (find("再睡一会儿") >= 0) return find("下床");
    if (find("做作业") >= 0 && choices.size() == 1) return find("做作业");
    if (find("探索真相") >= 0) return find("探索真相");
    if (text.contains("是否选择回宿舍")) return find("否");
    if (text.contains("好好睡一觉")) return find("再撑");
    if (text.contains("是否进入结局")) return find("否");
    if (text.contains("理科操场")) return find("运动");
    if (text.contains("要去哪里")) return find("操场");
    if (text.contains("你的宿舍")) return find("离开");
    return 0;
    (void)st;
}

// 主要测试：尝试推进到结局
static bool runTest(const QString& planName,
                    std::function<int(const QString&, const QVector<Choice>&,
                                      const GameState&)> picker,
                    int maxSteps,
                    QString* outEnding) {
    GameState state;
    HeadlessView view;
    view.attachState(&state);
    view.setVerbose(true);
    view.setMaxSteps(maxSteps);

    view.setPicker([&](const QString& t, const QVector<Choice>& c) {
        return picker(t, c, state);
    });

    SceneEngine engine(&view, &state);
    engine.startIntro();

    int safety = 0;
    while (view.pending() != HeadlessView::Pending::Ended) {
        if (!view.step()) break;
        if (++safety > maxSteps) {
            logLine(QStringLiteral("[%1] timeout at step %2")
                    .arg(planName).arg(safety));
            break;
        }
    }
    if (outEnding) *outEnding = view.endingTitle();
    logLine(QStringLiteral("[%1] reached ending: %2 in %3 steps")
            .arg(planName, view.endingTitle()).arg(safety));
    return view.pending() == HeadlessView::Pending::Ended
        && !view.endingTitle().isEmpty();
}

int main(int argc, char* argv[]) {
    auto sentMark = [](const char* tag) {
        QFile s(QStringLiteral("C:/nankai_test_tmp/sentinel.txt"));
        if (s.open(QIODevice::Append)) {
            s.write(tag);
            s.write("\n");
            s.close();
        }
    };
    sentMark("a-before-qapp");
    QCoreApplication app(argc, argv);
    sentMark("b-after-qapp");
    // 注意: QRandomGenerator::global() 是只读的, 不能调用 seed()
    // 改用本地实例并在 scenes 中通过 QRandomGenerator::global() 读
    sentMark("c-after-seed");

    sentMark("d-step1");
    extern void HeadlessView_setLogFile(QFile*);
    QString logPath = QStringLiteral("C:/nankai_test_tmp/test_output.log");
    sentMark("e-step2");
    QFile f(logPath);
    sentMark("f-step3");
    bool opened = f.open(QIODevice::WriteOnly | QIODevice::Text);
    sentMark(opened ? "g-open-ok" : "g-open-fail");
    if (!opened) return 99;
    g_logFile = &f;
    HeadlessView_setLogFile(&f);
    sentMark("h-step5");
    logLine(QStringLiteral("[INIT] cwd=%1 log=%2")
            .arg(QDir::currentPath(), f.fileName()));
    sentMark("i-step6");

    logLine(QStringLiteral("Headless test starting..."));

    int allOk = 0;
    int allRuns = 0;

    QString endingTitle;
    bool ok = runTest("LongestRoute", &pickerLongRoute, 200000, &endingTitle);
    logLine(QStringLiteral("Final ending title: %1 (ok=%2)")
            .arg(endingTitle).arg(ok ? 1 : 0));
    if (ok && endingTitle.contains("茧与蝶")) allOk++;
    allRuns++;

    logLine(QStringLiteral("=============== B1 Regression ==============="));
    QString e2;
    bool ok2 = runTest("B1Regression", &pickerB1, 20000, &e2);
    logLine(QStringLiteral("B1 ending: %1 (ok=%2)").arg(e2).arg(ok2 ? 1 : 0));
    if (ok2 && e2.contains("B1")) allOk++;
    allRuns++;

    logLine(QStringLiteral("=============== B2 Regression ==============="));
    QString e3;
    bool ok3 = runTest("B2Regression", &pickerB2, 20000, &e3);
    logLine(QStringLiteral("B2 ending: %1 (ok=%2)").arg(e3).arg(ok3 ? 1 : 0));
    if (ok3 && e3.contains("B2")) allOk++;
    allRuns++;

    logLine(QStringLiteral("=============== SUMMARY: %1/%2 passed ===============")
            .arg(allOk).arg(allRuns));
    f.close();
    return (allOk == allRuns) ? 0 : 1;
}
