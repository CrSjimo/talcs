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

#ifndef TALCS_REMOTEMIDIMESSAGEINTEGRATOR_P_H
#define TALCS_REMOTEMIDIMESSAGEINTEGRATOR_P_H

#include <TalcsRemote/RemoteMidiMessageIntegrator.h>

#include <TalcsMidi/private/AbstractMidiMessageIntegrator_p.h>

namespace talcs {
    class RemoteMidiMessageIntegratorPrivate : public AbstractMidiMessageIntegratorPrivate {
        Q_DECLARE_PUBLIC(RemoteMidiMessageIntegrator)
    public:
        QList<IntegratedMidiMessage> midiMessages;
        QMutex mutex;
    };
}

#endif //TALCS_REMOTEMIDIMESSAGEINTEGRATOR_P_H
