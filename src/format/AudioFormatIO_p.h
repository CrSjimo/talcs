#ifndef TALCS_AUDIOFORMATIO_P_H
#define TALCS_AUDIOFORMATIO_P_H

#include "AudioFormatIO.h"

namespace talcs {
    class AudioFormatIOPrivate {
        Q_DECLARE_PUBLIC(AudioFormatIO)
    public:
        AudioFormatIO *q_ptr;
        QIODevice *stream = nullptr;
        QScopedPointer<SndfileHandle> sf;
        QIODevice::OpenMode openMode = QIODevice::NotOpen;

        int64_t sfVioGetFilelen() const;
        int64_t sfVioSeek(int64_t offset, int whence) const;
        int64_t sfVioRead(void *ptr, int64_t count) const;
        int64_t sfVioWrite(const void *ptr, int64_t count) const;
        int64_t sfVioTell() const;
    };
}

#endif // TALCS_AUDIOFORMATIO_P_H
