#ifndef TALCS_AUDIOSAMPLECONVERTER_H
#define TALCS_AUDIOSAMPLECONVERTER_H

#include <QtGlobal>

#include "global/TalcsGlobal.h"

namespace talcs {
    class TALCS_EXPORT AudioSampleConverter {
    public:
        static void convertFloatToInt16(void *dest, const float *src, qint64 length, bool isLittleEndian);
        static void convertFloatToInt24(void *dest, const float *src, qint64 length, bool isLittleEndian);
        static void convertFloatToInt32(void *dest, const float *src, qint64 length, bool isLittleEndian);
        static void convertFloatToFloat32(void *dest, const float *src, qint64 length, bool isLittleEndian);
        static void convertFloatToFloat64(void *dest, const float *src, qint64 length, bool isLittleEndian);
    };
}



#endif // TALCS_AUDIOSAMPLECONVERTER_H
