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

#ifndef TALCS_MIDIMESSAGEINTEGRATOR_P_H
#define TALCS_MIDIMESSAGEINTEGRATOR_P_H

#include <TalcsMidi/MidiMessageIntegrator.h>

#include <TalcsMidi/private/AbstractMidiMessageIntegrator_p.h>

namespace talcs {

    struct MidiMessageIntegratorQueue {
        MidiMessage p[1024];
        QAtomicInteger<size_t> head = 0;
        size_t tail = 0;
        QAtomicInteger<size_t> counter = 0;
        void push(const MidiMessage &message) {
            if (counter.loadAcquire() == 1024)
                return;
            counter.fetchAndAddOrdered(1);
            p[tail++] = message;
            tail %= 1024;
        }
        void clear() {
            counter.storeRelease(0);
            tail = head;
            tail %= 1024;
        }
        bool empty() const {
            return !counter;
        }
        MidiMessage top() {
            if (!counter.loadAcquire())
                return {0xf0, 0xf7, -qInf()};
            return p[head];
        }
        void pop() {
            if (!counter.loadAcquire())
                return;
            counter.fetchAndAddOrdered(-1);
            head++;
            head = head % 1024;
        }
    };

    class MidiMessageIntegratorPrivate : public AbstractMidiMessageIntegratorPrivate {
        Q_DECLARE_PUBLIC(MidiMessageIntegrator)
    public:
        MidiMessageIntegratorQueue queue;
    };
}

#endif //TALCS_MIDIMESSAGEINTEGRATOR_P_H
