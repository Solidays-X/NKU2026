#ifndef STORYDISPLAY_H
#define STORYDISPLAY_H

#include <QWidget>
#include <QTextEdit>
#include <QStringList>

class StoryDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit StoryDisplay(QWidget *parent = nullptr);

    // 设置要显示的全部文本，自动按空行分割为段落
    void setFullText(const QString &text);
    // 直接设置段落列表
    void setParagraphs(const QStringList &paragraphs);
    // 清空并重新开始
    void reset();

    bool isFinished() const;   // 是否所有段落已显示完毕
    QString toPlainText() const;

signals:
    void allParagraphsShown();  // 所有段落显示完毕（包括仅有一段的情况）
    void continueRequested();   // 玩家按下回车请求下一段（可用于统计）

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void paintEvent(QPaintEvent *event) override;  // 绘制提示符

private:
    void showNextParagraph();   // 显示下一段
    void appendText(const QString &text);
    void updatePrompt();

    QTextEdit *m_textEdit;
    QStringList m_paragraphs;
    int m_currentIndex = -1;    // 当前已显示的最后一段索引
    bool m_finished = true;     // 是否全部显示完毕
    QString m_remainingText;    // 暂存未完全按回车显示的文本（兼容）
    bool m_promptVisible = false;

    // 样式
    static const QString kPromptText; // "▼ 按回车继续 ▼"
};

#endif // STORYDISPLAY_H