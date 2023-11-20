#ifndef TALCS_AUDIOSOURCE_H
#define TALCS_AUDIOSOURCE_H

#include <QScopedPointer>

#include <TalcsCore/IAudioSampleContainer.h>
#include <TalcsCore/AudioStreamBase.h>
#include <TalcsCore/DuplicatableObject.h>

namespace talcs {

    struct TALCSCORE_EXPORT AudioSourceReadData {
        AudioSourceReadData(IAudioSampleContainer *buffer);
        AudioSourceReadData(IAudioSampleContainer *buffer, qint64 startPos, qint64 length, int silentFlags = 0);
        IAudioSampleContainer *buffer;
        qint64 startPos;
        qint64 length;
        int silentFlags;
    };

    class AudioSourcePrivate;

    class TALCSCORE_EXPORT AudioSource : public AudioStreamBase, public DuplicatableObject {
        Q_DECLARE_PRIVATE(AudioSource)
    public:
        AudioSource();
        virtual ~AudioSource();
        virtual qint64 read(const AudioSourceReadData &readData) = 0;

    protected:
        explicit AudioSource(AudioSourcePrivate &d);
        QScopedPointer<AudioSourcePrivate> d_ptr;
    };
    
}

#endif // TALCS_AUDIOSOURCE_H
