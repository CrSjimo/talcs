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

#ifndef TALCS_SINEWAVENOTESYNTHESIZER_H
#define TALCS_SINEWAVENOTESYNTHESIZER_H

#include <TalcsCore/AudioSource.h>

namespace talcs {

    struct SineWaveNoteSynthesizerDetectorMessage {
        qint64 position;
        double frequency;
        double velocity;
        bool isNoteOn;
    };

    class SineWaveNoteSynthesizerDetector {
    public:
        virtual void detectInterval(qint64 intervalLength) = 0;
        virtual SineWaveNoteSynthesizerDetectorMessage nextMessage() = 0;
    };

    class SineWaveNoteSynthesizerPrivate;

    class TALCSCORE_EXPORT SineWaveNoteSynthesizer : public AudioSource {
        Q_DECLARE_PRIVATE(SineWaveNoteSynthesizer)
    public:
        explicit SineWaveNoteSynthesizer();
        ~SineWaveNoteSynthesizer() override;

        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        void setDetector(SineWaveNoteSynthesizerDetector *detector);
        SineWaveNoteSynthesizerDetector *detector() const;

    protected:
        explicit SineWaveNoteSynthesizer(SineWaveNoteSynthesizerPrivate &d);
        qint64 processReading(const AudioSourceReadData &readData) override;

    };

}

#endif //TALCS_SINEWAVENOTESYNTHESIZER_H
