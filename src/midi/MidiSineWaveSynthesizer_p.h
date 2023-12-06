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

#ifndef TALCS_MIDISINEWAVESYNTHESIZER_P_H
#define TALCS_MIDISINEWAVESYNTHESIZER_P_H

#include <TalcsMidi/MidiSineWaveSynthesizer.h>
#include <QMutex>

namespace talcs {
    class MidiSineWaveSynthesizerPrivate {
    public:
        QMutex mutex;
        double phase = 0;

        int note = -1;
        int velocity = 0;
        double fadeIn = 0.0;
        double fadeOut = 0.0;
    };
}

#endif //TALCS_MIDISINEWAVESYNTHESIZER_P_H
