#include "headless.h"
#include "gamestate.h"

#include <QTextStream>
#include <QRegularExpression>
#include <QFile>
#include <QDebug>

static QFile* s_logFile = nullptr;
void HeadlessView_setLogFile(QFile* f) { s_logFile = f; }
static void hlog(const QString& s) {
    if (s_logFile) {
        s_logFile->write(s.toLocal8Bit());
        s_logFile->write("\n");
        s_logFile->flush();
    }
}

HeadlessView::HeadlessView(QObject* parent) : QObject(parent) {}

QString HeadlessView::stripHtml(QString s) const {
    s.replace(QStringLiteral("<br/>"), QStringLiteral("\n"));
    s.replace(QStringLiteral("<br>"),  QStringLiteral("\n"));
    static const QRegularExpression tags("<[^>]+>");
    s.remove(tags);
    return s;
}

void HeadlessView::showNarration(const QString& text, std::function<void()> cont) {
    m_lastText = text;
    m_lastChoices.clear();
    m_lastCont = std::move(cont);
    m_pending = Pending::Narration;
    if (m_verbose) {
        hlog(QStringLiteral("[NARR] ") + stripHtml(text).left(160));
    }
}

void HeadlessView::showChoices(const QString& text, const QVector<Choice>& choices) {
    m_lastText = text;
    m_lastChoices = choices;
    m_lastCont = nullptr;
    m_pending = Pending::Choices;
    if (m_verbose) {
        hlog(QStringLiteral("[ASK ] ") + stripHtml(text).left(120));
        for (int i = 0; i < choices.size(); ++i) {
            hlog(QStringLiteral("       (%1) %2").arg(i).arg(choices[i].text));
        }
    }
}

void HeadlessView::showEnding(const QString& title, const QString& body) {
    m_endingTitle = title;
    m_endingBody = body;
    m_pending = Pending::Ended;
    hlog(QStringLiteral("\n========== ENDING =========="));
    hlog(title);
    hlog(body.left(800) + QStringLiteral(" ..."));
    hlog(QStringLiteral("============================"));
    emit endingReached(title);
}

void HeadlessView::refreshHud() {
    if (m_verbose && m_state) {
        hlog(QStringLiteral("[HUD ] %1 L=%2 E=%3 S=%4 K=%5 Q=%6 N=%7 M=%8")
             .arg(m_state->timeDisplay())
             .arg(m_state->L).arg(m_state->E).arg(m_state->S)
             .arg(m_state->K).arg(m_state->Q).arg(m_state->N).arg(m_state->M));
    }
}

bool HeadlessView::step() {
    if (++m_steps > m_maxSteps) {
        QTextStream(stderr) << "[ERR ] max steps exceeded\n";
        m_pending = Pending::Ended;
        m_endingTitle = QStringLiteral("MAX_STEPS");
        return false;
    }
    switch (m_pending) {
        case Pending::None:
            return false;
        case Pending::Ended:
            return false;
        case Pending::Narration: {
            auto cont = m_lastCont;
            m_lastCont = nullptr;
            m_pending = Pending::None;
            if (cont) cont();
            return true;
        }
        case Pending::Choices: {
            int idx = -1;
            if (m_picker) idx = m_picker(m_lastText, m_lastChoices);
            bool usedDefault = false;
            if (idx < 0 || idx >= m_lastChoices.size()) {
                usedDefault = true;
                idx = 0;
                for (int i = 0; i < m_lastChoices.size(); ++i) {
                    const QString& t = m_lastChoices[i].text;
                    if (!t.startsWith(QStringLiteral("←"))
                        && !t.contains(QStringLiteral("离开"))
                        && !t.contains(QStringLiteral("返回"))) {
                        idx = i; break;
                    }
                }
            }
            if (usedDefault) {
                hlog(QStringLiteral("[FALLBACK] ") +
                     stripHtml(m_lastText).left(80));
            }
            auto fn = m_lastChoices[idx].onClick;
            if (m_verbose) {
                QTextStream out(stdout);
                out << "       -> chose [" << idx << "] "
                    << m_lastChoices[idx].text << "\n";
                out.flush();
            }
            m_lastChoices.clear();
            m_pending = Pending::None;
            if (fn) fn();
            return true;
        }
    }
    return false;
}
