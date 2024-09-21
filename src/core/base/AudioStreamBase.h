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

    protected:
        ~AudioStreamBase() = default;

    private:
        qint64 m_bufferSize = 0;
        double m_sampleRate = 0;
        bool m_isOpened = false;
    };

}

#endif // TALCS_AUDIOSTREAMBASE_H
