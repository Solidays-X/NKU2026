#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QListWidget>
#include "gamestate.h"
#include "storydisplay.h"

class SceneManager;
class InventoryWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setSceneManager(SceneManager* sm);  // 注入场景管理器

    void showStoryParagraphs(const QStringList &paragraphs);
    void showStoryText(const QString &fullText);
    void setOptions(const QStringList &optionLabels);
    void clearOptions();

    StoryDisplay* storyDisplay() const { return m_storyDisplay; }

    GameState* gameState() const { return m_gameState; }

signals:
    void optionSelected(int index);
    void endingEntryRequested(const QString &endingNode); // 结局入口按钮点击

private slots:
    void onStoryFinished();
    void onOptionClicked();
    void saveGame();
    void loadGame();
    void showBacklog();
    void updateStatusLabels();
    void updateEndingEntries();  // 刷新结局入口按钮
    void onEndingButtonClicked(); // 结局入口按钮点击
    void openInventory();        // 打开背包窗口

private:
    void setupUI();
    void setupStatusBar();
    void connectSignals();

    // 状态栏左上/左下
    QLabel *m_timeLabel;
    QLabel *mLoopLabel;
    QLabel *m_statEnergy;
    QLabel *m_statSanity;
    QLabel *m_statKnowledge;
    QLabel *m_statPhysical;

    // 结局入口区域
    QWidget *m_endingPanel;
    QHBoxLayout *m_endingLayout;
    QList<QPushButton*> m_endingButtons;

    // 背包按钮
    QPushButton *m_inventoryButton;

    // 核心游戏状态
    GameState *m_gameState;

    // 场景管理器（用于进入结局等）
    SceneManager *m_sceneManager;

    // 剧情显示
    StoryDisplay *m_storyDisplay;

    // 选项区域
    QWidget *m_optionsContainer;
    QVBoxLayout *m_optionsLayout;
    QList<QPushButton*> m_optionButtons;

    // 日志窗口
    QTextEdit *m_logWindow;

    // 背包窗口
    InventoryWindow *m_inventoryWindow;
};

#endif // MAINWINDOW_H