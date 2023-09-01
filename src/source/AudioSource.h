//
// Created by Crs_1 on 2023/7/8.
//

#ifndef AUDIOSOURCE_H
#define AUDIOSOURCE_H

#include <QScopedPointer>

#include "buffer/IAudioSampleContainer.h"
#include "utils/IAudioStream.h"

class TALCS_EXPORT AudioSourceReadData {
public:
    AudioSourceReadData(IAudioSampleContainer *buffer); // implicit use IAudioSampleContainer
    AudioSourceReadData(IAudioSampleContainer *buffer, qint64 startPos, qint64 length);
    IAudioSampleContainer *buffer;
    qint64 startPos;
    qint64 length;
};

class AudioSourcePrivate;

/* abstract */
class TALCS_EXPORT AudioSource : public IAudioStream {
    Q_DECLARE_PRIVATE(AudioSource)
public:
    AudioSource();
    virtual ~AudioSource();
    virtual qint64 read(const AudioSourceReadData &readData) = 0;

protected:
    explicit AudioSource(AudioSourcePrivate &d);
    QScopedPointer<AudioSourcePrivate> d_ptr;
};



#endif // AUDIOSOURCE_H
