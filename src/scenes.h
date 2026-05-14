#pragma once

#include <QString>
#include <QObject>
#include <functional>

class GameState;
class IGameView;

class SceneEngine : public QObject {
    Q_OBJECT
public:
    SceneEngine(IGameView* view, GameState* state, QObject* parent = nullptr);

    void startIntro();
    // 读档时使用: 跳过开场剧情, 直接显示校园 hub
    void resumeFromHub();
    // 按 GameState.sceneCheckpoint 进入对应场景 (读档使用)
    void resumeByCheckpoint();

    // 节点 1
    void node1();
    // 节点 2 (L = 0, 首次循环)
    void node2();
    // 节点 3 (L >= 1)
    void node3();

    // 校园探索主界面
    void campusHub();

    // 各场景
    void dormScene();
    void canteenScene();
    void teachingScene();
    void libraryScene();
    void playgroundScene();
    void wanderScene();
    void pianoScene();

    // 宿舍子场景
    void dormDesk();
    void dormSleep();
    void dormSleepNight();
    void dormRoommate(int which); // 1=A, 2=B, 3=C

    // 食堂
    void canteenEat();
    void canteenAunt();

    // 公教楼/图书馆/操场子动作
    void teachingStudy();
    void teachingExplore();

    void libraryStudy();
    void libraryExplore();

    void playgroundSport();
    void playgroundExplore();

    // 到处看看
    void wanderHaitang();
    void wanderCorridor();
    void wanderStroll();

    // 琴房
    void pianoEnter();

    // 时间推进与日常结算
    void advanceSlot();  // 推进一个时段, 处理是否夜晚询问回宿舍
    void afterAction(std::function<void()> next); // 在动作执行之后的统一调度
    void onMidnightCheck(std::function<void()> next);

    // 进入下一天 (周六->周日, 早晨)
    void enterNextDayMorning();
    // 循环结束 (周日夜晚结算 -> L+1, 回到节点1)
    void endLoopAndRespawn();

    // 检查游戏结束条件
    bool checkBadEndings(); // E<=0 -> B2; L>52 -> B2
    void triggerEndingB2();
    void triggerEndingB1();
    void triggerEndingT1();
    void triggerEndingT2();
    void triggerHiddenEnding(int branch); // 1=成蝶, 2=共振

    // 通过名称进入结局(用于状态栏下的解锁入口按钮)
    void enterEndingByName(const QString& name);

    // 工具
    void mainCampusPrompt(const QString& extraText = QString());

private:
    IGameView* m_mw;
    GameState* m_st;

    // 在所有"白天动作"后调用以推进时间
    void postDaytimeAction(std::function<void()> next);

    // 状态/HUD 刷新简写
    void refreshHud();

    // 处理夜晚回宿舍询问
    void askReturnToDormIfNight(std::function<void()> next);

    // 校园探索菜单是否需要插入"试探循环"等
    QString endingsHint() const;
};
