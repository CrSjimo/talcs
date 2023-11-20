#ifndef TALCS_MEMORYAUDIOSOURCE_H
#define TALCS_MEMORYAUDIOSOURCE_H

#include <QScopedPointer>

#include <TalcsCore/PositionableAudioSource.h>

namespace talcs {

    class MemoryAudioSourcePrivate;

    class TALCSCORE_EXPORT MemoryAudioSource : public PositionableAudioSource {
        Q_DECLARE_PRIVATE(MemoryAudioSource)
    public:
        explicit MemoryAudioSource(IAudioSampleProvider *buffer = nullptr, bool takeOwnership = false);
        ~MemoryAudioSource() override;

        IAudioSampleProvider *buffer() const;
        IAudioSampleProvider *setBuffer(IAudioSampleProvider *newBuffer, bool takeOwnership = false);

        qint64 read(const AudioSourceReadData &readData) override;
        qint64 length() const override;

        void setNextReadPosition(qint64 pos) override;

        bool isDuplicatable() const override;

        DuplicatableObject *duplicate() const override;

    protected:
        explicit MemoryAudioSource(MemoryAudioSourcePrivate &d);
    };

}

#endif // TALCS_MEMORYAUDIOSOURCE_H
