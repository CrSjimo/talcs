/******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                 *
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

#ifndef TALCS_MIDIINPUTDEVICE_H
#define TALCS_MIDIINPUTDEVICE_H

#include <QObject>

#include <TalcsCore/ErrorStringProvider.h>
#include <TalcsCore/NameProvider.h>
#include <TalcsMidi/TalcsMidiGlobal.h>

namespace talcs {

    class MidiInputDeviceCallback;

    class MidiInputDevicePrivate;

    class TALCSMIDI_EXPORT MidiInputDevice : public QObject, public NameProvider, public ErrorStringProvider {
        Q_OBJECT
    public:
        explicit MidiInputDevice(int deviceIndex, QObject *parent = nullptr);
        ~MidiInputDevice();

        bool open();
        bool isOpen() const;
        void close();

        void addListener(MidiInputDeviceCallback *callback);
        void removeListener(MidiInputDeviceCallback *callback);

        static QStringList devices();

    private:
        QScopedPointer<MidiInputDevicePrivate> d;
    };

} // talcs

#endif //TALCS_MIDIINPUTDEVICE_H
