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

#ifndef TALCS_AUDIODRIVERMANAGER_H
#define TALCS_AUDIODRIVERMANAGER_H

#include <QObject>

#include <TalcsDevice/TalcsDeviceGlobal.h>

namespace talcs {

    class AudioDriverManagerPrivate;

    class AudioDriver;

    class TALCSDEVICE_EXPORT AudioDriverManager : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(AudioDriverManager)
    public:
        explicit AudioDriverManager(QObject *parent = nullptr);
        ~AudioDriverManager() override;

        bool addAudioDriver(AudioDriver *driver);
        bool removeDriver(AudioDriver *driver);

        AudioDriver *driver(const QString &name) const;
        QStringList drivers() const;

        static AudioDriverManager *createBuiltInDriverManager(QObject *parent = nullptr);

    signals:
        void driverAdded(AudioDriver *driver);
        void driverRemoved(AudioDriver *driver);

    protected:
        AudioDriverManager(AudioDriverManagerPrivate &d, QObject *parent);
        QScopedPointer<AudioDriverManagerPrivate> d_ptr;
    };
    
}

#endif // TALCS_AUDIODRIVERMANAGER_H
