#include "AudioStreamBase.h"

namespace talcs {
    bool AudioStreamBase::open(qint64 bufferSize, double sampleRate) {
        m_bufferSize = bufferSize;
        m_sampleRate = sampleRate;
        m_isOpened = true;
        return true;
    }
    bool AudioStreamBase::isOpen() const {
        return m_isOpened;
    }
    void AudioStreamBase::close() {
        m_bufferSize = 0;
        m_sampleRate = 0;
        m_isOpened = false;
    }
    qint64 AudioStreamBase::bufferSize() const {
        return m_bufferSize;
    }
    double AudioStreamBase::sampleRate() const {
        return m_sampleRate;
    }
}
