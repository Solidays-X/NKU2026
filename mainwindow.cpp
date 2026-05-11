#include "mainwindow.h"
#include "scenemanager.h"
#include "inventorywindow.h"

#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QFileDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QApplication>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    m_sceneManager(nullptr)
{
    setWindowTitle("循环校园 - LoopCampus");
    resize(900, 680);

    m_gameState = new GameState(this);

    // 日志窗口
    m_logWindow = new QTextEdit(nullptr);
    m_logWindow->setWindowTitle("剧情回看");
    m_logWindow->setReadOnly(true);
    m_logWindow->resize(600, 500);
    m_logWindow->setStyleSheet("QTextEdit { font-size: 12pt; background: #f5f5f5; }");

    // 背包窗口（非模态）
    m_inventoryWindow = new InventoryWindow(m_gameState, nullptr);
    m_inventoryWindow->setWindowTitle("背包");
    m_inventoryWindow->resize(300, 400);

    setupUI();
    setupStatusBar();
    connectSignals();

    updateStatusLabels();
}

MainWindow::~MainWindow()
{
    delete m_logWindow;
    delete m_inventoryWindow;
}

void MainWindow::setSceneManager(SceneManager *sm)
{
    m_sceneManager = sm;
}

void MainWindow::setupUI()
{
    QWidget *central = new QWidget(this);
    setCentralWidget(central);

    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 剧情显示区域
    m_storyDisplay = new StoryDisplay(central);
    m_storyDisplay->setMinimumHeight(400);

    // 选项区域
    m_optionsContainer = new QWidget(central);
    m_optionsLayout = new QVBoxLayout(m_optionsContainer);
    m_optionsLayout->setContentsMargins(0, 10, 0, 0);
    m_optionsContainer->setVisible(false);

    mainLayout->addWidget(m_storyDisplay, 1);
    mainLayout->addWidget(m_optionsContainer);

    // 菜单栏
    QMenu *fileMenu = menuBar()->addMenu("文件");
    QAction *saveAct = fileMenu->addAction("存档");
    QAction *loadAct = fileMenu->addAction("读档");
    QAction *logAct = fileMenu->addAction("回看剧情");
    QAction *exitAct = fileMenu->addAction("退出");

    connect(saveAct, &QAction::triggered, this, &MainWindow::saveGame);
    connect(loadAct, &QAction::triggered, this, &MainWindow::loadGame);
    connect(logAct, &QAction::triggered, this, &MainWindow::showBacklog);
    connect(exitAct, &QAction::triggered, this, &QWidget::close);
}

void MainWindow::setupStatusBar()
{
    // 左上角：时间 + 循环次数
    m_timeLabel = new QLabel(this);
    m_timeLabel->setStyleSheet("color: white; font-size: 14pt; background: rgba(0,0,0,100); padding: 4px;");
    m_timeLabel->move(10, 10);

    mLoopLabel = new QLabel(this);
    mLoopLabel->setStyleSheet("color: white; font-size: 12pt; background: rgba(0,0,0,100); padding: 4px;");
    mLoopLabel->move(10, 40);

    // 左下角：体力、精神、知识、身体素质
    m_statEnergy = new QLabel(this);
    m_statSanity = new QLabel(this);
    m_statKnowledge = new QLabel(this);
    m_statPhysical = new QLabel(this);

    QString statStyle = "color: white; font-size: 12pt; background: rgba(0,0,0,100); padding: 4px;";
    m_statEnergy->setStyleSheet(statStyle);
    m_statSanity->setStyleSheet(statStyle);
    m_statKnowledge->setStyleSheet(statStyle);
    m_statPhysical->setStyleSheet(statStyle);

    int baseY = height() - 140;
    int x = 10;
    m_statEnergy->move(x, baseY);
    m_statSanity->move(x, baseY + 25);
    m_statKnowledge->move(x, baseY + 50);
    m_statPhysical->move(x, baseY + 75);

    // 结局入口面板（左下角属性标签上方或下方，我们放在属性下方，即 baseY + 105）
    m_endingPanel = new QWidget(this);
    m_endingPanel->move(x, baseY + 105);
    m_endingLayout = new QHBoxLayout(m_endingPanel);
    m_endingLayout->setContentsMargins(0, 0, 0, 0);
    m_endingLayout->setSpacing(4);
    m_endingPanel->setVisible(false);

    // 背包按钮（放在右下角？文档未指定，可放在菜单栏或独立按钮。这里放在右下角固定位置）
    m_inventoryButton = new QPushButton("背包", this);
    m_inventoryButton->setStyleSheet(
        "QPushButton { background-color: #2e2e2e; color: white; border: 1px solid #555; "
        "font-size: 10pt; padding: 4px 8px; }"
        "QPushButton:hover { background-color: #444; }"
        );
    m_inventoryButton->move(width() - 80, height() - 40);
    connect(m_inventoryButton, &QPushButton::clicked, this, &MainWindow::openInventory);
}

