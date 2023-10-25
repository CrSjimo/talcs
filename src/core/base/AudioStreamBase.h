#ifndef TALCS_AUDIOSTREAMBASE_H
#define TALCS_AUDIOSTREAMBASE_H

#include <TalcsCore/TalcsCoreGlobal.h>

namespace talcs {

    class TALCSCORE_EXPORT AudioStreamBase {
    public:
        virtual bool open(qint64 bufferSize, double sampleRate);
        bool isOpen() const;
        virtual void close();

        qint64 bufferSize() const;
        double sampleRate() const;

    private:
        qint64 m_bufferSize = 0;
        double m_sampleRate = 0;
        bool m_isOpened = false;
    };

}

#endif // TALCS_AUDIOSTREAMBASE_H
