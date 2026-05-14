#include "mainwindow.h"
#include "gamestate.h"
#include "scenes.h"
#include "audio.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QPainter>
#include <QResizeEvent>
#include <QFontDatabase>
#include <QTextBrowser>
#include <QScrollBar>
#include <QRandomGenerator>
#include <QTimer>
#include <QMouseEvent>
#include <QEvent>
#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QStandardPaths>
#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QDateTime>
#include <functional>
#include <memory>
#include <QScrollArea>

static QString kScrambleSrc =
    QStringLiteral("◢◣◤◥░▒▓▤▥▦▧▨▩░▒▓◯◇◆◊·•※╳╳▌▍▎");

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent) {
    setWindowTitle(QStringLiteral("南开大学循环校园 - 交互叙事游戏"));
    resize(1280, 800);
    setMinimumSize(960, 600);

    // 背景图
    m_bgPixmap.load(QStringLiteral(":/background.jpg"));
    if (m_bgPixmap.isNull()) {
        m_bgPixmap.load(QStringLiteral("assets/background.jpg"));
    }

    QWidget* central = new QWidget(this);
    setCentralWidget(central);
    central->setAttribute(Qt::WA_StyledBackground, false);

    buildHud();

    // 中央对话框
    m_dialogBox = new QFrame(central);
    m_dialogBox->setObjectName("dialogBox");
    m_dialogBox->setStyleSheet(
        "#dialogBox { background-color: rgba(0,0,0,170); border-radius: 14px;"
        " border: 1px solid rgba(255,255,255,40); }");

    QVBoxLayout* dialogLay = new QVBoxLayout(m_dialogBox);
    dialogLay->setContentsMargins(28, 24, 28, 24);
    dialogLay->setSpacing(18);

    // 使用 QTextBrowser 替代 QLabel+QScrollArea: 自带滚动 + 稳定 wordwrap
    m_dialogText = new QTextBrowser(m_dialogBox);
    m_dialogText->setFrameShape(QFrame::NoFrame);
    m_dialogText->setOpenExternalLinks(false);
    m_dialogText->setReadOnly(true);
    m_dialogText->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_dialogText->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_dialogText->setStyleSheet(
        "QTextBrowser {"
        " background: transparent;"
        " color: #f3f3f3;"
        " font-size: 18px;"
        " font-family: 'Microsoft YaHei','PingFang SC','Noto Sans CJK SC';"
        " border: none;"
        " padding: 0;"
        "}"
        "QScrollBar:vertical{background:rgba(255,255,255,20);width:8px;}"
        "QScrollBar::handle:vertical{background:rgba(255,255,255,100);border-radius:4px;}"
        "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical{height:0;}");
    // 让点击文字区域也能触发 "继续"
    m_dialogText->viewport()->installEventFilter(this);
    m_dialogText->viewport()->setCursor(Qt::PointingHandCursor);
    dialogLay->addWidget(m_dialogText, 1);

    m_choicesPanel = new QFrame(m_dialogBox);
    m_choicesPanel->setStyleSheet("background: transparent;");
    m_choicesLayout = new QVBoxLayout(m_choicesPanel);
    m_choicesLayout->setContentsMargins(0, 0, 0, 0);
    m_choicesLayout->setSpacing(10);
    dialogLay->addWidget(m_choicesPanel, 0);

    // 让整个对话框背景空白处也能点击触发 "继续"
    m_dialogBox->installEventFilter(this);

    // 状态对象 & 场景引擎
    m_state = std::make_unique<GameState>();
    m_engine = std::make_unique<SceneEngine>(static_cast<IGameView*>(this),
                                             m_state.get(), this);

    refreshHud();
    layoutChildren();

    // 背景音乐
    Audio::playBgm(QStringLiteral(":/bgm.mp3"), true);
    Audio::setVolume(45);

    // 启动菜单 (开始游戏 / 读档)
    showStartMenu();
}

void MainWindow::showStartMenu() {
    m_atStartMenu = true;
    clearChoices();
    m_pendingContinue = nullptr;
    refreshHud();  // 让按钮 enable 状态正确

    QString html = QStringLiteral(
        "<div style='text-align:center;'>"
        "<h1 style='color:#ffd97a; margin-top: 30px;'>南开大学循环校园</h1>"
        "<h2 style='color:#e0e0e0; margin-top: 6px;'>—— 交互叙事游戏 ——</h2>"
        "<p style='color:#bbb; margin-top: 40px; font-size:14px;'>"
        "    一个关于停滞、循环与告别的故事。<br/>"
        "    点击下方按钮开始你的周末。"
        "</p>"
        "</div>");
    setDialogText(html);

    QVector<Choice> choices;
    choices.append({QStringLiteral("开始游戏"), [this]() {
        m_atStartMenu = false;
        m_engine->startIntro();
    }});
    bool hasAny = false;
    for (int i = 1; i <= kSaveSlots; ++i) {
        if (QFile::exists(slotPath(i))) { hasAny = true; break; }
    }
    if (hasAny) {
        choices.append({QStringLiteral("继续游戏（读取存档）"), [this]() {
            // 把启动菜单先重建一遍 (showChoices 在点击时已经 clearChoices 了),
            // 这样玩家从读档面板"返回菜单"后还能看到按钮.
            showStartMenu();
            openLoadPanel(true);
        }});
    }
    choices.append({QStringLiteral("退出游戏"), [this]() {
        close();
    }});
    showChoices(QString(), choices);
}

MainWindow::~MainWindow() {
    Audio::stopBgm();
}

void MainWindow::buildHud() {
    QWidget* central = centralWidget();

    auto makeHudLabel = [&](const QString& obj) {
        auto* l = new QLabel(central);
        l->setObjectName(obj);
        l->setStyleSheet(
            "QLabel {"
            "  color: #ffffff;"
            "  background-color: rgba(0,0,0,140);"
            "  padding: 6px 12px; border-radius: 8px;"
            "  font-size: 15px;"
            "  font-family: 'Microsoft YaHei','PingFang SC';"
            "}");
        l->setAttribute(Qt::WA_TransparentForMouseEvents);
        return l;
    };

    m_timeLabel = makeHudLabel("hudTime");
    m_loopLabel = makeHudLabel("hudLoop");

    m_eLabel = makeHudLabel("hudE");
    m_sLabel = makeHudLabel("hudS");
    m_kLabel = makeHudLabel("hudK");
    m_qLabel = makeHudLabel("hudQ");

    auto makeTopBtn = [&](const QString& label) {
        auto* b = new QPushButton(central);
        b->setText(label);
        b->setCursor(Qt::PointingHandCursor);
        b->setStyleSheet(
            "QPushButton {"
            " color: #ffffff; background-color: rgba(0,0,0,160);"
            " border: 1px solid rgba(255,255,255,90); border-radius: 8px;"
            " padding: 6px 14px;"
            " font-size: 14px; font-family: 'Microsoft YaHei','PingFang SC';"
            "}"
            "QPushButton:hover { background-color: rgba(255,255,255,40); }");
        return b;
    };
    m_journalBtn  = makeTopBtn(QStringLiteral("图鉴 / 任务"));
    m_backpackBtn = makeTopBtn(QStringLiteral("背包"));
    m_saveBtn     = makeTopBtn(QStringLiteral("存档"));
    m_loadBtn     = makeTopBtn(QStringLiteral("读档"));
    m_logBtn      = makeTopBtn(QStringLiteral("剧情回看"));
    connect(m_journalBtn,  &QPushButton::clicked, this, &MainWindow::showJournal);
    connect(m_backpackBtn, &QPushButton::clicked, this, &MainWindow::showBackpack);
    connect(m_saveBtn,     &QPushButton::clicked, this, &MainWindow::openSavePanel);
    connect(m_loadBtn,     &QPushButton::clicked, this, [this]() { openLoadPanel(false); });
    connect(m_logBtn,      &QPushButton::clicked, this, &MainWindow::showLog);

    m_endingPanel = new QFrame(central);
    m_endingPanel->setStyleSheet("background: transparent;");
    m_endingPanelLayout = new QVBoxLayout(m_endingPanel);
    m_endingPanelLayout->setContentsMargins(0, 0, 0, 0);
    m_endingPanelLayout->setSpacing(6);
}

