/******************************************************************************
 * Copyright (c) 2024 CrSjimo                                                 *
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

#ifndef TALCS_METRONOMEAUDIOSOURCE_H
#define TALCS_METRONOMEAUDIOSOURCE_H

#include <TalcsCore/PositionableAudioSource.h>

namespace talcs {

    class MetronomeAudioSourceBeatDetector {
    public:
        virtual void initialize() = 0;
        virtual void detectInterval(qint64 intervalStart, qint64 intervalLength) = 0;
        virtual QPair<qint64, bool> nextBeat() = 0;
    };

    class MetronomeAudioSourcePrivate;

    class TALCSCORE_EXPORT MetronomeAudioSource : public PositionableAudioSource {
        Q_DECLARE_PRIVATE(MetronomeAudioSource)
    public:
        MetronomeAudioSource();
        ~MetronomeAudioSource() override;

        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;
        qint64 read(const AudioSourceReadData &readData) override;
        qint64 length() const override;
        qint64 nextReadPosition() const override;
        void setNextReadPosition(qint64 pos) override;

        bool setMajorBeatSource(PositionableAudioSource *src, bool takeOwnership = false);
        PositionableAudioSource *majorBeatSource() const;
        bool setMinorBeatSource(PositionableAudioSource *src, bool takeOwnership = false);
        PositionableAudioSource *minorBeatSource() const;

        void setDetector(MetronomeAudioSourceBeatDetector *detector);
        MetronomeAudioSourceBeatDetector *detector() const;

        static PositionableAudioSource *builtInMajorBeatSource();
        static PositionableAudioSource *builtInMinorBeatSource();

    protected:
        explicit MetronomeAudioSource(MetronomeAudioSourcePrivate &d);
    };

} // talcs

#endif //TALCS_METRONOMEAUDIOSOURCE_H
