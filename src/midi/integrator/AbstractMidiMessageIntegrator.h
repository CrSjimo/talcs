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

#ifndef TALCS_ABSTRACTMIDIMESSAGEINTEGRATOR_H
#define TALCS_ABSTRACTMIDIMESSAGEINTEGRATOR_H

#include <TalcsCore/AudioSource.h>
#include <TalcsMidi/IntegratedMidiMessage.h>

namespace talcs {

    class AudioMidiStream;

    class AbstractMidiMessageIntegratorPrivate;

    class TALCSMIDI_EXPORT AbstractMidiMessageIntegrator : public AudioSource {
        Q_DECLARE_PRIVATE(AbstractMidiMessageIntegrator)
    public:
        explicit AbstractMidiMessageIntegrator();
        ~AbstractMidiMessageIntegrator() override;

        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        void setStream(AudioMidiStream *stream, bool takeOwnership = false);
        AudioMidiStream *stream() const;

    protected:
        qint64 processReading(const AudioSourceReadData &readData) override;

        virtual QList<IntegratedMidiMessage> fetch(qint64 length) = 0;

        explicit AbstractMidiMessageIntegrator(AbstractMidiMessageIntegratorPrivate &d);
    };

} // talcs

#endif //TALCS_ABSTRACTMIDIMESSAGEINTEGRATOR_H
