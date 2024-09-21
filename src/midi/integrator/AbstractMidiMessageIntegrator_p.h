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


#ifndef TALCS_ABSTRACTMIDIMESSAGEINTEGRATOR_P_H
#define TALCS_ABSTRACTMIDIMESSAGEINTEGRATOR_P_H

#include <TalcsMidi/AbstractMidiMessageIntegrator.h>

#include <QMutex>

#include <TalcsCore/private/AudioSource_p.h>
#include <TalcsCore/TakeOwnershipPointer.h>

#include <TalcsMidi/AudioMidiStream.h>

namespace talcs {
    class AbstractMidiMessageIntegratorPrivate : public AudioSourcePrivate {
        Q_DECLARE_PUBLIC(AbstractMidiMessageIntegrator)
    public:
        QMutex mutex;
        TakeOwnershipPointer<AudioMidiStream> stream;
    };
}

#endif //TALCS_ABSTRACTMIDIMESSAGEINTEGRATOR_P_H