void MainWindow::connectSignals()
{
    connect(m_gameState, &GameState::energyChanged, this, &MainWindow::updateStatusLabels);
    connect(m_gameState, &GameState::sanityChanged, this, &MainWindow::updateStatusLabels);
    connect(m_gameState, &GameState::knowledgeChanged, this, &MainWindow::updateStatusLabels);
    connect(m_gameState, &GameState::physicalChanged, this, &MainWindow::updateStatusLabels);
    connect(m_gameState, &GameState::loopCountChanged, this, &MainWindow::updateStatusLabels);
    connect(m_gameState, &GameState::dayChanged, this, &MainWindow::updateStatusLabels);
    connect(m_gameState, &GameState::slotChanged, this, &MainWindow::updateStatusLabels);

    // 结局永久解锁时刷新面板
    connect(m_gameState, &GameState::endingUnlocked, this, &MainWindow::updateEndingEntries);

    // 不在构造函数中连接 StoryDisplay，由 SceneManager 连接
}

void MainWindow::updateStatusLabels()
{
    QString dayStr = (m_gameState->currentDay() == DayOfWeek::Saturday) ? "周六" : "周日";
    QString slotStr;
    switch (m_gameState->currentSlot()) {
    case TimeSlot::Morning: slotStr = "早晨"; break;
    case TimeSlot::Forenoon: slotStr = "上午"; break;
    case TimeSlot::Noon: slotStr = "中午"; break;
    case TimeSlot::Afternoon: slotStr = "下午"; break;
    case TimeSlot::Evening: slotStr = "傍晚"; break;
    case TimeSlot::Night: slotStr = "夜晚"; break;
    }
    m_timeLabel->setText(QString("%1 %2").arg(dayStr, slotStr));
    mLoopLabel->setText(QString("循环次数：%1").arg(m_gameState->loopCount()));

    m_statEnergy->setText(QString("体力值：%1").arg(m_gameState->energy()));
    m_statSanity->setText(QString("精神稳定值：%1").arg(m_gameState->sanity()));
    m_statKnowledge->setText(QString("知识：%1").arg(m_gameState->knowledge()));
    m_statPhysical->setText(QString("身体素质：%1").arg(m_gameState->physical()));
}

void MainWindow::updateEndingEntries()
{
    // 清除旧按钮
    while (QLayoutItem *item = m_endingLayout->takeAt(0)) {
        if (QWidget *w = item->widget()) w->deleteLater();
        delete item;
    }
    m_endingButtons.clear();

    const QList<Ending> unlocked = m_gameState->unlockedEndings();
    for (Ending e : unlocked) {
        QString label;
        QString endingNode;
        if (e == Ending::BondSpark) {
            label = "羁绊·星火";
            endingNode = "ending_T1";
        } else if (e == Ending::TodayUntie) {
            label = "今日·解铃";
            endingNode = "ending_T2";
        } // 其他结局可扩展
        QPushButton *btn = new QPushButton(label, m_endingPanel);
        btn->setProperty("endingNode", endingNode);
        btn->setStyleSheet(
            "QPushButton { background-color: #3a3a3a; color: #e0e0e0; border: 1px solid #5a5a5a; "
            "font-size: 10pt; padding: 3px 6px; }"
            "QPushButton:hover { background-color: #555; }"
            );
        connect(btn, &QPushButton::clicked, this, &MainWindow::onEndingButtonClicked);
        m_endingLayout->addWidget(btn);
        m_endingButtons.append(btn);
    }
    m_endingPanel->setVisible(!unlocked.isEmpty());
}

// 结局按钮点击
void MainWindow::onEndingButtonClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    // 检查时间
    if (m_gameState->currentSlot() == TimeSlot::Night) {
        QMessageBox::information(this, "提示", "夜晚无法进入结局，请先休息。");
        return;
    }
    QString endingNode = btn->property("endingNode").toString();
    if (!endingNode.isEmpty() && m_sceneManager) {
        m_sceneManager->enterEnding(endingNode);
    }
}

