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

#include "SDLAudioDriver_p.h"

#include <QDebug>
#include <QTimer>

#include <SDL2/SDL.h>

#include "SDLAudioDevice_p.h"

namespace talcs {

    static QString getDisplayName(const QString &driverName) {
        static QHash<QString, QString> map = {
                {"pipewire",    "Pipewire (SDL)"                   },
                {"pulseaudio",  "PulseAudio (SDL)"                 },
                {"alsa",        "ALSA PCM audio (SDL)"             },
                {"jack",        "JACK Audio Connection Kit (SDL)"  },
                {"sndio",       "OpenBSD sndio (SDL)"              },
                {"netbsd",      "NetBSD audio (SDL)"               },
                {"dsp",         "OSS /dev/dsp standard audio (SDL)"},
                {"qsa",         "QNX QSA Audio (SDL)"              },
                {"audio",       "UNIX /dev/audio interface (SDL)"  },
                {"arts",        "Analog RealTime Synthesizer (SDL)"},
                {"esd",         "Enlightened Sound Daemon (SDL)"   },
                {"nacl",        "SDL NaCl Audio Driver (SDL)"      },
                {"nas",         "Network Audio System (SDL)"       },
                {"wasapi",      "WASAPI (SDL)"                     },
                {"directsound", "DirectSound (SDL)"                },
                {"winmm",       "Windows Waveform Audio (SDL)"     },
                {"paud",        "AIX Paudio (SDL)"                 },
                {"haiku",       "Haiku BSoundPlayer (SDL)"         },
                {"coreaudio",   "CoreAudio (SDL)"                  },
                {"disk",        "direct-to-disk audio (SDL)"       },
                {"dummy",       "SDL dummy audio driver (SDL)"     },
                {"fusionsound", "FusionSound (SDL)"                },
                {"AAudio",      "AAudio audio driver (SDL)"        },
                {"openslES",    "opensl ES audio driver (SDL)"     },
                {"android",     "SDL Android audio driver (SDL)"   },
                {"ps2",         "PS2 audio driver (SDL)"           },
                {"psp",         "PSP audio driver (SDL)"           },
                {"vita",        "VITA audio driver (SDL)"          },
                {"n3ds",        "SDL N3DS audio driver (SDL)"      },
                {"emscripten",  "SDL emscripten audio driver (SDL)"},
                {"DART",        "OS/2 DART (SDL)"                  },
        };
        return map.value(driverName, driverName);
    }

    SDLAudioDriver::SDLAudioDriver(QObject * parent) : SDLAudioDriver(*new SDLAudioDriverPrivate, parent) {
        Q_D(SDLAudioDriver);
        d->eventPoller.reset(new SDLEventPoller);
        d->eventPollerThread = new QThread(this);
        d->eventPoller->moveToThread(d->eventPollerThread);
        connect(d->eventPollerThread, &QThread::started, d->eventPoller.data(), &SDLEventPoller::start);
        connect(d->eventPoller.data(), &SDLEventPoller::event, this,
                [=](const QByteArray &sdlEventData) { d->handleSDLEvent(sdlEventData); });
    }

    SDLAudioDriver::SDLAudioDriver(SDLAudioDriverPrivate & d, QObject * parent) : AudioDriver(d, parent) {
    }

    SDLAudioDriver::~SDLAudioDriver() {
        Q_D(SDLAudioDriver);
        SDLAudioDriver::finalize();
    }

    bool SDLAudioDriver::initialize() {
        Q_D(SDLAudioDriver);
        if (SDL_Init(SDL_INIT_AUDIO) == 0 && SDL_AudioInit(d->internalName.toLocal8Bit()) == 0) {
            QTimer::singleShot(0, [=] {
                SDL_FlushEvents(SDL_AUDIODEVICEADDED, SDL_AUDIODEVICEREMOVED);
                d->eventPollerThread->start();
            });
            return AudioDriver::initialize();
        } else {
            setErrorString(SDL_GetError());
            return false;
        }
    }
    void SDLAudioDriver::finalize() {
        Q_D(SDLAudioDriver);
        d->eventPoller->quit();
        d->eventPollerThread->quit();
        d->eventPollerThread->wait();
        SDL_AudioQuit();
        SDL_Quit();
        AudioDriver::finalize();
    }
    QStringList SDLAudioDriver::devices() const {
        Q_D(const SDLAudioDriver);
        QStringList list;
        if (d->createVirtualDefaultDevice) {
            char *name;
            SDL_AudioSpec spec;
            if (SDL_GetDefaultAudioInfo(&name, &spec, 0) == 0) {
                SDL_free(name);
                list.append("");
            }
        }
        int cnt = SDL_GetNumAudioDevices(0);
        for (int i = 0; i < cnt; i++) {
            list.append(QString::fromUtf8(SDL_GetAudioDeviceName(i, 0)));
        }
        return list;
    }
    QString SDLAudioDriver::defaultDevice() const {
        Q_D(const SDLAudioDriver);
        if (d->createVirtualDefaultDevice) {
            return "";
        }
        char *name;
        SDL_AudioSpec spec;
        if (SDL_GetDefaultAudioInfo(&name, &spec, 0) == 0) {
            auto s = QString::fromUtf8(name);
            SDL_free(name);
            return s;
        } else {
            return {};
        }
    }

