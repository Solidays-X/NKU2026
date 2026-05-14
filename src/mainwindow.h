#pragma once

#include <QMainWindow>
#include <QPixmap>
#include <QVector>
#include <QString>
#include <QStringList>
#include <functional>
#include <memory>

#include "gameview.h"

class QLabel;
class QPushButton;
class QVBoxLayout;
class QFrame;
class QTextBrowser;
class GameState;
class SceneEngine;

class MainWindow : public QMainWindow, public IGameView {
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;

    // ===== IGameView =====
    void showNarration(const QString& text, std::function<void()> cont) override;
    void showChoices(const QString& text, const QVector<Choice>& choices) override;
    void showEnding(const QString& title, const QString& body) override;
    void refreshHud() override;

    // 仅追加文本（不弹选项；通常会随后调用 showChoices / showNarration ）
    void setPrompt(const QString& text);

protected:
    void resizeEvent(QResizeEvent* e) override;
    void paintEvent(QPaintEvent* e) override;
    bool eventFilter(QObject* obj, QEvent* ev) override;

private:
    void clearChoices();
    void layoutChildren();
    void buildHud();
    void scrollToTop();
    void setDialogText(const QString& html);
    QString styleSheetForButton() const;
    QString applyScramble(const QString& s) const;
    static QString stripChoiceNumber(const QString& s);  // 去掉选项前的"1." "（1）" 等编号
    void showJournal();                  // 显示道具/任务/裂缝/结局图鉴
    void showBackpack();                 // 显示背包(已获得物品列表)
    void showLog();                      // 显示剧情回看
    void openSavePanel();                // 弹出存档面板 (5 个槽位)
    void openLoadPanel(bool fromStartMenu = false);  // 弹出读档面板
    bool saveToSlot(int slot, QString* err = nullptr);  // 写入第 slot 个槽位
    bool loadFromSlot(int slot, QString* err = nullptr); // 读取第 slot 个槽位
    void showResumedDialog();            // 读档后重现存档点对话
    void appendToLog(const QString& text); // 追加一行到回看日志
    void showStartMenu();                // 启动菜单
    QString savesDir() const;            // 存档目录
    QString slotPath(int slot) const;    // 第 slot 个槽位的文件路径
    static constexpr int kSaveSlots = 5;

    QPixmap m_bgPixmap;
    QPixmap m_bgScaled;

    // 当前 narration 的 "继续" 回调，供点击对话框空白处触发
    std::function<void()> m_pendingContinue;

    // HUD 顶部左
    QLabel* m_timeLabel = nullptr;
    QLabel* m_loopLabel = nullptr;
    // HUD 底部左
    QLabel* m_eLabel = nullptr;
    QLabel* m_sLabel = nullptr;
    QLabel* m_kLabel = nullptr;
    QLabel* m_qLabel = nullptr;
    // 状态栏下方：已解锁结局入口
    QFrame* m_endingPanel = nullptr;
    QVBoxLayout* m_endingPanelLayout = nullptr;
    QPushButton* m_journalBtn = nullptr;  // "图鉴" 按钮 (顶部)
    QPushButton* m_backpackBtn = nullptr; // "背包" 按钮 (顶部, 在图鉴下方)
    QPushButton* m_saveBtn = nullptr;     // "存档" (右下)
    QPushButton* m_loadBtn = nullptr;     // "读档" (右下)
    QPushButton* m_logBtn = nullptr;      // "剧情回看" (右下)

    // 剧情回看缓存 (最近若干条, 跨"重新开始"也保留)
    QStringList m_storyLog;
    static constexpr int kMaxLog = 200;

    // 最近显示的对话框文本 / 是否在选项模式 (供存档"截图"显示)
    QString m_lastDialogText;
    bool m_lastWasChoices = false;
    QStringList m_lastChoiceLabels;

    // showNarration 的递归 lambda 持有者 (保活直到读完最后一页)
    QVector<std::shared_ptr<std::function<void(int)>>> m_narrationHolders;

    // 是否处于启动菜单状态
    bool m_atStartMenu = true;

    // 中央对话框
    QFrame* m_dialogBox = nullptr;
    QTextBrowser* m_dialogText = nullptr;
    QFrame* m_choicesPanel = nullptr;
    QVBoxLayout* m_choicesLayout = nullptr;
    QVector<QPushButton*> m_choiceButtons;

    std::unique_ptr<GameState> m_state;
    std::unique_ptr<SceneEngine> m_engine;

public:
    GameState* state() { return m_state.get(); }
    SceneEngine* engine() { return m_engine.get(); }
};