void MainWindow::openInventory()
{
    m_inventoryWindow->show();
    m_inventoryWindow->raise();
}

void MainWindow::showStoryParagraphs(const QStringList &paragraphs)
{
    // 记录到日志
    for (const QString &p : paragraphs) {
        m_logWindow->append(p);
    }
    m_storyDisplay->setParagraphs(paragraphs);
    clearOptions();
}

void MainWindow::showStoryText(const QString &fullText)
{
    m_logWindow->append(fullText);
    m_storyDisplay->setFullText(fullText);
    clearOptions();
}

void MainWindow::setOptions(const QStringList &optionLabels)
{
    clearOptions();
    for (const QString &label : optionLabels) {
        QPushButton *btn = new QPushButton(label, m_optionsContainer);
        btn->setMinimumHeight(40);
        btn->setStyleSheet(
            "QPushButton { background-color: #2e2e2e; color: white; border: 1px solid #555; "
            "font-size: 12pt; padding: 8px; text-align: left; }"
            "QPushButton:hover { background-color: #444; }"
            );
        connect(btn, &QPushButton::clicked, this, &MainWindow::onOptionClicked);
        m_optionsLayout->addWidget(btn);
        m_optionButtons.append(btn);
    }
    m_optionsContainer->setVisible(true);
}

void MainWindow::clearOptions()
{
    m_optionsContainer->setVisible(false);
    while (QLayoutItem *item = m_optionsLayout->takeAt(0)) {
        if (QWidget *w = item->widget()) w->deleteLater();
        delete item;
    }
    m_optionButtons.clear();
}

void MainWindow::onStoryFinished()
{
    // 空实现，SceneManager 会处理
}

void MainWindow::onOptionClicked()
{
    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    if (!btn) return;
    int index = m_optionButtons.indexOf(btn);
    emit optionSelected(index);
    clearOptions();
}

void MainWindow::saveGame()
{
    QString fileName = QFileDialog::getSaveFileName(this, "存档", "", "JSON 文件 (*.json)");
    if (fileName.isEmpty()) return;
    QJsonObject saveObj = m_gameState->toJson();
    if (m_sceneManager) {
        saveObj["currentNode"] = m_sceneManager->currentNode;
        // 保存标记
        QJsonObject markersObj;
        markersObj["persistent"] = QJsonArray::fromStringList(QStringList(m_sceneManager->m_markers.values()));
        markersObj["daily"] = QJsonArray::fromStringList(QStringList(m_sceneManager->m_dailyMarkers.values()));
        markersObj["loop"] = QJsonArray::fromStringList(QStringList(m_sceneManager->m_loopMarkers.values()));
        markersObj["firstTime"] = QJsonArray::fromStringList(QStringList(m_sceneManager->m_firstTimeFlags.values()));
        saveObj["markers"] = markersObj;
    }
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(saveObj).toJson());
        file.close();
    } else {
        QMessageBox::warning(this, "存档失败", "无法写入文件。");
    }
}

void MainWindow::loadGame()
{
    QString fileName = QFileDialog::getOpenFileName(this, "读档", "", "JSON 文件 (*.json)");
    if (fileName.isEmpty()) return;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "读档失败", "无法打开文件。");
        return;
    }
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
    file.close();
    if (error.error != QJsonParseError::NoError) {
        QMessageBox::warning(this, "读档失败", "存档文件格式错误。");
        return;
    }
    QJsonObject saveObj = doc.object();
    m_gameState->fromJson(saveObj);
    updateStatusLabels();
    if (m_sceneManager && saveObj.contains("currentNode")) {
        // 恢复标记
        if (saveObj.contains("markers")) {
            QJsonObject markersObj = saveObj["markers"].toObject();
            auto setFromArray = [](QJsonArray arr, QSet<QString> &target) {
                target.clear();
                for (auto v : arr) target.insert(v.toString());
            };
            setFromArray(markersObj["persistent"].toArray(), m_sceneManager->m_markers);
            setFromArray(markersObj["daily"].toArray(), m_sceneManager->m_dailyMarkers);
            setFromArray(markersObj["loop"].toArray(), m_sceneManager->m_loopMarkers);
            setFromArray(markersObj["firstTime"].toArray(), m_sceneManager->m_firstTimeFlags);
        }
        m_sceneManager->restoreFromLoad(saveObj["currentNode"].toString());
    }
}

void MainWindow::showBacklog()
{
    m_logWindow->show();
    m_logWindow->raise();
}