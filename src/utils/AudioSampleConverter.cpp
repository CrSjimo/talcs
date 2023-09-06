#include "AudioSampleConverter.h"

#include <QSysInfo>
#include <QtEndian>

namespace talcs {
    static const double factor16 = (double) 0x7fffL + 0.49999;
    static const double factor24 = (double) 0x7fffffL + 0.49999;
    static const double factor32 = (double) 0x7fffffffL + 0.49999;

    /**
     * @class AudioSampleConverter
     * @brief The class provides functions to convert between several sample types.
     */

    /**
     * Converts @c float type samples to 16-bit int.
     * @param dest the pointer to a pre-allocated destination memory
     * @param src the pointer to source samples
     * @param length the number of samples
     * @param isLittleEndian whether the destination values are little-endian
     */
    void AudioSampleConverter::convertFloatToInt16(void *dest, const float *src, qint64 length, bool isLittleEndian) {
        auto p = reinterpret_cast<qint16 *>(dest);
        while (--length >= 0)
            *p++ = isLittleEndian ? (qint16) qToLittleEndian((double) (*src++) * factor16)
                                  : (qint16) qToBigEndian((double) (*src++) * factor16);
    }

    /**
     * Converts @c float type samples to 24-bit int.
     * @param dest the pointer to a pre-allocated destination memory
     * @param src the pointer to source samples
     * @param length the number of samples
     * @param isLittleEndian whether the destination values are little-endian
     */
    void AudioSampleConverter::convertFloatToInt24(void *dest, const float *src, qint64 length, bool isLittleEndian) {
        long a;
        char *b = (char *) dest;
        char *aa = (char *) &a;
        while (--length >= 0) {
            if (isLittleEndian) {
                a = (qint32) qToLittleEndian((double) (*src++) * factor24);
                *b++ = aa[0];
                *b++ = aa[1];
                *b++ = aa[2];
            } else {
                a = (qint32) qToBigEndian((double) (*src++) * factor24);
                *b++ = aa[3];
                *b++ = aa[2];
                *b++ = aa[1];
            }
        }
    }

    /**
     * Converts @c float type samples to 32-bit int.
     * @param dest the pointer to a pre-allocated destination memory
     * @param src the pointer to source samples
     * @param length the number of samples
     * @param isLittleEndian whether the destination values are little-endian
     */
    void AudioSampleConverter::convertFloatToInt32(void *dest, const float *src, qint64 length, bool isLittleEndian) {
        auto p = reinterpret_cast<qint32 *>(dest);
        while (--length >= 0)
            *p++ = isLittleEndian ? (qint32) qToLittleEndian((double) (*src++) * factor32)
                                  : (qint32) qToBigEndian((double) (*src++) * factor32);
    }

    /**
     * Converts @c float type samples to 32-bit float.
     * @param dest the pointer to a pre-allocated destination memory
     * @param src the pointer to source samples
     * @param length the number of samples
     * @param isLittleEndian whether the destination values are little-endian
     */
    void AudioSampleConverter::convertFloatToFloat32(void *dest, const float *src, qint64 length, bool isLittleEndian) {
        if (isLittleEndian) {
            qToLittleEndian<float>(src, length, dest);
        } else {
            qToBigEndian<float>(src, length, dest);
        }
    }

    /**
     * Converts @c float type samples to 64-bit float.
     * @param dest the pointer to a pre-allocated destination memory
     * @param src the pointer to source samples
     * @param length the number of samples
     * @param isLittleEndian whether the destination values are little-endian
     */
    void AudioSampleConverter::convertFloatToFloat64(void *dest, const float *src, qint64 length, bool isLittleEndian) {
        auto p = reinterpret_cast<double *>(dest);
        while (--length >= 0)
            *p++ = isLittleEndian ? qToLittleEndian(*src++) : qToBigEndian(*src++);
    }
}