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

#include "MidiMessageIntegrator.h"
#include "MidiMessageIntegrator_p.h"

#include <algorithm>
#include <cmath>
#include <mutex>

namespace talcs {

    MidiMessageIntegrator::MidiMessageIntegrator() : MidiMessageIntegrator(*new MidiMessageIntegratorPrivate) {
    }

    MidiMessageIntegrator::~MidiMessageIntegrator() {
        close();
    }

    bool MidiMessageIntegrator::open(qint64 bufferSize, double sampleRate) {
        Q_D(MidiMessageIntegrator);
        if (bufferSize <= 0 || !std::isfinite(sampleRate) || sampleRate <= 0)
            return false;
        {
            QMutexLocker locker(&d->queueMutex);
            d->queue.clear();
            d->queue.reserve(d->capacity);
            d->accepting = false;
        }
        const bool result = AbstractMidiMessageIntegrator::open(bufferSize, sampleRate);
        QMutexLocker locker(&d->queueMutex);
        d->accepting = result;
        return result;
    }

    void MidiMessageIntegrator::close() {
        Q_D(MidiMessageIntegrator);
        {
            QMutexLocker locker(&d->queueMutex);
            d->accepting = false;
            d->queue.clear();
        }
        AbstractMidiMessageIntegrator::close();
    }

    void MidiMessageIntegrator::flush() {
        Q_D(MidiMessageIntegrator);
        QMutexLocker locker(&d->queueMutex);
        d->queue.clear();
        if (d->accepting)
            d->queue.push_back({MidiMessage().withTimeStamp(MidiInputEvent::currentTime()), 0, MidiInputEvent::AllSourcesStopped});
    }

    quint64 MidiMessageIntegrator::droppedEventCount() const {
        Q_D(const MidiMessageIntegrator);
        QMutexLocker locker(&d->queueMutex);
        return d->droppedEvents;
    }

    bool MidiMessageIntegrator::enqueueBlock(const QList<IntegratedMidiMessage> &events, double blockTime, double blockSampleRate) {
        if (!std::isfinite(blockTime) || !std::isfinite(blockSampleRate) || blockSampleRate <= 0)
            return false;
        const auto droppedBefore = droppedEventCount();
        for (const auto &event : events) {
            if (event.position < 0 || !std::isfinite(blockTime + event.position / blockSampleRate) ||
                (event.type == MidiInputEvent::Message && !event.message.isValid()))
                return false;
            eventCallback({event.message.withTimeStamp(blockTime + event.position / blockSampleRate), event.sourceId, event.type});
        }
        Q_D(MidiMessageIntegrator);
        QMutexLocker locker(&d->queueMutex);
        return d->accepting && droppedBefore == d->droppedEvents;
    }

    bool MidiMessageIntegrator::processEvent(const MidiInputEvent &input) {
        Q_D(MidiMessageIntegrator);
        if (input.type == MidiInputEvent::Message && !input.message.isValid())
            return false;
        auto event = input;
        if (!std::isfinite(event.message.getTimeStamp()) || event.message.getTimeStamp() <= 0)
            event.message.setTimeStamp(MidiInputEvent::currentTime());
        QMutexLocker locker(&d->queueMutex);
        if (!d->accepting)
            return false;
        if (event.type == MidiInputEvent::AllSourcesStopped) {
            d->queue.clear();
        } else if (event.type == MidiInputEvent::SourceStopped) {
            d->queue.erase(std::remove_if(d->queue.begin(), d->queue.end(), [&](const auto &queued) { return queued.sourceId == event.sourceId; }), d->queue.end());
        }
        if (d->queue.size() == d->capacity) {
            d->droppedEvents += d->queue.size();
            d->queue.clear();
            d->queue.push_back({MidiMessage().withTimeStamp(MidiInputEvent::currentTime()), 0, MidiInputEvent::AllSourcesStopped});
        }
        const auto position = std::upper_bound(d->queue.begin(), d->queue.end(), event.message.getTimeStamp(),
            [](double time, const MidiInputEvent &queued) { return time < queued.message.getTimeStamp(); });
        d->queue.insert(position, event);
        return false;
    }

    QList<IntegratedMidiMessage> MidiMessageIntegrator::fetch(qint64 length) {
        Q_D(MidiMessageIntegrator);
        std::unique_lock locker(d->queueMutex, std::try_to_lock);
        if (!locker.owns_lock() || length <= 0)
            return {};
        // Live input is rendered one block after capture. Late events are applied at the block start.
        const double end = MidiInputEvent::currentTime();
        const double start = end - static_cast<double>(length) / sampleRate();
        QList<IntegratedMidiMessage> result;
        const auto last = std::upper_bound(d->queue.begin(), d->queue.end(), end,
            [](double time, const MidiInputEvent &event) { return time < event.message.getTimeStamp(); });
        result.reserve(static_cast<qsizetype>(last - d->queue.begin()));
        for (auto it = d->queue.begin(); it != last; ++it) {
            const double offset = std::clamp((it->message.getTimeStamp() - start) * sampleRate(), 0.0, static_cast<double>(length - 1));
            result.append({static_cast<qint64>(offset), it->message, it->sourceId, it->type});
        }
        d->queue.erase(d->queue.begin(), last);
        return result;
    }

    MidiMessageIntegrator::MidiMessageIntegrator(MidiMessageIntegratorPrivate &d) : AbstractMidiMessageIntegrator(d) {
    }

}
