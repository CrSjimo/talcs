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

#ifndef TALCS_MIDISINEWAVESYNTHESIZER_H
#define TALCS_MIDISINEWAVESYNTHESIZER_H

#include <TalcsCore/AudioSource.h>
#include <TalcsMidi/MidiInputDeviceCallback.h>

namespace talcs {

    class MidiSineWaveSynthesizerPrivate;

    class TALCSMIDI_EXPORT MidiSineWaveSynthesizer : public AudioSource, public MidiInputDeviceCallback {
    public:
        MidiSineWaveSynthesizer();

        bool open(qint64 bufferSize, double sampleRate) override;

        void close() override;

        ~MidiSineWaveSynthesizer() override;

        qint64 read(const AudioSourceReadData &readData) override;

        void deviceWillStartCallback(MidiInputDevice *device) override;

        void deviceStoppedCallback() override;

        void workCallback(const MidiMessage &message) override;

        void errorCallback(const QString &errorString) override;

    private:
        QScopedPointer<MidiSineWaveSynthesizerPrivate> d;
    };

} // talcs

#endif //TALCS_MIDISINEWAVESYNTHESIZER_H
