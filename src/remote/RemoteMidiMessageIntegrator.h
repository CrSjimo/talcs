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

#ifndef TALCS_REMOTEMIDIMESSAGEINTEGRATOR_H
#define TALCS_REMOTEMIDIMESSAGEINTEGRATOR_H

#include <TalcsMidi/AbstractMidiMessageIntegrator.h>
#include <TalcsRemote/RemoteAudioDevice.h>

namespace talcs {

    class RemoteMidiMessageIntegratorPrivate;

    class TALCSREMOTE_EXPORT RemoteMidiMessageIntegrator : public AbstractMidiMessageIntegrator, public RemoteProcessInfoCallback {
        Q_DECLARE_PRIVATE(RemoteMidiMessageIntegrator)
    public:
        explicit RemoteMidiMessageIntegrator();
        ~RemoteMidiMessageIntegrator() override;

        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

    protected:
        QList<IntegratedMidiMessage> fetch(qint64 length) override;
        void onThisBlockProcessInfo(const RemoteProcessInfo &processInfo) override;

        explicit RemoteMidiMessageIntegrator(RemoteMidiMessageIntegratorPrivate &d);

    };

} // talcs

#endif //TALCS_REMOTEMIDIMESSAGEINTEGRATOR_H
