#include "IAudioStream.h"

namespace talcs {
    bool IAudioStream::open(qint64 bufferSize, double sampleRate) {
        m_bufferSize = bufferSize;
        m_sampleRate = sampleRate;
        m_isOpened = true;
        return true;
    }
    bool IAudioStream::isOpen() const {
        return m_isOpened;
    }
    void IAudioStream::close() {
        m_bufferSize = 0;
        m_sampleRate = 0;
        m_isOpened = false;
    }
    qint64 IAudioStream::bufferSize() const {
        return m_bufferSize;
    }
    double IAudioStream::sampleRate() const {
        return m_sampleRate;
    }
}
