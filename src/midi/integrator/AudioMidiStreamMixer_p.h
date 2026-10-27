/******************************************************************************
 * Copyright (c) 2023-2024 CrSjimo                                            *
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

#ifndef TALCS_AUDIOMIDISTREAMMIXER_P_H
#define TALCS_AUDIOMIDISTREAMMIXER_P_H

#include <TalcsMidi/AudioMidiStreamMixer.h>

#include <QMutex>

#include <TalcsCore/AudioBuffer.h>
#include <TalcsMidi/private/AudioMidiStream_p.h>

namespace talcs {

    class AudioMidiStreamMixerPrivate : public AudioMidiStreamPrivate {
        Q_DECLARE_PUBLIC(AudioMidiStreamMixer)
    public:
        QMutex mutex;
        QList<AudioMidiStream *> streams;
        AudioBuffer scratch;
    };

}

#endif // TALCS_AUDIOMIDISTREAMMIXER_P_H
