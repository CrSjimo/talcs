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

#ifndef TALCS_AUDIOSOURCE_H
#define TALCS_AUDIOSOURCE_H

#include <QScopedPointer>

#include <TalcsCore/IAudioSampleContainer.h>
#include <TalcsCore/AudioStreamBase.h>
#include <TalcsCore/DuplicatableObject.h>

namespace talcs {

    struct TALCSCORE_EXPORT AudioSourceReadData {
        AudioSourceReadData(IAudioSampleContainer *buffer);
        AudioSourceReadData(IAudioSampleContainer *buffer, qint64 startPos, qint64 length, int silentFlags = 0);
        IAudioSampleContainer *buffer;
        qint64 startPos;
        qint64 length;
        int silentFlags;
    };

    class AudioSourcePrivate;

    class TALCSCORE_EXPORT AudioSource : public AudioStreamBase, public DuplicatableObject {
        Q_DECLARE_PRIVATE(AudioSource)
    public:
        AudioSource();
        virtual ~AudioSource();
        virtual qint64 read(const AudioSourceReadData &readData) = 0;

    protected:
        explicit AudioSource(AudioSourcePrivate &d);
        QScopedPointer<AudioSourcePrivate> d_ptr;
    };
    
}

#endif // TALCS_AUDIOSOURCE_H