void MainWindow::refreshHud() {
    if (!m_state) return;
    m_timeLabel->setText(QStringLiteral("时间： %1").arg(m_state->timeDisplay()));
    m_loopLabel->setText(QStringLiteral("循环次数： %1").arg(m_state->L));

    // 体力低 -> 黄/红警告
    auto applyLowColor = [](QLabel* l, int val, int warnAt, int dangerAt) {
        QString base =
            "QLabel { color:%1; background-color: rgba(0,0,0,140);"
            " padding:6px 12px; border-radius:8px; font-size:15px;"
            " font-family:'Microsoft YaHei','PingFang SC'; }";
        QString color = "#ffffff";
        if (val <= dangerAt) color = "#ff8080";
        else if (val <= warnAt) color = "#ffcc66";
        l->setStyleSheet(base.arg(color));
    };

    m_eLabel->setText(QStringLiteral("体力值： %1").arg(m_state->E));
    applyLowColor(m_eLabel, m_state->E, 30, 15);

    // 精神稳定值: S<=0 时会触发乱码, 提前警告
    QString sExtra;
    if (m_state->S <= 0) sExtra = QStringLiteral("  (文字混乱)");
    else if (m_state->S <= 20) sExtra = QStringLiteral("  (即将混乱)");
    m_sLabel->setText(QStringLiteral("精神稳定值： %1%2")
                          .arg(m_state->S).arg(sExtra));
    applyLowColor(m_sLabel, m_state->S, 30, 15);

    m_kLabel->setText(QStringLiteral("知识： %1").arg(m_state->K));
    m_qLabel->setText(QStringLiteral("身体素质： %1").arg(m_state->Q));

    m_timeLabel->adjustSize();
    m_loopLabel->adjustSize();
    m_eLabel->adjustSize();
    m_sLabel->adjustSize();
    m_kLabel->adjustSize();
    m_qLabel->adjustSize();
    if (m_journalBtn) m_journalBtn->adjustSize();

    // 重建结局入口按钮：必须立即销毁旧 widget，否则 sizeHint 还包含旧 widget。
    QLayoutItem* item;
    while ((item = m_endingPanelLayout->takeAt(0)) != nullptr) {
        if (auto* w = item->widget()) { w->setParent(nullptr); delete w; }
        delete item;
    }

    if (!m_state->unlockedEndings.isEmpty()) {
        auto* hint = new QLabel(m_endingPanel);
        hint->setText(QStringLiteral("已解锁结局入口："));
        hint->setStyleSheet(
            "QLabel {"
            " color: #fffcdc;"
            " background-color: rgba(0,0,0,120);"
            " padding: 4px 10px; border-radius: 6px;"
            " font-size: 13px; font-family: 'Microsoft YaHei';"
            "}");
        m_endingPanelLayout->addWidget(hint);

        const QStringList endings = m_state->unlockedEndings.values();
        for (const QString& name : endings) {
            auto* btn = new QPushButton(m_endingPanel);
            btn->setText(QStringLiteral("【%1】").arg(name));
            btn->setCursor(Qt::PointingHandCursor);
            btn->setStyleSheet(
                "QPushButton {"
                " color: #fffcdc; background-color: rgba(0,0,0,140);"
                " border: 1px solid rgba(255,240,180,140); border-radius: 6px;"
                " padding: 4px 12px; font-family: 'Microsoft YaHei';"
                " font-size: 13px; text-align: left;"
                "}"
                "QPushButton:hover { background-color: rgba(255,240,180,40); }"
                "QPushButton:disabled { color: #888; border-color: #555; }");

            // 夜晚时段禁用
            const bool canUse = (m_state->slot != TimeSlot::Night);
            btn->setEnabled(canUse);
            const QString endingName = name;
            connect(btn, &QPushButton::clicked, this, [this, endingName]() {
                m_engine->enterEndingByName(endingName);
            });
            m_endingPanelLayout->addWidget(btn);
        }
    }

    layoutChildren();
}

void MainWindow::layoutChildren() {
    if (!centralWidget()) return;
    const int W = width();
    const int H = height();

    // 顶部左 HUD
    int x = 18, y = 14;
    m_timeLabel->move(x, y);
    m_loopLabel->move(x, y + m_timeLabel->height() + 6);

    // 顶部右 - 图鉴 / 背包 (从上往下)
    int rightX = W - 18;
    int topY = y;
    auto placeTopRight = [&](QPushButton* b) {
        if (!b) return;
        b->adjustSize();
        b->move(rightX - b->width(), topY);
        topY += b->height() + 6;
    };
    placeTopRight(m_journalBtn);
    placeTopRight(m_backpackBtn);

    // 底部右 - 存档 / 读档 / 剧情回看 (从下往上)
    int botY = H - 14;
    auto placeBottomRight = [&](QPushButton* b) {
        if (!b) return;
        b->adjustSize();
        botY -= b->height();
        b->move(rightX - b->width(), botY);
        botY -= 6;
    };
    placeBottomRight(m_logBtn);
    placeBottomRight(m_loadBtn);
    placeBottomRight(m_saveBtn);

    // 底部左 HUD
    int by = H - 14;
    QVector<QLabel*> bottom = {m_qLabel, m_kLabel, m_sLabel, m_eLabel};
    for (auto* l : bottom) {
        by -= l->height();
        l->move(x, by);
        by -= 6;
    }

    // 结局入口面板（顶部 HUD 下方）
    int epx = x;
    int epy = m_loopLabel->y() + m_loopLabel->height() + 10;
    int epw = qMax(220, m_loopLabel->width() + 80);
    int eph = m_endingPanel->sizeHint().height();
    if (eph < 8) eph = 0;
    m_endingPanel->setGeometry(epx, epy, epw, eph);

    // 中央对话框 - 四周留出空间
    int hMargin = qMax(120, W / 8);
    int vMargin = qMax(90,  H / 8);
    int dx = hMargin;
    int dy = vMargin;
    int dw = W - 2 * hMargin;
    int dh = H - 2 * vMargin;
    m_dialogBox->setGeometry(dx, dy, dw, dh);
}

void MainWindow::resizeEvent(QResizeEvent* e) {
    QMainWindow::resizeEvent(e);
    if (!m_bgPixmap.isNull()) {
        m_bgScaled = m_bgPixmap.scaled(size(), Qt::KeepAspectRatioByExpanding,
                                       Qt::SmoothTransformation);
    }
    layoutChildren();
    update();
}

void MainWindow::paintEvent(QPaintEvent* e) {
    {
        QPainter p(this);
        if (!m_bgScaled.isNull()) {
            QSize sz = m_bgScaled.size();
            QPoint origin((width() - sz.width()) / 2, (height() - sz.height()) / 2);
            p.drawPixmap(origin, m_bgScaled);
        } else {
            p.fillRect(rect(), QColor(40, 40, 50));
        }
    } // QPainter 必须先析构, 再交给基类
    QMainWindow::paintEvent(e);
}

QString MainWindow::styleSheetForButton() const {
    return QStringLiteral(
        "QPushButton {"
        "  color: #ffffff;"
        "  background-color: rgba(255,255,255,28);"
        "  border: 1px solid rgba(255,255,255,90);"
        "  border-radius: 8px;"
        "  padding: 10px 18px;"
        "  font-size: 16px;"
        "  font-family: 'Microsoft YaHei','PingFang SC';"
        "  text-align: left;"
        "}"
        "QPushButton:hover { background-color: rgba(255,255,255,72); }"
        "QPushButton:pressed { background-color: rgba(255,255,255,110); }");
}

