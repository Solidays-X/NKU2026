#pragma once

#include <QString>
#include <QVector>
#include <functional>

struct Choice {
    QString text;
    std::function<void()> onClick;
};

class IGameView {
public:
    virtual ~IGameView() = default;

    virtual void showNarration(const QString& text, std::function<void()> cont) = 0;
    virtual void showChoices(const QString& text,
                             const QVector<Choice>& choices) = 0;
    virtual void showEnding(const QString& title, const QString& body) = 0;

    // 状态栏刷新通知
    virtual void refreshHud() = 0;
};
