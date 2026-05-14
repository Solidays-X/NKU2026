#include "audio.h"

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#endif

#include <QFile>
#include <QFileInfo>
#include <QDir>

namespace Audio {

static QString g_alias = QStringLiteral("nankai_bgm");
static QString g_tempPath; // 解压到磁盘的临时副本
static bool g_playing = false;

static bool sendCmd(const QString& cmd) {
#ifdef _WIN32
    return mciSendStringW(reinterpret_cast<LPCWSTR>(cmd.utf16()), nullptr, 0, nullptr) == 0;
#else
    Q_UNUSED(cmd);
    return false;
#endif
}

bool playBgm(const QString& path, bool loop) {
#ifdef _WIN32
    stopBgm();

    QString realPath = path;
    // 如果路径来自 Qt 资源系统(:/), 复制到临时目录
    if (path.startsWith(":/") || path.startsWith("qrc:")) {
        QFile in(path);
        if (!in.open(QIODevice::ReadOnly)) return false;
        QString tmp = QDir::tempPath() + "/nankai_bgm_" +
                      QFileInfo(path).fileName();
        QFile out(tmp);
        if (!out.open(QIODevice::WriteOnly)) return false;
        out.write(in.readAll());
        out.close();
        in.close();
        g_tempPath = tmp;
        realPath = tmp;
    }

    realPath.replace('/', '\\');

    QString openCmd = QStringLiteral("open \"%1\" type mpegvideo alias %2")
                          .arg(realPath, g_alias);
    if (!sendCmd(openCmd)) {
        // 尝试不指定 type, 让 MCI 自行推断
        openCmd = QStringLiteral("open \"%1\" alias %2").arg(realPath, g_alias);
        if (!sendCmd(openCmd)) return false;
    }

    QString playCmd = loop
                          ? QStringLiteral("play %1 repeat").arg(g_alias)
                          : QStringLiteral("play %1").arg(g_alias);
    g_playing = sendCmd(playCmd);
    return g_playing;
#else
    Q_UNUSED(path); Q_UNUSED(loop);
    return false;
#endif
}

void stopBgm() {
#ifdef _WIN32
    if (g_playing) {
        sendCmd(QStringLiteral("stop %1").arg(g_alias));
        sendCmd(QStringLiteral("close %1").arg(g_alias));
        g_playing = false;
    }
#endif
}

void setVolume(int percent) {
#ifdef _WIN32
    if (!g_playing) return;
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    int v = percent * 10; // MCI volume 0..1000
    sendCmd(QStringLiteral("setaudio %1 volume to %2").arg(g_alias).arg(v));
#endif
}

}
