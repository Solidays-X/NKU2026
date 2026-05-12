#pragma once

#include <QString>

namespace Audio {

bool playBgm(const QString& path, bool loop = true);
void stopBgm();
void setVolume(int percent); // 0..100

}
