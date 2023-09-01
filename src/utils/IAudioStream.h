//
// Created by Crs_1 on 2023/7/10.
//

#ifndef IAUDIOSTREAM_H
#define IAUDIOSTREAM_H

#include <QtGlobal>

#include "global/TalcsGlobal.h"

class TALCS_EXPORT IAudioStream {
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



#endif // IAUDIOSTREAM_H
