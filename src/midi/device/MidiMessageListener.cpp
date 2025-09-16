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

#include <algorithm>

namespace talcs {
    MidiMessageListener::MidiMessageListener() : MidiMessageListener(*new MidiMessageListenerPrivate) {

    }

    MidiMessageListener::~MidiMessageListener() = default;

    bool MidiMessageListener::deviceWillStartCallback(MidiInputDevice *device) {
        Q_D(MidiMessageListener);
        QMutexLocker locker(&d->filterMutex);
        return std::all_of(d->filters.cbegin(), d->filters.cend(), [device](MidiMessageListener *filter) {
            return filter->deviceWillStartCallback(device);
        }) && processDeviceWillStart(device);
    }

    void MidiMessageListener::deviceStoppedCallback() {
        Q_D(MidiMessageListener);
        QMutexLocker locker(&d->filterMutex);
        std::for_each(d->filters.cbegin(), d->filters.cend(), [](MidiMessageListener *filter) {
            filter->deviceStoppedCallback();
        });
        processDeviceStopped();
    }

    bool MidiMessageListener::messageCallback(const MidiMessage &message) {
        Q_D(MidiMessageListener);
        QMutexLocker locker(&d->filterMutex);
        return std::any_of(d->filters.cbegin(), d->filters.cend(), [&message](MidiMessageListener *filter) {
            return filter->messageCallback(message);
        }) || processMessage(message);
    }

    void MidiMessageListener::errorCallback(const QString &errorString) {
        Q_D(MidiMessageListener);
        QMutexLocker locker(&d->filterMutex);
        std::for_each(d->filters.cbegin(), d->filters.cend(), [&errorString](MidiMessageListener *filter) {
            filter->errorCallback(errorString);
        });
        processError(errorString);
    }

    void MidiMessageListener::addFilter(MidiMessageListener *filter) {
        Q_D(MidiMessageListener);
        QMutexLocker locker(&d->filterMutex);
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
