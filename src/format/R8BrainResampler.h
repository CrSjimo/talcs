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

#ifndef TALCS_R8BRAINRESAMPLER_H
#define TALCS_R8BRAINRESAMPLER_H

#include <functional>

#include <QScopedPointer>

#include <TalcsFormat/TalcsFormatGlobal.h>

namespace r8b {
    class CDSPResampler;
}

namespace talcs {

    class R8BrainResamplerPrivate;

    class TALCSFORMAT_EXPORT R8BrainResampler {
    public:
        explicit R8BrainResampler(double ratio, qint64 bufferSize);
        ~R8BrainResampler();

        void reset();

        void process(float *buffer);

        double ratio() const;
        qint64 bufferSize() const;

    protected:
        virtual void read(float *inputBlock, qint64 length) = 0;

    private:
        QScopedPointer<R8BrainResamplerPrivate> d;

    };

} // talcs

#endif //TALCS_R8BRAINRESAMPLER_H
