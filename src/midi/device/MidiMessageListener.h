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

#ifndef TALCS_MIDIMESSAGELISTENER_H
#define TALCS_MIDIMESSAGELISTENER_H

#include <QScopedPointer>

#include <TalcsMidi/MidiMessage.h>

namespace talcs {

    class MidiInputDevice;

    class MidiMessageListenerPrivate;

    class TALCSMIDI_EXPORT MidiMessageListener {
        Q_DECLARE_PRIVATE(MidiMessageListener)
    public:
        explicit MidiMessageListener();
        virtual ~MidiMessageListener();

        bool deviceWillStartCallback(MidiInputDevice *device);
        void deviceStoppedCallback();
        bool messageCallback(const MidiMessage &message);
        void errorCallback(const QString &errorString);

        void addFilter(MidiMessageListener *filter);
        void removeFilter(MidiMessageListener *filter);

    protected:
        virtual bool processDeviceWillStart(MidiInputDevice *device) = 0;
        virtual void processDeviceStopped() = 0;
        virtual bool processMessage(const MidiMessage &message) = 0;
        virtual void processError(const QString &errorString) = 0;

        explicit MidiMessageListener(MidiMessageListenerPrivate &d);

        QScopedPointer<MidiMessageListenerPrivate> d_ptr;

    };

} // talcs

#endif //TALCS_MIDIMESSAGELISTENER_H
