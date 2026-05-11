#include "storydisplay.h"
#include <QKeyEvent>
#include <QPainter>
#include <QVBoxLayout>
#include <QTextCursor>
#include <QScrollBar>
#include <QApplication>

const QString StoryDisplay::kPromptText = QStringLiteral("▼ 按回车继续 ▼");

StoryDisplay::StoryDisplay(QWidget *parent)
    : QWidget(parent)
{
    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setFrameShape(QFrame::NoFrame);
    m_textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_textEdit->setTextInteractionFlags(Qt::NoTextInteraction);
    m_textEdit->setStyleSheet(
        "QTextEdit { background: transparent; color: white; font-size: 14pt; "
        "border: none; padding: 20px; }"
        );

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_textEdit);

    setStyleSheet("background-color: black;");
    setFocusPolicy(Qt::StrongFocus);

    reset();
}

void StoryDisplay::setFullText(const QString &text)
{
    reset();

    if (text.isEmpty()) {
        m_finished = true;
        emit allParagraphsShown();
        return;
    }

    // 修改点：按单个换行符 '\n' 分割，并过滤掉空行
    QStringList lines = text.split('\n');
    m_paragraphs.clear();
    for (const QString &line : lines) {
        QString trimmed = line.trimmed();
        if (!trimmed.isEmpty()) {
            // 保留原始内容（不去除前导空格等，以保留原文格式）
            m_paragraphs.append(line);
        }
    }

    if (m_paragraphs.isEmpty()) {
        m_finished = true;
        emit allParagraphsShown();
        return;
    }

    m_finished = false;
    m_currentIndex = -1;
    showNextParagraph();
}

void StoryDisplay::setParagraphs(const QStringList &paragraphs)
{
    reset();
    if (paragraphs.isEmpty()) {
        m_finished = true;
        emit allParagraphsShown();
        return;
    }
    m_paragraphs = paragraphs;
    m_finished = false;
    m_currentIndex = -1;
    showNextParagraph();
}

void StoryDisplay::reset()
{
    m_textEdit->clear();
    m_paragraphs.clear();
    m_currentIndex = -1;
    m_finished = true;
    m_remainingText.clear();
    m_promptVisible = false;
    update();
}

QString StoryDisplay::toPlainText() const {
    return m_textEdit->toPlainText();
}

bool StoryDisplay::isFinished() const
{
    return m_finished;
}

void StoryDisplay::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        event->accept();
        if (m_finished) {
            return;
        }
        showNextParagraph();
        emit continueRequested();
    } else {
        QWidget::keyPressEvent(event);
    }
}

void StoryDisplay::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.fillRect(rect(), Qt::black);
    if (m_promptVisible && !m_finished) {
        painter.setPen(QColor(180, 180, 180));
        painter.setFont(QFont("Sans", 10));
        QRect promptRect = rect().adjusted(0, 0, -30, -30);
        painter.drawText(promptRect, Qt::AlignBottom | Qt::AlignRight, kPromptText);
    }
    QWidget::paintEvent(event);
}

void StoryDisplay::showNextParagraph()
{
    if (m_currentIndex + 1 < m_paragraphs.size()) {
        m_currentIndex++;
        appendText(m_paragraphs[m_currentIndex]);
        updatePrompt();
    } else {
        m_finished = true;
        m_promptVisible = false;
        update();
        emit allParagraphsShown();
    }
}

void StoryDisplay::appendText(const QString &text)
{
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    if (!m_textEdit->toPlainText().isEmpty()) {
        // 段落之间添加一个空行作为视觉分隔
        cursor.insertBlock();
    }
    cursor.insertText(text.trimmed());

    QScrollBar *vbar = m_textEdit->verticalScrollBar();
    if (vbar) {
        vbar->setValue(vbar->maximum());
    }
}

void StoryDisplay::updatePrompt()
{
    m_promptVisible = (m_currentIndex + 1 < m_paragraphs.size());
    update();
}