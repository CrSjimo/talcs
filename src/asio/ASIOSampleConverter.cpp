/*******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                  *
 * SPDX-License-Identifier: GPL-3.0-only                                       *
 *                                                                             *
 * This file is part of TALCS.                                                 *
 *                                                                             *
 * TALCS is free software: you can redistribute it and/or modify it under the  *
 * terms of the GNU General Public License as published by the Free Software   *
 * Foundation, version 3 of the License.                                       *
 *                                                                             *
 * TALCS is distributed in the hope that it will be useful, but WITHOUT ANY    *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS   *
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more       *
 * details.                                                                    *
 *                                                                             *
 * You should have received a copy of the GNU General Public License along     *
 * with TALCS. If not, see <https://www.gnu.org/licenses/>.                    *
 ******************************************************************************/

#include "ASIOSampleConverter_p.h"

#include <algorithm>
#include <cmath>

#include <QtEndian>

namespace talcs {

    struct ASIOSampleFormat {
        int bytes = 0;
        int validBits = 0;
        bool floatingPoint = false;
        bool littleEndian = false;
    };

    static ASIOSampleFormat sampleFormat(ASIOSampleType type) {
        switch (type) {
            case ASIOSTInt16LSB:
                return {2, 16, false, true};
            case ASIOSTInt16MSB:
                return {2, 16, false, false};
            case ASIOSTInt24LSB:
                return {3, 24, false, true};
            case ASIOSTInt24MSB:
                return {3, 24, false, false};
            case ASIOSTInt32LSB:
                return {4, 32, false, true};
            case ASIOSTInt32MSB:
                return {4, 32, false, false};
            case ASIOSTInt32LSB16:
                return {4, 16, false, true};
            case ASIOSTInt32MSB16:
                return {4, 16, false, false};
            case ASIOSTInt32LSB18:
                return {4, 18, false, true};
            case ASIOSTInt32MSB18:
                return {4, 18, false, false};
            case ASIOSTInt32LSB20:
                return {4, 20, false, true};
            case ASIOSTInt32MSB20:
                return {4, 20, false, false};
            case ASIOSTInt32LSB24:
                return {4, 24, false, true};
            case ASIOSTInt32MSB24:
                return {4, 24, false, false};
            case ASIOSTFloat32LSB:
                return {4, 32, true, true};
            case ASIOSTFloat32MSB:
                return {4, 32, true, false};
            case ASIOSTFloat64LSB:
                return {8, 64, true, true};
            case ASIOSTFloat64MSB:
                return {8, 64, true, false};
            default:
                return {};
        }
    }

    int ASIOSampleConverter::sampleSize(ASIOSampleType type) {
        return sampleFormat(type).bytes;
    }

    bool ASIOSampleConverter::convert(void *dest, const float *src, qint64 length, ASIOSampleType type) {
        const auto format = sampleFormat(type);
        if (!format.bytes) {
            return false;
        }
        auto output = static_cast<unsigned char *>(dest);
        if (format.floatingPoint) {
            if (format.bytes == 4) {
                if (format.littleEndian) {
                    qToLittleEndian<float>(src, length, dest);
                } else {
                    qToBigEndian<float>(src, length, dest);
                }
            } else {
                for (qint64 i = 0; i < length; i++) {
                    const auto value = static_cast<double>(src[i]);
                    if (format.littleEndian) {
                        qToLittleEndian<double>(value, output + i * 8);
                    } else {
                        qToBigEndian<double>(value, output + i * 8);
                    }
                }
            }
            return true;
        }

        const qint64 scale = qint64(1) << (format.validBits - 1);
        for (qint64 i = 0; i < length; i++) {
            const double sample = std::isfinite(src[i]) ? std::clamp(static_cast<double>(src[i]), -1.0, 1.0) : 0.0;
            const auto value = std::clamp<qint64>(std::llround(sample * scale), -scale, scale - 1);
            // Signed conversion extends the sign through the complete 32-bit container.
            const auto bits = static_cast<quint32>(static_cast<qint32>(value));
            for (int byte = 0; byte < format.bytes; byte++) {
                const int shift = 8 * (format.littleEndian ? byte : format.bytes - byte - 1);
                *output++ = static_cast<unsigned char>(bits >> shift);
            }
        }
        return true;
    }

}
