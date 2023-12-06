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

#ifndef TALCS_MEMORYAUDIOSOURCE_H
#define TALCS_MEMORYAUDIOSOURCE_H

#include <QScopedPointer>

#include <TalcsCore/PositionableAudioSource.h>

namespace talcs {

    class MemoryAudioSourcePrivate;

    class TALCSCORE_EXPORT MemoryAudioSource : public PositionableAudioSource {
        Q_DECLARE_PRIVATE(MemoryAudioSource)
    public:
        explicit MemoryAudioSource(IAudioSampleProvider *buffer = nullptr, bool takeOwnership = false);
        ~MemoryAudioSource() override;

        IAudioSampleProvider *buffer() const;
        IAudioSampleProvider *setBuffer(IAudioSampleProvider *newBuffer, bool takeOwnership = false);

        qint64 read(const AudioSourceReadData &readData) override;
        qint64 length() const override;

        void setNextReadPosition(qint64 pos) override;

        bool isDuplicatable() const override;

        DuplicatableObject *duplicate() const override;

    protected:
        explicit MemoryAudioSource(MemoryAudioSourcePrivate &d);
    };

}

#endif // TALCS_MEMORYAUDIOSOURCE_H
