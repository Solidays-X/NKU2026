#pragma once

#include "gameview.h"
#include <QObject>
#include <QString>
#include <QVector>
#include <QTextStream>
#include <functional>

class GameState;
class SceneEngine;

// Picker: 给定当前文本和选项列表，返回选择的 index。
// 返回 -1 表示由默认策略选第一个非"返回/离开"的选项。
using ChoicePicker =
    std::function<int(const QString& text, const QVector<Choice>& choices)>;

class HeadlessView : public QObject, public IGameView {
    Q_OBJECT
public:
    explicit HeadlessView(QObject* parent = nullptr);

    void showNarration(const QString& text, std::function<void()> cont) override;
    void showChoices(const QString& text, const QVector<Choice>& choices) override;
    void showEnding(const QString& title, const QString& body) override;
    void refreshHud() override;

    void setPicker(ChoicePicker p) { m_picker = std::move(p); }
    void attachState(GameState* st) { m_state = st; }

    // 状态 (在事件循环中由 step() 推进)
    enum class Pending { None, Narration, Choices, Ended };
    Pending pending() const { return m_pending; }
    const QString& lastText() const { return m_lastText; }
    const QVector<Choice>& lastChoices() const { return m_lastChoices; }
    const QString& endingTitle() const { return m_endingTitle; }
    const QString& endingBody() const { return m_endingBody; }
    int stepCount() const { return m_steps; }
    void resetEnded() { m_pending = Pending::None; }

    // 推进一步：执行 cont 或挑一个选项。返回 false 表示已结束/无事可做。
    bool step();

    void setVerbose(bool v) { m_verbose = v; }
    void setMaxSteps(int n) { m_maxSteps = n; }

    QString stripHtml(QString s) const;

signals:
    void endingReached(const QString& title);

private:
    GameState* m_state = nullptr;
    ChoicePicker m_picker;
    Pending m_pending = Pending::None;

    QString m_lastText;
    QVector<Choice> m_lastChoices;
    std::function<void()> m_lastCont;

    QString m_endingTitle;
    QString m_endingBody;

    int m_steps = 0;
    int m_maxSteps = 200000;
    bool m_verbose = false;
};
