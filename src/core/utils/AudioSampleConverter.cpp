/******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                 *
 *                                                                            *
 * This file is part of TALCS.                                                *
 *                                                                            *
 * TALCS is free software: you can redistribute it and/or modify it under the *
 * terms of the GNU Lesser General Public License as published by the Free    *
 * Software Foundation, either version 3 of the License, or (at your option)  *
 * any later version.                                                         *
 *                                                                            *
 * TALCS is distributed in the hope that it will be useful, but WITHOUT ANY   *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS  *
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for    *
 * more details.                                                              *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with TALCS. If not, see <https://www.gnu.org/licenses/>.             *
 ******************************************************************************/

#include "AudioSampleConverter.h"

#include <limits>

#include <QSysInfo>
#include <QtEndian>

namespace talcs {

    static const double factor16 = (double) 0x7fffL + 0.49999;
    static const double factor24 = (double) 0x7fffffL + 0.49999;
    static const double factor32 = (double) 0x7fffffffL + 0.49999;

    static inline float rangeRestrict(float x) {
        return qMax(-1.0f, qMin(1.0f, x));
    }

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
    void AudioSampleConverter::convertToInt16(void *dest, const float *src, qint64 length, bool isLittleEndian, bool restrictRange) {
        auto p = reinterpret_cast<qint16 *>(dest);
        if (restrictRange)
            while (--length >= 0)
                *p++ = isLittleEndian ? qToLittleEndian(qint16(rangeRestrict(*src++) * factor16))
                                      : qToBigEndian(qint16(rangeRestrict(*src++) * factor16));
        else
            while (--length >= 0)
                *p++ = isLittleEndian ? qToLittleEndian(qint16((*src++) * factor16))
                                      : qToBigEndian(qint16((*src++) * factor16));
    }

    /**
     * Converts @c float type samples to 24-bit int.
     * @param dest the pointer to a pre-allocated destination memory
     * @param src the pointer to source samples
     * @param length the number of samples
     * @param isLittleEndian whether the destination values are little-endian
     */
    void AudioSampleConverter::convertToInt24(void *dest, const float *src, qint64 length, bool isLittleEndian, bool restrictRange) {
        long a;
        char *b = (char *) dest;
        char *aa = (char *) &a;
        if (restrictRange)
            while (--length >= 0) {
                if (isLittleEndian) {
                    a = qToLittleEndian(qint32(rangeRestrict(*src++) * factor24));
                    *b++ = aa[0];
                    *b++ = aa[1];
                    *b++ = aa[2];
                } else {
                    a = qToBigEndian(qint32(rangeRestrict(*src++) * factor24));
                    *b++ = aa[3];
                    *b++ = aa[2];
                    *b++ = aa[1];
                }
            }
        else
            while (--length >= 0) {
                if (isLittleEndian) {
                    a = qToLittleEndian(qint32((*src++) * factor24));
                    *b++ = aa[0];
                    *b++ = aa[1];
                    *b++ = aa[2];
                } else {
                    a = qToBigEndian(qint32((*src++) * factor24));
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
    void AudioSampleConverter::convertToInt32(void *dest, const float *src, qint64 length, bool isLittleEndian, bool restrictRange) {
        auto p = reinterpret_cast<qint32 *>(dest);
        if (restrictRange)
            while (--length >= 0)
                *p++ = isLittleEndian ? qToLittleEndian(qint32(rangeRestrict(*src++) * factor32))
                                      : qToBigEndian(qint32(rangeRestrict(*src++) * factor32));
        else
            while (--length >= 0)
                *p++ = isLittleEndian ? qToLittleEndian(qint32((*src++) * factor32))
                                      : qToBigEndian(qint32((*src++) * factor32));
    }

    /**
     * Converts @c float type samples to 32-bit float.
     * @param dest the pointer to a pre-allocated destination memory
     * @param src the pointer to source samples
     * @param length the number of samples
     * @param isLittleEndian whether the destination values are little-endian
     */
    void AudioSampleConverter::convertToFloat32(void *dest, const float *src, qint64 length, bool isLittleEndian) {
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
    void AudioSampleConverter::convertToFloat64(void *dest, const float *src, qint64 length, bool isLittleEndian) {
        auto p = reinterpret_cast<double *>(dest);
        while (--length >= 0)
            *p++ = isLittleEndian ? qToLittleEndian(*src++) : qToBigEndian(*src++);
    }
    
}