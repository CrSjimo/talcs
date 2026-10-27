/******************************************************************************
 * Copyright (c) 2023-2024 CrSjimo                                            *
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

#ifndef TALCS_MIDIINPUTEVENT_H
#define TALCS_MIDIINPUTEVENT_H

#include <QMetaType>

#include <TalcsMidi/MidiMessage.h>

namespace talcs {

    struct TALCSMIDI_EXPORT MidiInputEvent {
        enum Type {
            Message,
            SourceStopped,
            AllSourcesStopped,
        };

        MidiMessage message;
        quint64 sourceId{};
        Type type{Message};

        /// Returns seconds from the process-wide steady clock used by MIDI input events.
        static double currentTime() noexcept;
        /// Returns a process-wide source identity for physical or callback-based inputs.
        static quint64 createSourceId() noexcept;
    };

}

Q_DECLARE_METATYPE(talcs::MidiInputEvent)

#endif // TALCS_MIDIINPUTEVENT_H
