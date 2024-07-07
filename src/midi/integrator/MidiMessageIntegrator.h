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

#ifndef TALCS_MIDIMESSAGEINTEGRATOR_H
#define TALCS_MIDIMESSAGEINTEGRATOR_H

#include <TalcsMidi/AbstractMidiMessageIntegrator.h>
#include <TalcsMidi/MidiMessageListener.h>

namespace talcs {

    class MidiMessageIntegratorPrivate;

    class TALCSMIDI_EXPORT MidiMessageIntegrator : public AbstractMidiMessageIntegrator, public MidiMessageListener {
        Q_DECLARE_PRIVATE_D(AbstractMidiMessageIntegrator::d_ptr, MidiMessageIntegrator)
    public:
        explicit MidiMessageIntegrator();
        ~MidiMessageIntegrator() override;

        bool open(qint64 bufferSize, double sampleRate) override;

        void close() override;

        void flush();

    protected:
        bool processDeviceWillStart(MidiInputDevice *device) override;
        void processDeviceStopped() override;
        bool processMessage(const MidiMessage &message) override;
        void processError(const QString &errorString) override;

        QList<IntegratedMidiMessage> fetch(qint64 length) override;

        explicit MidiMessageIntegrator(MidiMessageIntegratorPrivate &d);

    };

} // talcs

#endif //TALCS_MIDIMESSAGEINTEGRATOR_H