void MainWindow::appendToLog(const QString& text) {
    if (text.trimmed().isEmpty()) return;
    m_storyLog.append(text);
    while (m_storyLog.size() > kMaxLog) m_storyLog.removeFirst();
}

QString MainWindow::stripChoiceNumber(const QString& s) {
    // 匹配开头形式: "1. xxx"、"1.xxx"、"（1）xxx"、"(1) xxx"、"A. xxx"、"a. xxx"
    static const QRegularExpression re(
        QStringLiteral("^\\s*(?:[（(]\\s*[0-9０-９a-zA-Z]+\\s*[)）]|"
                       "[0-9０-９a-zA-Z]+\\s*[\\.、．])\\s*"));
    QString out = s;
    out.remove(re);
    return out;
}

QString MainWindow::applyScramble(const QString& s) const {
    if (!m_state || !m_state->sanityScrambled()) return s;
    QString out;
    out.reserve(s.size());
    auto* rng = QRandomGenerator::global();
    for (QChar ch : s) {
        if (ch.isSpace() || ch == '\n' || ch == '<' || ch == '>' || ch == '/' ||
            ch == '"' || ch == '=' || ch == ';' || ch == '\'') {
            // 保留排版/HTML 字符
            out.append(ch);
        } else {
            out.append(kScrambleSrc.at(rng->bounded(kScrambleSrc.size())));
        }
    }
    return out;
}

void MainWindow::clearChoices() {
    for (auto* btn : m_choiceButtons) {
        btn->deleteLater();
    }
    m_choiceButtons.clear();
}

void MainWindow::scrollToTop() {
    if (!m_dialogText) return;
    auto* bar = m_dialogText->verticalScrollBar();
    bar->setValue(0);
    // 在 layout/wordwrap 稳定后再回到顶部一次, 避免首屏停在中间
    QTimer::singleShot(0, this, [this]() {
        if (m_dialogText) m_dialogText->verticalScrollBar()->setValue(0);
    });
}

// 统一的文本设置函数: 包一层 div, 增加行高/段距, 保证 wordwrap 正常工作
void MainWindow::setDialogText(const QString& html) {
    QString wrapped = QStringLiteral(
        "<div style='line-height:160%;'>%1</div>").arg(html);
    m_dialogText->setHtml(wrapped);
    scrollToTop();
}

void MainWindow::setPrompt(const QString& text) {
    setDialogText(applyScramble(text));
}

// 把长文本拆成多页 (按 <br/> / \n 切分, 拼到接近阈值时分页)
static QStringList splitToPages(const QString& text, int charsPerPage = 280) {
    // 兼容多种段落分隔
    QString norm = text;
    norm.replace("<br/>", "\n");
    norm.replace("<br>",  "\n");
    norm.replace("<br />","\n");
    const QStringList paragraphs = norm.split('\n');
    QStringList pages;
    QString cur;
    auto plainLen = [](const QString& s) {
        // 粗略估算: HTML 标签不计入字符数
        QString t = s;
        t.remove(QRegularExpression("<[^>]+>"));
        return t.size();
    };
    for (const QString& p : paragraphs) {
        if (!cur.isEmpty() && plainLen(cur) + plainLen(p) > charsPerPage) {
            pages << cur;
            cur.clear();
        }
        if (!cur.isEmpty()) cur += QStringLiteral("<br/>");
        cur += p;
    }
    if (!cur.isEmpty()) pages << cur;
    if (pages.isEmpty()) pages << text;
    return pages;
}

void MainWindow::showNarration(const QString& text, std::function<void()> cont) {
    if (!m_atStartMenu) {
        appendToLog(text);
        m_lastDialogText = text;
        m_lastWasChoices = false;
        m_lastChoiceLabels.clear();
    }
    m_narrationHolders.clear();  // 释放旧的 holder, 打破循环引用
    QStringList pages = splitToPages(text);

    // 用 shared_ptr 包装递归 lambda, 让自身的生命周期足够长
    // 在最后一页绑定 cont 之后, 手动把内部引用清掉, 打破循环引用避免泄漏
    auto holder = std::make_shared<std::function<void(int)>>();
    std::weak_ptr<std::function<void(int)>> weak = holder;
    *holder = [this, pages, cont, weak](int idx) {
        clearChoices();
        const bool isLast = (idx == pages.size() - 1);
        setDialogText(applyScramble(pages[idx]));
        auto* btn = new QPushButton(m_choicesPanel);
        if (!isLast) {
            m_pendingContinue = [weak, idx]() {
                if (auto sp = weak.lock()) (*sp)(idx + 1);
            };
            btn->setText(QStringLiteral("继续 ↓ (%1/%2, 或点击对话框任意位置)")
                             .arg(idx + 1).arg(pages.size()));
        } else {
            m_pendingContinue = cont;
            QString label = (pages.size() == 1)
                ? QStringLiteral("继续 → (或点击对话框任意位置)")
                : QStringLiteral("继续 → (%1/%2, 或点击对话框任意位置)")
                      .arg(idx + 1).arg(pages.size());
            btn->setText(label);
        }
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(styleSheetForButton());
        connect(btn, &QPushButton::clicked, this, [this]() {
            auto cb = m_pendingContinue;
            m_pendingContinue = nullptr;
            clearChoices();
            if (cb) cb();
        });
        m_choicesLayout->addWidget(btn);
        m_choiceButtons.append(btn);
    };
    m_narrationHolders.append(holder);
    (*holder)(0);
}

void MainWindow::showChoices(const QString& text, const QVector<Choice>& choices) {
    clearChoices();
    m_pendingContinue = nullptr;
    m_narrationHolders.clear();
    setDialogText(applyScramble(text));
    if (!m_atStartMenu) {
        appendToLog(text);
        m_lastDialogText = text;
        m_lastWasChoices = true;
        m_lastChoiceLabels.clear();
        for (const Choice& c : choices) {
            m_lastChoiceLabels << stripChoiceNumber(c.text);
        }
    }
    for (const Choice& c : choices) {
        auto* btn = new QPushButton(m_choicesPanel);
        const QString labelStripped = stripChoiceNumber(c.text);
        btn->setText(applyScramble(labelStripped));
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(styleSheetForButton());
        auto fn = c.onClick;
        const QString logLabel = labelStripped;
        connect(btn, &QPushButton::clicked, this, [this, fn, logLabel]() {
            if (!m_atStartMenu) {
                appendToLog(QStringLiteral("→ %1").arg(logLabel));
            }
            clearChoices();
            if (fn) fn();
        });
        m_choicesLayout->addWidget(btn);
        m_choiceButtons.append(btn);
    }
}

