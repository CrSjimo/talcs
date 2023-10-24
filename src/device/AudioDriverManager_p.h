#ifndef AUDIODRIVERMANAGER_P_H
#define AUDIODRIVERMANAGER_P_H

#include <TalcsDevice/AudioDriverManager.h>

#include <QList>

namespace talcs {
    class AudioDriverManagerPrivate {
        Q_DECLARE_PUBLIC(AudioDriverManager)
    public:
        AudioDriverManager *q_ptr;
        QList<QPair<QString, AudioDriver *>> driverDict;
    };
}

#endif // AUDIODRIVERMANAGER_P_H
