#ifndef TALCS_AUDIODRIVERMANAGER_P_H
#define TALCS_AUDIODRIVERMANAGER_P_H

#include "AudioDriverManager.h"

#include <QMChronMap.h>

namespace talcs {
    class AudioDriverManagerPrivate {
        Q_DECLARE_PUBLIC(AudioDriverManager)
    public:
        AudioDriverManager *q_ptr;
        QMChronMap<QString, AudioDriver *> driverDict;
    };
}

#endif // TALCS_AUDIODRIVERMANAGER_P_H