void MainWindow::showEnding(const QString& title, const QString& body) {
    clearChoices();
    m_narrationHolders.clear();
    // 把"已经游玩到过的结局"记录到图鉴, 包括 B1/B2/Hidden, 持久化
    if (m_state) {
        m_state->seenEndings.insert(title);
    }
    m_pendingContinue = nullptr;

    // 1) 把 body 拆成"段落组" (每页). 优先按 "\n\n" 切段, 每页装若干段以填满对话框.
    // 这样长结局 (如 T1/T2/隐藏结局) 就不会一次性挤进滚动条里看不完.
    auto splitPages = [](const QString& text) -> QStringList {
        // 先按双换行切自然段
        QStringList paragraphs = text.split(QStringLiteral("\n\n"), Qt::SkipEmptyParts);
        // 如果没有双换行, 退化为按单换行
        if (paragraphs.size() <= 1) {
            paragraphs = text.split('\n', Qt::SkipEmptyParts);
        }
        // 每页大约 280 个汉字, 段落较短时合并, 较长时各自一页
        const int kCharsPerPage = 280;
        QStringList pages;
        QString cur;
        int curLen = 0;
        for (const QString& p : paragraphs) {
            const QString trimmed = p.trimmed();
            if (trimmed.isEmpty()) continue;
            // 用"——"开头 (分隔小节) 时强制换页
            const bool isSection = trimmed.startsWith(QStringLiteral("——"));
            if (isSection && !cur.isEmpty()) {
                pages << cur;
                cur.clear();
                curLen = 0;
            }
            if (!cur.isEmpty()) cur += QStringLiteral("\n\n");
            cur += trimmed;
            curLen += trimmed.size();
            if (curLen >= kCharsPerPage) {
                pages << cur;
                cur.clear();
                curLen = 0;
            }
        }
        if (!cur.isEmpty()) pages << cur;
        if (pages.isEmpty()) pages << text;
        return pages;
    };

    auto pages = std::make_shared<QStringList>(splitPages(body));
    auto pageIndex = std::make_shared<int>(0);

    // 渲染指定页: 第一页带标题, 末页才出现"重新开始"按钮, 否则是"下一页"
    auto render = std::make_shared<std::function<void()>>();
    *render = [this, title, pages, pageIndex, render]() {
        clearChoices();
        const int total = pages->size();
        const int idx = *pageIndex;
        const bool isFirst = (idx == 0);
        const bool isLast  = (idx == total - 1);

        QString pageHtml = pages->at(idx).toHtmlEscaped().replace("\n", "<br/>");
        QString html;
        if (isFirst) {
            html = QStringLiteral(
                "<div style='text-align:center;'>"
                "<h2 style='color:#ffd97a;'>%1</h2>"
                "</div>"
                "<div style='text-align:left; margin-top:10px;'>%2</div>")
                .arg(title.toHtmlEscaped(), pageHtml);
        } else {
            html = QStringLiteral(
                "<div style='text-align:left;'>%1</div>")
                .arg(pageHtml);
        }
        if (isLast) {
            html += QStringLiteral(
                "<p style='text-align:center; margin-top:18px; color:#ffd97a;'>—— 完 ——</p>");
        }
        // 右下角页码提示
        if (total > 1) {
            html += QStringLiteral(
                "<p style='text-align:right; margin-top:8px; color:#9c9c9c;"
                " font-size:12px;'>第 %1 / %2 页</p>")
                .arg(idx + 1).arg(total);
        }
        setDialogText(html);

        auto* btn = new QPushButton(m_choicesPanel);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(styleSheetForButton());
        if (!isLast) {
            btn->setText(QStringLiteral("下一页 ▶"));
            connect(btn, &QPushButton::clicked, this, [pageIndex, render]() {
                ++(*pageIndex);
                (*render)();
            });
        } else {
            btn->setText(QStringLiteral("重新开始"));
            connect(btn, &QPushButton::clicked, this, [this]() {
                clearChoices();
                QTimer::singleShot(0, this, [this]() {
                    QSet<QString> seen = m_state ? m_state->seenEndings : QSet<QString>();
                    m_engine.reset();
                    m_state.reset(new GameState());
                    m_state->seenEndings = seen;
                    m_engine.reset(new SceneEngine(static_cast<IGameView*>(this),
                                                   m_state.get(), this));
                    refreshHud();
                    m_engine->startIntro();
                });
            });
        }
        m_choicesLayout->addWidget(btn);
        m_choiceButtons.append(btn);
    };
    (*render)();
}

// 道具/任务/裂缝/结局图鉴
void MainWindow::showJournal() {
    if (!m_state) return;

    // 构造 HTML 内容
    auto htmlEscape = [](const QString& s) { return s.toHtmlEscaped(); };

    // 1) 当前持有道具
    QStringList itemRows;
    static const QVector<QPair<QString, QString>> itemRefs = {
        {QStringLiteral("学子卡"), QStringLiteral("宿舍桌前取走, 进入大通琴房需要")},
        {QStringLiteral("心愿卡"), QStringLiteral("海棠树下铁盒内 (需 K>20 或 Q>20)")},
        {QStringLiteral("一段钢琴录音"), QStringLiteral("琴房弹奏后获得")},
        {QStringLiteral("阿姨的手帕"), QStringLiteral("L≥8 后在操场探索捡到")},
        {QStringLiteral("陈旧的作业纸"), QStringLiteral("公教楼学习时随机获得")},
        {QStringLiteral("科幻小说书摘"), QStringLiteral("图书馆学习时随机获得")},
    };
    for (const auto& ref : itemRefs) {
        const QString& name = ref.first;
        const QString& hint = ref.second;
        bool owned = m_state->hasItem(name);
        itemRows << QStringLiteral(
            "<tr><td style='padding:4px 24px 4px 0;'>%1</td>"
            "<td style='padding:4px 0; color:%2;'>%3</td></tr>")
            .arg(htmlEscape(name),
                 owned ? "#9be37e" : "#777",
                 owned ? QStringLiteral("已持有") : QStringLiteral("—"));
        (void)hint;
    }

    // 2) 任务
    static const QVector<QPair<QString, QString>> taskRefs = {
        {QStringLiteral("食堂阿姨的手帕"),
         QStringLiteral("L≥8 后, 找食堂阿姨问起 → 操场拾到手帕 → 交还给阿姨")},
        {QStringLiteral("琴房的心愿卡"),
         QStringLiteral("海棠铁盒取卡 → 大通琴房弹奏 → 宿舍发帖 → 收到私信回复")},
        {QStringLiteral("陪猫猫看日落"),
         QStringLiteral("傍晚在公教楼 / 图书馆 / 操场三个地点都坐过去陪猫")},
    };
    QStringList taskRows;
    for (const auto& ref : taskRefs) {
        const QString& name = ref.first;
        bool done = m_state->completedTasks.contains(name);
        taskRows << QStringLiteral(
            "<tr><td style='padding:4px 24px 4px 0;'>%1</td>"
            "<td style='padding:4px 0; color:%2;'>%3</td></tr>")
            .arg(htmlEscape(name),
                 done ? "#9be37e" : "#aaa",
                 done ? QStringLiteral("已完成") : QStringLiteral("未完成"));
    }

    // 3) 世界裂缝
    static const QVector<QPair<QString, QString>> crackRefs = {
        {QStringLiteral("公教楼连廊的人影"),
         QStringLiteral("到处看看 → 公教楼连廊 (本循环触发, 仅一次)")},
        {QStringLiteral("一张空白的纸"),
         QStringLiteral("宿舍 → 桌前 → 一张空白的纸")},
        {QStringLiteral("阅览桌上的测试记录"),
         QStringLiteral("图书馆 → 探索 (需 K>30 且完成手帕任务)")},
    };
    QStringList crackRows;
    for (const auto& ref : crackRefs) {
        const QString& name = ref.first;
        bool got = m_state->worldCracks.contains(name);
        crackRows << QStringLiteral(
            "<tr><td style='padding:4px 24px 4px 0;'>%1</td>"
            "<td style='padding:4px 0; color:%2;'>%3</td></tr>")
            .arg(htmlEscape(name),
                 got ? "#9be37e" : "#aaa",
                 got ? QStringLiteral("已收集") : QStringLiteral("未收集"));
    }

    // 4) 结局成就
    struct EndingRef { QString fullTitle; QString hint; };
    static const QVector<EndingRef> endingRefs = {
        {QStringLiteral("结局 B1：困缚·明日何其多"), QStringLiteral("反复做作业, N > 19")},
        {QStringLiteral("结局 B2：沉溺·长眠"),       QStringLiteral("体力 ≤ 0 / 沉沦 M > 7 / L > 52")},
        {QStringLiteral("结局 T1：羁绊·星火"),       QStringLiteral("完成 三个 T 任务 (手帕 / 心愿卡 / 陪猫)")},
        {QStringLiteral("结局 T2：今日·解铃"),       QStringLiteral("周日夜晚, 持有作业纸 + 书摘 时入睡")},
        {QStringLiteral("隐藏结局：茧与蝶 · 成蝶"),  QStringLiteral("解锁 T1 + T2 后, 集齐 3 个世界裂缝, 周日夜晚 → 马蹄湖 → 敲下结局")},
        {QStringLiteral("隐藏结局：茧与蝶 · 共振"),  QStringLiteral("同上, 但在马蹄湖选择 写下留言")},
    };
    QStringList endingRows;
    for (const auto& ref : endingRefs) {
        bool seen = m_state->seenEndings.contains(ref.fullTitle);
        endingRows << QStringLiteral(
            "<tr><td style='padding:4px 24px 4px 0;'>%1</td>"
            "<td style='padding:4px 0; color:%2;'>%3</td></tr>")
            .arg(htmlEscape(ref.fullTitle),
                 seen ? "#ffd97a" : "#666",
                 seen ? QStringLiteral("★ 已达成") : QStringLiteral("未达成"));
    }

    QString section = QStringLiteral(
        "<h3 style='color:#ffd97a; margin:14px 0 6px 0;'>%1</h3>"
        "<table style='border-collapse:collapse;'>%2</table>");
    QString html = QStringLiteral(
        "<div style='color:#eee; font-family:Microsoft YaHei;'>"
        "<h2 style='color:#ffd97a; margin-top:0;'>图鉴 / 任务清单</h2>"
        "%1%2%3%4"
        "</div>")
        .arg(section.arg(QStringLiteral("关键道具"), itemRows.join(QString())),
             section.arg(QStringLiteral("任务"),     taskRows.join(QString())),
             section.arg(QStringLiteral("世界裂缝"), crackRows.join(QString())),
             section.arg(QStringLiteral("结局"),     endingRows.join(QString())));

    QDialog* dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(QStringLiteral("图鉴 / 任务"));
    dlg->resize(720, 600);
    dlg->setStyleSheet("QDialog { background-color: #1a1a22; }");

    auto* lay = new QVBoxLayout(dlg);
    lay->setContentsMargins(18, 18, 18, 18);

    auto* view = new QTextBrowser(dlg);
    view->setFrameShape(QFrame::NoFrame);
    view->setOpenExternalLinks(false);
    view->setStyleSheet(
        "QTextBrowser { background: transparent; color:#eee; font-size:14px;"
        " border: none; padding: 0; }");
    view->setHtml(html);
    lay->addWidget(view, 1);

    auto* close = new QPushButton(QStringLiteral("关闭"), dlg);
    close->setCursor(Qt::PointingHandCursor);
    close->setStyleSheet(styleSheetForButton());
    connect(close, &QPushButton::clicked, dlg, &QDialog::accept);
    lay->addWidget(close, 0);

    dlg->show();
}

