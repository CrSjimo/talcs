#ifndef TALCS_AUDIODRIVERMANAGER_H
#define TALCS_AUDIODRIVERMANAGER_H

#include "AudioDriver.h"

namespace talcs {
    class AudioDriverManagerPrivate;

    class TALCS_EXPORT AudioDriverManager : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(AudioDriverManager)
    public:
        AudioDriverManager(QObject *parent = nullptr);
        ~AudioDriverManager();

        bool addAudioDriver(AudioDriver *driver);
        bool removeDriver(AudioDriver *driver);

        AudioDriver *driver(const QString &name) const;
        QStringList drivers() const;

        static AudioDriverManager *createBuiltInDriverManager(QObject *parent = nullptr);

    signals:
        void driverAdded(AudioDriver *driver);
        void driverRemoved(AudioDriver *driver);

    protected:
        AudioDriverManager(AudioDriverManagerPrivate &d, QObject *parent);
        QScopedPointer<AudioDriverManagerPrivate> d_ptr;
    };
}

#endif // TALCS_AUDIODRIVERMANAGER_H
