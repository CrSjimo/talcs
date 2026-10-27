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

#include "MidiMessageListener.h"
#include "MidiMessageListener_p.h"

#include <cmath>

#include <QMutexLocker>

namespace talcs {
    MidiMessageListener::MidiMessageListener() : MidiMessageListener(*new MidiMessageListenerPrivate) {

    }

    MidiMessageListener::~MidiMessageListener() = default;

    bool MidiMessageListener::deviceWillStartCallback(MidiInputDevice *device) {
        Q_D(MidiMessageListener);
        QMutexLocker locker(&d->filterMutex);
        const auto filters = d->filters;
        for (auto filter : filters) {
            if (d->filters.contains(filter) && !filter->deviceWillStartCallback(device))
                return false;
        }
        return processDeviceWillStart(device);
    }

    void MidiMessageListener::deviceStoppedCallback() {
        Q_D(MidiMessageListener);
        QMutexLocker locker(&d->filterMutex);
        const auto filters = d->filters;
        for (auto filter : filters) {
            if (d->filters.contains(filter))
                filter->deviceStoppedCallback();
        }
        processDeviceStopped();
    }

    bool MidiMessageListener::messageCallback(const MidiMessage &message) {
        return eventCallback({message});
    }

    bool MidiMessageListener::eventCallback(const MidiInputEvent &event) {
        Q_D(MidiMessageListener);
        QMutexLocker locker(&d->filterMutex);
        if (event.type == MidiInputEvent::Message && !event.message.isValid())
            return false;
        auto normalized = event;
        if (!std::isfinite(normalized.message.getTimeStamp()) || normalized.message.getTimeStamp() <= 0)
            normalized.message.setTimeStamp(MidiInputEvent::currentTime());
        const auto filters = d->filters;
        for (auto filter : filters) {
            if (d->filters.contains(filter) && filter->eventCallback(normalized) && event.type == MidiInputEvent::Message)
                return true;
        }
        return processEvent(normalized);
    }

    bool MidiMessageListener::processEvent(const MidiInputEvent &event) {
        return event.type == MidiInputEvent::Message && processMessage(event.message);
    }

    void MidiMessageListener::errorCallback(const QString &errorString) {
        Q_D(MidiMessageListener);
        QMutexLocker locker(&d->filterMutex);
        const auto filters = d->filters;
        for (auto filter : filters) {
            if (d->filters.contains(filter))
                filter->errorCallback(errorString);
        }
        processError(errorString);
    }

    void MidiMessageListener::addFilter(MidiMessageListener *filter) {
        Q_D(MidiMessageListener);
        QMutexLocker locker(&d->filterMutex);
        if (filter && filter != this && !d->filters.contains(filter))
            d->filters.append(filter);
    }

    void MidiMessageListener::removeFilter(MidiMessageListener *filter) {
        Q_D(MidiMessageListener);
        QMutexLocker locker(&d->filterMutex);
        d->filters.removeOne(filter);
    }

    bool MidiMessageListener::processDeviceWillStart(MidiInputDevice *device) {
        return true;
    }
    void MidiMessageListener::processDeviceStopped() {
    }
    bool MidiMessageListener::processMessage(const MidiMessage &message) {
        return false;
    }
    void MidiMessageListener::processError(const QString &errorString) {
    }

    MidiMessageListener::MidiMessageListener(MidiMessageListenerPrivate &d) : d_ptr(&d) {
        d.q_ptr = this;
    }
} // talcs
