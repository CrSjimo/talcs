#ifndef AUDIODRIVERMANAGER_P_H
#define AUDIODRIVERMANAGER_P_H

#include <TalcsDevice/AudioDriverManager.h>

#include <QMChronMap.h>

namespace talcs {
    class AudioDriverManagerPrivate {
        Q_DECLARE_PUBLIC(AudioDriverManager)
    public:
        AudioDriverManager *q_ptr;
        QMChronMap<QString, AudioDriver *> driverDict;
    };
}

#endif // AUDIODRIVERMANAGER_P_H