// 背包 - 显示所有当前持有的物品/道具; 消耗品可点击"使用"
void MainWindow::showBackpack() {
    if (!m_state) return;

    auto* dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(QStringLiteral("背包"));
    dlg->resize(560, 560);
    // 整个对话框统一深底浅字; 子 widget 默认透明,
    // 用 .ItemRow / .EmptyHint 这种 class 选择器给特定 QFrame/QLabel 上色
    dlg->setStyleSheet(
        "QDialog { background-color: #1a1a22; }"
        "QFrame#ItemRow {"
        "  background-color: #2a2a36;"
        "  border: 1px solid #4a4a5a; border-radius: 8px;"
        "}"
        "QLabel { background: transparent; color: #eee; }"
        "QScrollArea { background: transparent; border: none; }"
        "QScrollArea > QWidget > QWidget { background: transparent; }");
    auto* lay = new QVBoxLayout(dlg);
    lay->setContentsMargins(18, 18, 18, 18);
    lay->setSpacing(8);

    auto* title = new QLabel(QStringLiteral(
        "<h2 style='color:#ffd97a; margin:0;'>背包</h2>"
        "<p style='color:#aaa; margin-top:6px; font-size:13px;'>"
        "标 <span style='color:#9be37e;'>★</span> 的是消耗品，点「使用」可立即生效（用完即消失）。"
        "</p>"), dlg);
    title->setTextFormat(Qt::RichText);
    lay->addWidget(title);

    // 滚动区域容纳物品行
    auto* listWrap = new QWidget(dlg);
    auto* listLay = new QVBoxLayout(listWrap);
    listLay->setContentsMargins(0, 0, 0, 0);
    listLay->setSpacing(6);

    QStringList list = m_state->items.values();
    std::sort(list.begin(), list.end());
    if (list.isEmpty()) {
        auto* empty = new QLabel(QStringLiteral(
            "<p style='color:#888;'>（背包是空的。在校园各处探索能捡到东西。）</p>"),
            listWrap);
        empty->setTextFormat(Qt::RichText);
        listLay->addWidget(empty);
    } else {
        const auto& tbl = GameState::consumableTable();
        for (const QString& name : list) {
            bool isCons = tbl.contains(name);
            auto* row = new QFrame(listWrap);
            row->setObjectName("ItemRow");
            row->setAttribute(Qt::WA_StyledBackground, true);
            auto* h = new QHBoxLayout(row);
            h->setContentsMargins(12, 8, 12, 8);

            QString star = isCons ? QStringLiteral(
                "<span style='color:#9be37e; font-size:16px;'>★</span> ") : QString();
            QString hintText;
            if (isCons) {
                const auto& eff = tbl.value(name);
                QStringList parts;
                if (eff.dE) parts << QStringLiteral("体力 %1%2")
                                         .arg(eff.dE > 0 ? "+" : "").arg(eff.dE);
                if (eff.dS) parts << QStringLiteral("精神 %1%2")
                                         .arg(eff.dS > 0 ? "+" : "").arg(eff.dS);
                if (eff.dK) parts << QStringLiteral("知识 %1%2")
                                         .arg(eff.dK > 0 ? "+" : "").arg(eff.dK);
                if (eff.dQ) parts << QStringLiteral("身体素质 %1%2")
                                         .arg(eff.dQ > 0 ? "+" : "").arg(eff.dQ);
                hintText = QStringLiteral(
                    "<br/><span style='color:#bbb; font-size:13px;'>%1</span>")
                    .arg(parts.join(QStringLiteral("，")));
            }
            auto* lbl = new QLabel(
                QStringLiteral("%1<b style='color:#fff;'>%2</b>%3")
                    .arg(star, name.toHtmlEscaped(), hintText), row);
            lbl->setTextFormat(Qt::RichText);
            h->addWidget(lbl, 1);

            if (isCons) {
                auto* btn = new QPushButton(QStringLiteral("使用"), row);
                btn->setCursor(Qt::PointingHandCursor);
                btn->setStyleSheet(styleSheetForButton() +
                    "QPushButton:disabled { color:#666; }");
                // 仅当目前处于"选项等待"状态时(即停留在某个主场景的菜单), 才能安全打断
                // 当前在 narration 中段 (m_pendingContinue 有值) 不允许使用
                const bool canUse = m_lastWasChoices && !m_pendingContinue;
                btn->setEnabled(canUse);
                if (!canUse) {
                    btn->setToolTip(QStringLiteral(
                        "请先把当前的叙述读完, 回到选项菜单再使用消耗品。"));
                }
                const QString itemName = name;
                connect(btn, &QPushButton::clicked, dlg, [this, itemName, dlg]() {
                    const auto& tbl2 = GameState::consumableTable();
                    auto it = tbl2.find(itemName);
                    if (it == tbl2.end() || !m_state) return;
                    m_state->E += it->dE;
                    m_state->S += it->dS;
                    m_state->K += it->dK;
                    m_state->Q += it->dQ;
                    m_state->clampStats();
                    m_state->removeItem(itemName);
                    refreshHud();
                    QString useText = it->useText;
                    dlg->accept();
                    if (m_engine) {
                        showNarration(useText, [this]() {
                            if (m_engine) m_engine->resumeByCheckpoint();
                        });
                    }
                });
                h->addWidget(btn, 0);
            }
            listLay->addWidget(row);
        }
    }
    listLay->addStretch(1);

    auto* scroll = new QScrollArea(dlg);
    scroll->setFrameShape(QFrame::NoFrame);
    scroll->setStyleSheet("QScrollArea { background: transparent; }");
    scroll->setWidgetResizable(true);
    scroll->setWidget(listWrap);
    lay->addWidget(scroll, 1);

    auto* close = new QPushButton(QStringLiteral("关闭"), dlg);
    close->setCursor(Qt::PointingHandCursor);
    close->setStyleSheet(styleSheetForButton());
    connect(close, &QPushButton::clicked, dlg, &QDialog::accept);
    lay->addWidget(close, 0);
    dlg->show();
}

