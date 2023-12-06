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

#ifndef AUDIOSOURCEPLAYBACK_H
#define AUDIOSOURCEPLAYBACK_H

#include <QScopedPointer>

#include <TalcsDevice/AudioDeviceCallback.h>

namespace talcs {

    class AudioSourcePlaybackPrivate;
    class AudioSource;

    class TALCSDEVICE_EXPORT AudioSourcePlayback : public AudioDeviceCallback {
        Q_DECLARE_PRIVATE(AudioSourcePlayback)
    public:
        explicit AudioSourcePlayback(AudioSource *src, bool takeOwnership = false);
        ~AudioSourcePlayback();

        AudioSource *source() const;
        void setSource(AudioSource *src, bool takeOwnership = false);

        bool deviceWillStartCallback(AudioDevice *device) override;
        void deviceStoppedCallback() override;
        void workCallback(const AudioSourceReadData &readData) override;

    protected:
        explicit AudioSourcePlayback(AudioSourcePlaybackPrivate &d);
        QScopedPointer<AudioSourcePlaybackPrivate> d_ptr;
    };
    
}

#endif // AUDIOSOURCEPLAYBACK_H
