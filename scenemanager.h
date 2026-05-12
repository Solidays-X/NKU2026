#ifndef SCENEMANAGER_H
#define SCENEMANAGER_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include "gamestate.h"
#include <QRandomGenerator>
#include <QSet>
// 添加成员：

class MainWindow;

class SceneManager : public QObject
{
    Q_OBJECT

public:
    QSet<QString> m_firstTimeFlags;
    QSet<QString> m_markers;         // 持久标记
    QSet<QString> m_dailyMarkers;    // 每日标记（每天重置）
    QSet<QString> m_loopMarkers;     // 每循环标记（如 random discovery 仅一次）
    QString currentNode; // 改为 public，便于存档
    void enterEnding(const QString &endingNode);
    void restoreFromLoad(const QString &nodeKey);
    int m_currentLoop = 0;           // 检测循环变化以重置 loopMarkers

    void onStoryFinished();

    DayOfWeek m_lastDay = DayOfWeek::Saturday;

    explicit SceneManager(MainWindow *window, QObject *parent = nullptr);

    void startGame(); // 开始新游戏

private slots:
    void onOptionSelected(int index);

private:
    void loadScenarioData(const QString &filePath);
    void processNode(const QString &nodeKey);
    void applyEffect(const QJsonObject &effect);
    bool checkCondition(const QString &conditionExpr);
    bool evaluateComplexCondition(const QString &cond);  // 新增复合条件
    void goToNode(const QString &nodeKey);
    void advanceTimeAndGoTo(const QString &nextNode);     // 推进时间并跳转
    void returnToCampusHub();                             // 返回校园主界面
    bool m_isEnding = false;   // 标记当前是否正在播放结局文本，防止结束后弹出选项
    QString previousNode;

    void applyLoopEffect(const QJsonObject &loopEffect);
    void checkGameOver();

    MainWindow *m_window;
    GameState *m_state;
    QJsonObject m_data;             // 整个 JSON 场景数据

    // 存储当前节点选项对应的 next 节点列表
    QStringList m_optionNextNodes;
    QStringList m_pendingOptions;
    QString m_autoNext;
    bool m_waitingForOption = false;
    int m_repeatCount = 0;          // 用于 node2 作业重复计数
    QString m_previousLocation;   // 进入子场景前的节点（如"campus_hub"）
};

#endif // SCENEMANAGER_H