// 剧情回看 - 显示最近的对话/选择日志
void MainWindow::showLog() {
    QString rows;
    if (m_storyLog.isEmpty()) {
        rows = QStringLiteral("<p style='color:#888;'>（还没有可回看的剧情。）</p>");
    } else {
        QStringList items;
        for (const QString& t : m_storyLog) {
            // 直接把 narration 的 HTML 嵌进去, 用 hr 分隔
            items << QStringLiteral(
                "<div style='padding:6px 0; border-bottom:1px dashed #444;'>%1</div>")
                .arg(t);
        }
        rows = items.join(QString());
    }
    QString html = QStringLiteral(
        "<div style='color:#eee; font-family:Microsoft YaHei; line-height:160%;'>"
        "<h2 style='color:#ffd97a; margin-top:0;'>剧情回看</h2>"
        "<p style='color:#aaa; margin-top:0; font-size:13px;'>"
        "最近 %1 条对话与选择（最多保留 %2 条）。</p>%3"
        "</div>")
        .arg(m_storyLog.size()).arg(kMaxLog).arg(rows);

    auto* dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(QStringLiteral("剧情回看"));
    dlg->resize(760, 640);
    dlg->setStyleSheet("QDialog { background-color: #1a1a22; }");
    auto* lay = new QVBoxLayout(dlg);
    lay->setContentsMargins(18, 18, 18, 18);
    auto* view = new QTextBrowser(dlg);
    view->setFrameShape(QFrame::NoFrame);
    view->setStyleSheet(
        "QTextBrowser { background: transparent; color:#eee; font-size:14px;"
        " border: none; padding: 0; }");
    view->setHtml(html);
    lay->addWidget(view, 1);
    auto* close = new QPushButton(QStringLiteral("关闭"), dlg);
    close->setCursor(Qt::PointingHandCursor);
    close->setStyleSheet(styleSheetForButton());
    connect(close, &QPushButton::clicked, dlg, &QDialog::accept);
    lay->addWidget(close, 0);
    dlg->show();
    // 滚动到底部
    QTimer::singleShot(0, this, [view]() {
        view->verticalScrollBar()->setValue(view->verticalScrollBar()->maximum());
    });
}

// ===== 多槽存档 =====
static QJsonArray setToArray(const QSet<QString>& s) {
    QJsonArray a;
    for (const auto& x : s) a.append(x);
    return a;
}
static QSet<QString> arrayToSet(const QJsonArray& a) {
    QSet<QString> s;
    for (const auto& v : a) s.insert(v.toString());
    return s;
}

QString MainWindow::savesDir() const {
    QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    if (dir.isEmpty()) dir = QDir::currentPath();
    QDir().mkpath(dir);
    return dir;
}

QString MainWindow::slotPath(int slot) const {
    return savesDir() + QStringLiteral("/save_%1.json").arg(slot);
}

