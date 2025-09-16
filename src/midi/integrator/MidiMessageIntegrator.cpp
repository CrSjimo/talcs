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

#include "MidiMessageIntegrator.h"
#include "MidiMessageIntegrator_p.h"

#include <QList>

namespace talcs {
    MidiMessageIntegrator::MidiMessageIntegrator() : MidiMessageIntegrator(*new MidiMessageIntegratorPrivate) {

    }

    MidiMessageIntegrator::~MidiMessageIntegrator() = default;

    bool MidiMessageIntegrator::open(qint64 bufferSize, double sampleRate) {
        Q_D(MidiMessageIntegrator);
        d->queue.clear();
        return AbstractMidiMessageIntegrator::open(bufferSize, sampleRate);
    }

    void MidiMessageIntegrator::close() {
        Q_D(MidiMessageIntegrator);
        d->queue.clear();
        AbstractMidiMessageIntegrator::close();
    }

    void MidiMessageIntegrator::flush() {
        Q_D(MidiMessageIntegrator);
        d->queue.clear();
    }

    bool MidiMessageIntegrator::processDeviceWillStart(MidiInputDevice *device) {
        Q_D(MidiMessageIntegrator);
        d->queue.clear();
        return true;
    }

    void MidiMessageIntegrator::processDeviceStopped() {
        Q_D(MidiMessageIntegrator);
        d->queue.clear();
    }

    bool MidiMessageIntegrator::processMessage(const MidiMessage &message) {
        Q_D(MidiMessageIntegrator);
        d->queue.push(message);
        return false;
    }

    void MidiMessageIntegrator::processError(const QString &errorString) {
        Q_D(MidiMessageIntegrator);
        d->queue.push({0xf0, 0xf7, -qInf()});
    }

    QList<IntegratedMidiMessage> MidiMessageIntegrator::fetch(qint64 length) {
        Q_D(MidiMessageIntegrator);
        if (d->queue.empty())
            return {};
        QList<IntegratedMidiMessage> midiEvents = {{0, d->queue.top()}};
        d->queue.pop();
        while (!d->queue.empty()) {
            auto message = d->queue.top();
            auto position = static_cast<qint64>((message.getTimeStamp() - midiEvents[0].message.getTimeStamp()) * sampleRate());
            if (position >= length)
                break;
            d->queue.pop();
            midiEvents.append({position, message});
        }
        return midiEvents;
    }

    MidiMessageIntegrator::MidiMessageIntegrator(MidiMessageIntegratorPrivate &d) : AbstractMidiMessageIntegrator(d) {

    }
} // talcs