    AudioDevice *SDLAudioDriver::createDefaultDevice() {
        Q_D(SDLAudioDriver);
        {
            char *name;
            SDL_AudioSpec spec;
            if (SDL_GetDefaultAudioInfo(&name, &spec, 0) != 0) {
                return nullptr;
            }
            SDL_free(name);
        }
        if (d->createVirtualDefaultDevice) {
            return new SDLAudioDevice({}, this);
        } else {
            auto name = defaultDevice();
            if (!name.isEmpty()) {
                return new SDLAudioDevice(defaultDevice(), this);
            }
            return nullptr;
        }

    }

    AudioDevice *SDLAudioDriver::createDevice(const QString &name) {
        auto dev = new SDLAudioDevice(name, this);
        return dev;
    }

    void SDLAudioDriver::addOpenedDevice(quint32 devId, SDLAudioDevice * dev) {
        Q_D(SDLAudioDriver);
        d->openedDevices.insert(devId, dev);
    }

    void SDLAudioDriver::removeOpenedDevice(quint32 devId) {
        Q_D(SDLAudioDriver);
        d->openedDevices.remove(devId);
    }
    QList<SDLAudioDriver *> SDLAudioDriver::getDrivers(bool createVirtualDefaultDevice) {
        int cnt = SDL_GetNumAudioDrivers();
        QList<SDLAudioDriver *> list;
        for (int i = 0; i < cnt; i++) {
            auto name = QString::fromLatin1(SDL_GetAudioDriver(i));
            if (name == "disk" || name == "dummy")
                continue;
            auto drv = new SDLAudioDriver;
            drv->d_func()->internalName = name;
            drv->d_func()->createVirtualDefaultDevice = createVirtualDefaultDevice;
            drv->setName(getDisplayName(name));
            list.append(drv);
        }
        return list;
    }

    void SDLAudioDriverPrivate::handleSDLEvent(const QByteArray &sdlEventData) {
        Q_Q(SDLAudioDriver);
        auto *e = reinterpret_cast<const SDL_Event *>(sdlEventData.data());
        if (e->type == SDL_AUDIODEVICEADDED && e->adevice.iscapture == 0) {
            qDebug() << "SDL_AUDIODEVICEADDED";
            emit q->deviceChanged();
        } else if (e->type == SDL_AUDIODEVICEREMOVED && e->adevice.iscapture == 0) {
            qDebug() << "SDL_AUDIODEVICEREMOVED";
            handleDeviceRemoved(e->adevice.which);
            emit q->deviceChanged();
        }
    }
    void SDLAudioDriverPrivate::handleDeviceRemoved(quint32 devId) {
        auto it = openedDevices.find(devId);
        if (it == openedDevices.end())
            return;
        auto dev = it.value();
        dev->close();
        emit dev->closed();
    }

    /**
     * @internal
     * @class SDLEventPoller
     * The object runs in another thread to poll event from SDL2.
     */

    /**
     * @internal
     * @fn void SDLEventPoller::event(QByteArray sdlEventData)
     * Emitted when received event.
     * @param sdlEventData the raw buffer of SDL_Event struct
     */

    void SDLEventPoller::start() {
        stopRequested = false;
        SDL_Event e;
        while (!stopRequested) {
            while (SDL_PollEvent(&e) > 0) {
                emit event(QByteArray(reinterpret_cast<char *>(&e), sizeof(SDL_Event)));
            }
            SDL_Delay(1);
        }
    }
    void SDLEventPoller::quit() {
        stopRequested = true;
    }
}