bool MainWindow::saveToSlot(int slot, QString* err) {
    if (slot < 1 || slot > kSaveSlots) { if (err) *err = "槽位无效"; return false; }
    if (!m_state) { if (err) *err = "当前没有游戏状态"; return false; }
    QJsonObject root;
    // 基本属性
    root["E"] = m_state->E; root["S"] = m_state->S;
    root["K"] = m_state->K; root["Q"] = m_state->Q;
    root["L"] = m_state->L; root["M"] = m_state->M; root["N"] = m_state->N;
    root["day"]  = (int)m_state->day;
    root["slot"] = (int)m_state->slot;
    root["sceneCheckpoint"] = m_state->sceneCheckpoint;
    // 集合字段
    root["items"]                  = setToArray(m_state->items);
    root["completedTasks"]         = setToArray(m_state->completedTasks);
    root["tasksCompletedThisLoop"] = setToArray(m_state->tasksCompletedThisLoop);
    root["worldCracks"]            = setToArray(m_state->worldCracks);
    root["unlockedEndings"]        = setToArray(m_state->unlockedEndings);
    root["seenEndings"]            = setToArray(m_state->seenEndings);
    // 持久 bool
    root["carryingStudentCard"]       = m_state->carryingStudentCard;
    root["firstHaitangBoxOpened"]     = m_state->firstHaitangBoxOpened;
    root["haitangBoxTakenCard"]       = m_state->haitangBoxTakenCard;
    root["everCompletedHandkerchief"] = m_state->everCompletedHandkerchief;
    root["everPublishedRecording"]    = m_state->everPublishedRecording;
    root["everReceivedReply"]         = m_state->everReceivedReply;
    root["catTeachingSat"]   = m_state->catTeachingSat;
    root["catLibrarySat"]    = m_state->catLibrarySat;
    root["catPlaygroundSat"] = m_state->catPlaygroundSat;
    root["teachingBbDone"]   = m_state->teachingBbDone;
    root["libraryBbDone"]    = m_state->libraryBbDone;
    root["libraryTestDone"]  = m_state->libraryTestDone;
    root["canteenBDone"]     = m_state->canteenBDone;
    root["canteenCDone"]     = m_state->canteenCDone;
    root["corridorShadowDone"] = m_state->corridorShadowDone;
    root["blankPaperTaken"]    = m_state->blankPaperTaken;
    root["storyAAA_done"]      = m_state->storyAAA_done;
    root["inDormTonight"]      = m_state->inDormTonight;
    root["nightChoiceMadeThisLoop"] = m_state->nightChoiceMadeThisLoop;
    root["canteenAuntUnlocked"]= m_state->canteenAuntUnlocked;
    // 回看日志
    QJsonArray logArr;
    for (const QString& s : m_storyLog) logArr.append(s);
    root["storyLog"] = logArr;
    // 存档点的对话快照 (读档后直接呈现给玩家)
    root["lastDialogText"]   = m_lastDialogText;
    root["lastWasChoices"]   = m_lastWasChoices;
    QJsonArray labels;
    for (const QString& s : m_lastChoiceLabels) labels.append(s);
    root["lastChoiceLabels"] = labels;
    // 元信息
    root["saveTime"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm");
    root["display"]  = QStringLiteral("L=%1 · %2 %3").arg(m_state->L)
                          .arg(m_state->dayName()).arg(m_state->slotName());

    QFile f(slotPath(slot));
    if (!f.open(QIODevice::WriteOnly)) {
        if (err) *err = QStringLiteral("无法写入: %1").arg(f.fileName());
        return false;
    }
    f.write(QJsonDocument(root).toJson());
    f.close();
    return true;
}

bool MainWindow::loadFromSlot(int slot, QString* err) {
    if (slot < 1 || slot > kSaveSlots) { if (err) *err = "槽位无效"; return false; }
    QFile f(slotPath(slot));
    if (!f.exists() || !f.open(QIODevice::ReadOnly)) {
        if (err) *err = "存档不存在";
        return false;
    }
    QJsonDocument doc = QJsonDocument::fromJson(f.readAll());
    f.close();
    if (!doc.isObject()) { if (err) *err = "存档已损坏"; return false; }
    QJsonObject root = doc.object();

    // 释放旧的 engine (持有指向 state 的裸指针)
    m_engine.reset();
    m_narrationHolders.clear();
    m_pendingContinue = nullptr;

    m_state.reset(new GameState());
    auto* st = m_state.get();
    st->E = root["E"].toInt(100); st->S = root["S"].toInt(80);
    st->K = root["K"].toInt(0);   st->Q = root["Q"].toInt(0);
    st->L = root["L"].toInt(0);   st->M = root["M"].toInt(0);
    st->N = root["N"].toInt(0);
    st->day  = (Day)root["day"].toInt(0);
    st->slot = (TimeSlot)root["slot"].toInt(0);
    st->sceneCheckpoint = root["sceneCheckpoint"].toString("campusHub");
    st->items                  = arrayToSet(root["items"].toArray());
    st->completedTasks         = arrayToSet(root["completedTasks"].toArray());
    st->tasksCompletedThisLoop = arrayToSet(root["tasksCompletedThisLoop"].toArray());
    st->worldCracks            = arrayToSet(root["worldCracks"].toArray());
    st->unlockedEndings        = arrayToSet(root["unlockedEndings"].toArray());
    st->seenEndings            = arrayToSet(root["seenEndings"].toArray());
    st->carryingStudentCard       = root["carryingStudentCard"].toBool();
    st->firstHaitangBoxOpened     = root["firstHaitangBoxOpened"].toBool();
    st->haitangBoxTakenCard       = root["haitangBoxTakenCard"].toBool();
    st->everCompletedHandkerchief = root["everCompletedHandkerchief"].toBool();
    st->everPublishedRecording    = root["everPublishedRecording"].toBool();
    st->everReceivedReply         = root["everReceivedReply"].toBool();
    st->catTeachingSat   = root["catTeachingSat"].toBool();
    st->catLibrarySat    = root["catLibrarySat"].toBool();
    st->catPlaygroundSat = root["catPlaygroundSat"].toBool();
    st->teachingBbDone   = root["teachingBbDone"].toBool();
    st->libraryBbDone    = root["libraryBbDone"].toBool();
    st->libraryTestDone  = root["libraryTestDone"].toBool();
    st->canteenBDone     = root["canteenBDone"].toBool();
    st->canteenCDone     = root["canteenCDone"].toBool();
    st->corridorShadowDone = root["corridorShadowDone"].toBool();
    st->blankPaperTaken    = root["blankPaperTaken"].toBool();
    st->storyAAA_done      = root["storyAAA_done"].toBool();
    st->inDormTonight      = root["inDormTonight"].toBool(true);
    st->nightChoiceMadeThisLoop = root["nightChoiceMadeThisLoop"].toBool();
    st->canteenAuntUnlocked     = root["canteenAuntUnlocked"].toBool();
    st->clampStats();

    m_storyLog.clear();
    for (const auto& v : root["storyLog"].toArray()) m_storyLog.append(v.toString());

    m_lastDialogText   = root["lastDialogText"].toString();
    m_lastWasChoices   = root["lastWasChoices"].toBool();
    m_lastChoiceLabels.clear();
    for (const auto& v : root["lastChoiceLabels"].toArray())
        m_lastChoiceLabels.append(v.toString());

    m_engine.reset(new SceneEngine(static_cast<IGameView*>(this), st, this));
    m_atStartMenu = false;
    refreshHud();

    // 先把"存档时的对话/选项"重现出来; 玩家点继续或选择后进入对应场景
    showResumedDialog();
    return true;
}

// 把存档点的对话作为"快照"重新显示;
// 如果存档时是 choices 模式, 显示一个"返回该场景"按钮 (玩家可重新做选择)
// 如果是 narration, 显示一个"继续"按钮 (点完进入场景 checkpoint)
void MainWindow::showResumedDialog() {
    clearChoices();
    m_pendingContinue = nullptr;
    m_narrationHolders.clear();

    QString resumeBanner = QStringLiteral(
        "<div style='color:#9eddff; font-size:13px; margin-bottom:8px;'>"
        "—— 存档已加载，从此处继续 ——</div>");
    setDialogText(resumeBanner + applyScramble(m_lastDialogText));

    auto goToScene = [this]() {
        m_engine->resumeByCheckpoint();
    };

    if (m_lastWasChoices && !m_lastChoiceLabels.isEmpty()) {
        // 把当时的选项作为"只读快照"展示在下方, 点击任意一个都回到当前场景
        // (无法重建 onClick lambda; 玩家可在场景里重新选)
        for (const QString& lbl : m_lastChoiceLabels) {
            auto* btn = new QPushButton(m_choicesPanel);
            btn->setText(applyScramble(lbl));
            btn->setCursor(Qt::PointingHandCursor);
            btn->setStyleSheet(styleSheetForButton());
            connect(btn, &QPushButton::clicked, this, [this, goToScene]() {
                clearChoices();
                goToScene();
            });
            m_choicesLayout->addWidget(btn);
            m_choiceButtons.append(btn);
        }
    } else {
        auto* btn = new QPushButton(m_choicesPanel);
        btn->setText(QStringLiteral("继续 →"));
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(styleSheetForButton());
        m_pendingContinue = goToScene;
        connect(btn, &QPushButton::clicked, this, [this]() {
            auto cb = m_pendingContinue;
            m_pendingContinue = nullptr;
            clearChoices();
            if (cb) cb();
        });
        m_choicesLayout->addWidget(btn);
        m_choiceButtons.append(btn);
    }
}

void MainWindow::openSavePanel() {
    if (!m_state || m_atStartMenu) {
        QMessageBox::information(this, QStringLiteral("存档"),
            QStringLiteral("当前不在游戏中，无法存档。"));
        return;
    }
    auto* dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(QStringLiteral("存档 - 选择槽位"));
    dlg->resize(560, 460);
    dlg->setStyleSheet(
        "QDialog { background-color: #1a1a22; }"
        "QFrame#SlotRow { background-color: #2a2a36;"
        "  border: 1px solid #4a4a5a; border-radius: 8px; }"
        "QLabel { background: transparent; color: #eee; }");
    auto* lay = new QVBoxLayout(dlg);
    lay->setContentsMargins(18, 18, 18, 18);
    lay->setSpacing(10);

    auto* title = new QLabel(QStringLiteral(
        "<h2 style='color:#ffd97a; margin:0;'>存档</h2>"
        "<p style='color:#aaa; margin-top:6px;'>最多 %1 个槽位。选择一个槽位写入当前进度。</p>")
        .arg(kSaveSlots), dlg);
    title->setTextFormat(Qt::RichText);
    lay->addWidget(title);

    for (int i = 1; i <= kSaveSlots; ++i) {
        QString info;
        QFile f(slotPath(i));
        if (f.exists() && f.open(QIODevice::ReadOnly)) {
            auto root = QJsonDocument::fromJson(f.readAll()).object();
            f.close();
            info = QStringLiteral("%1   %2")
                       .arg(root["saveTime"].toString("?"))
                       .arg(root["display"].toString("?"));
        } else {
            info = QStringLiteral("（空）");
        }
        auto* row = new QFrame(dlg);
        row->setObjectName("SlotRow");
        row->setAttribute(Qt::WA_StyledBackground, true);
        auto* h = new QHBoxLayout(row);
        h->setContentsMargins(12, 8, 12, 8);
        auto* lbl = new QLabel(
            QStringLiteral("<b style='color:#fff;'>槽位 %1</b>"
                           "<br/><span style='color:#bbb; font-size:13px;'>%2</span>")
                .arg(i).arg(info.toHtmlEscaped()), row);
        lbl->setTextFormat(Qt::RichText);
        h->addWidget(lbl, 1);
        auto* btn = new QPushButton(QStringLiteral("写入"), row);
        btn->setCursor(Qt::PointingHandCursor);
        btn->setStyleSheet(styleSheetForButton());
        connect(btn, &QPushButton::clicked, dlg, [this, i, dlg]() {
            if (QFile::exists(slotPath(i))) {
                auto r = QMessageBox::question(dlg, QStringLiteral("覆盖确认"),
                    QStringLiteral("槽位 %1 已有存档，确定覆盖？").arg(i));
                if (r != QMessageBox::Yes) return;
            }
            QString err;
            if (saveToSlot(i, &err)) {
                QMessageBox::information(dlg, QStringLiteral("存档"),
                    QStringLiteral("已写入槽位 %1。").arg(i));
                dlg->accept();
            } else {
                QMessageBox::warning(dlg, QStringLiteral("存档失败"), err);
            }
        });
        h->addWidget(btn, 0);
        lay->addWidget(row);
    }

    lay->addStretch(1);
    auto* close = new QPushButton(QStringLiteral("关闭"), dlg);
    close->setCursor(Qt::PointingHandCursor);
    close->setStyleSheet(styleSheetForButton());
    connect(close, &QPushButton::clicked, dlg, &QDialog::reject);
    lay->addWidget(close, 0);
    dlg->show();
}

void MainWindow::openLoadPanel(bool fromStartMenu) {
    auto* dlg = new QDialog(this);
    dlg->setAttribute(Qt::WA_DeleteOnClose);
    dlg->setWindowTitle(QStringLiteral("读档 - 选择槽位"));
    dlg->resize(560, 460);
    dlg->setStyleSheet(
        "QDialog { background-color: #1a1a22; }"
        "QFrame#SlotRow { background-color: #2a2a36;"
        "  border: 1px solid #4a4a5a; border-radius: 8px; }"
        "QLabel { background: transparent; color: #eee; }");
    auto* lay = new QVBoxLayout(dlg);
    lay->setContentsMargins(18, 18, 18, 18);
    lay->setSpacing(10);

    auto* title = new QLabel(QStringLiteral(
        "<h2 style='color:#ffd97a; margin:0;'>读档</h2>"
        "<p style='color:#aaa; margin-top:6px;'>选择一个槽位读取。"
        "读档后会返回存档时的对话点。</p>"), dlg);
    title->setTextFormat(Qt::RichText);
    lay->addWidget(title);

    bool anySlot = false;
    for (int i = 1; i <= kSaveSlots; ++i) {
        QString info;
        bool exists = QFile::exists(slotPath(i));
        if (exists) {
            anySlot = true;
            QFile f(slotPath(i));
            if (f.open(QIODevice::ReadOnly)) {
                auto root = QJsonDocument::fromJson(f.readAll()).object();
                f.close();
                info = QStringLiteral("%1   %2")
                           .arg(root["saveTime"].toString("?"))
                           .arg(root["display"].toString("?"));
            }
        } else {
            info = QStringLiteral("（空）");
        }

        auto* row = new QFrame(dlg);
        row->setObjectName("SlotRow");
        row->setAttribute(Qt::WA_StyledBackground, true);
        auto* h = new QHBoxLayout(row);
        h->setContentsMargins(12, 8, 12, 8);
        auto* lbl = new QLabel(
            QStringLiteral("<b style='color:#fff;'>槽位 %1</b>"
                           "<br/><span style='color:#bbb; font-size:13px;'>%2</span>")
                .arg(i).arg(info.toHtmlEscaped()), row);
        lbl->setTextFormat(Qt::RichText);
        h->addWidget(lbl, 1);

        auto* btnLoad = new QPushButton(QStringLiteral("读取"), row);
        btnLoad->setCursor(Qt::PointingHandCursor);
        btnLoad->setEnabled(exists);
        btnLoad->setStyleSheet(styleSheetForButton() +
            "QPushButton:disabled { color:#666; }");
        connect(btnLoad, &QPushButton::clicked, dlg, [this, i, dlg]() {
            QString err;
            if (loadFromSlot(i, &err)) {
                dlg->accept();
            } else {
                QMessageBox::warning(dlg, QStringLiteral("读档失败"), err);
            }
        });
        h->addWidget(btnLoad, 0);

        auto* btnDel = new QPushButton(QStringLiteral("删除"), row);
        btnDel->setCursor(Qt::PointingHandCursor);
        btnDel->setEnabled(exists);
        btnDel->setStyleSheet(styleSheetForButton() +
            "QPushButton:disabled { color:#666; }");
        connect(btnDel, &QPushButton::clicked, dlg, [this, i, dlg]() {
            auto r = QMessageBox::question(dlg, QStringLiteral("删除确认"),
                QStringLiteral("确认删除槽位 %1 的存档？").arg(i));
            if (r != QMessageBox::Yes) return;
            QFile::remove(slotPath(i));
            dlg->reject();
            // 重开面板刷新
            QTimer::singleShot(0, this, [this]() { openLoadPanel(m_atStartMenu); });
        });
        h->addWidget(btnDel, 0);
        lay->addWidget(row);
    }

    if (!anySlot && fromStartMenu) {
        auto* hint = new QLabel(QStringLiteral(
            "<p style='color:#888;'>（还没有任何存档。）</p>"), dlg);
        hint->setTextFormat(Qt::RichText);
        lay->addWidget(hint);
    }

    lay->addStretch(1);

    // 重新开始 (新游戏): 弃用当前进度, 从开场重启
    auto* restart = new QPushButton(QStringLiteral("重新开始（新游戏）"), dlg);
    restart->setCursor(Qt::PointingHandCursor);
    restart->setStyleSheet(styleSheetForButton());
    connect(restart, &QPushButton::clicked, dlg, [this, dlg]() {
        auto r = QMessageBox::question(dlg, QStringLiteral("重新开始"),
            QStringLiteral("将放弃当前进度，从开场重新开始。\n"
                           "（已有的存档不会被删除。）\n\n确定吗？"));
        if (r != QMessageBox::Yes) return;
        // 保留 seenEndings 跨重开持久化 (图鉴用)
        QSet<QString> seen = m_state ? m_state->seenEndings : QSet<QString>();
        m_engine.reset();
        m_state.reset(new GameState());
        m_state->seenEndings = seen;
        m_engine.reset(new SceneEngine(static_cast<IGameView*>(this),
                                       m_state.get(), this));
        m_storyLog.clear();
        m_atStartMenu = false;
        refreshHud();
        dlg->accept();
        m_engine->startIntro();
    });
    lay->addWidget(restart, 0);

    auto* close = new QPushButton(
        fromStartMenu ? QStringLiteral("返回菜单") : QStringLiteral("关闭"), dlg);
    close->setCursor(Qt::PointingHandCursor);
    close->setStyleSheet(styleSheetForButton());
    connect(close, &QPushButton::clicked, dlg, &QDialog::reject);
    lay->addWidget(close, 0);
    dlg->show();
}

bool MainWindow::eventFilter(QObject* obj, QEvent* ev) {
    // 点击对话框任意位置 (文字区域 viewport 或 dialogBox 自身空白) -> 等同于点 "继续"
    // 仅在 narration 状态 (m_pendingContinue 有值) 下生效, 选择/结局状态不触发
    if (ev->type() == QEvent::MouseButtonRelease) {
        auto* me = static_cast<QMouseEvent*>(ev);
        if (me->button() == Qt::LeftButton && m_pendingContinue) {
            // 仅当点击的是 dialogBox/viewport 自身, 而不是按钮等其它子控件
            bool eligible = (obj == m_dialogBox)
                || (m_dialogText && obj == m_dialogText->viewport());
            if (eligible) {
                auto cb = m_pendingContinue;
                m_pendingContinue = nullptr;
                clearChoices();
                if (cb) cb();
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(obj, ev);
}
