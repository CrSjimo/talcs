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
#include <QLoggingCategory>
#include <QTimer>

#include <SDL2/SDL.h>

#include <TalcsDevice/private/SDLAudioDevice_p.h>

namespace talcs {

    Q_LOGGING_CATEGORY(lcSDLAudioDriver, "talcs.device.sdl.driver")

    static QString getDisplayName(const QString &driverName) {
        static QHash<QString, QString> map = {
                {"pipewire",    QT_TRANSLATE_NOOP("talcs::AudioDriver", "Pipewire (SDL)")                   },
                {"pulseaudio",  QT_TRANSLATE_NOOP("talcs::AudioDriver", "PulseAudio (SDL)")                 },
                {"alsa",        QT_TRANSLATE_NOOP("talcs::AudioDriver", "ALSA PCM audio (SDL)")             },
                {"jack",        QT_TRANSLATE_NOOP("talcs::AudioDriver", "JACK Audio Connection Kit (SDL)")  },
                {"sndio",       QT_TRANSLATE_NOOP("talcs::AudioDriver", "OpenBSD sndio (SDL)")              },
                {"netbsd",      QT_TRANSLATE_NOOP("talcs::AudioDriver", "NetBSD audio (SDL)")               },
                {"dsp",         QT_TRANSLATE_NOOP("talcs::AudioDriver", "OSS /dev/dsp standard audio (SDL)")},
                {"qsa",         QT_TRANSLATE_NOOP("talcs::AudioDriver", "QNX QSA Audio (SDL)")              },
                {"audio",       QT_TRANSLATE_NOOP("talcs::AudioDriver", "UNIX /dev/audio interface (SDL)")  },
                {"arts",        QT_TRANSLATE_NOOP("talcs::AudioDriver", "Analog RealTime Synthesizer (SDL)")},
                {"esd",         QT_TRANSLATE_NOOP("talcs::AudioDriver", "Enlightened Sound Daemon (SDL)")   },
                {"nacl",        QT_TRANSLATE_NOOP("talcs::AudioDriver", "SDL NaCl Audio Driver (SDL)")      },
                {"nas",         QT_TRANSLATE_NOOP("talcs::AudioDriver", "Network Audio System (SDL)")       },
                {"paud",        QT_TRANSLATE_NOOP("talcs::AudioDriver", "AIX Paudio (SDL)")                 },
                {"haiku",       QT_TRANSLATE_NOOP("talcs::AudioDriver", "Haiku BSoundPlayer (SDL)")         },
                {"coreaudio",   QT_TRANSLATE_NOOP("talcs::AudioDriver", "CoreAudio (SDL)")                  },
                {"disk",        QT_TRANSLATE_NOOP("talcs::AudioDriver", "direct-to-disk audio (SDL)")       },
                {"dummy",       QT_TRANSLATE_NOOP("talcs::AudioDriver", "SDL dummy audio driver (SDL)")     },
                {"fusionsound", QT_TRANSLATE_NOOP("talcs::AudioDriver", "FusionSound (SDL)")                },
                {"AAudio",      QT_TRANSLATE_NOOP("talcs::AudioDriver", "AAudio audio driver (SDL)")        },
                {"openslES",    QT_TRANSLATE_NOOP("talcs::AudioDriver", "opensl ES audio driver (SDL)")     },
                {"android",     QT_TRANSLATE_NOOP("talcs::AudioDriver", "SDL Android audio driver (SDL)")   },
                {"ps2",         QT_TRANSLATE_NOOP("talcs::AudioDriver", "PS2 audio driver (SDL)")           },
                {"psp",         QT_TRANSLATE_NOOP("talcs::AudioDriver", "PSP audio driver (SDL)")           },
                {"vita",        QT_TRANSLATE_NOOP("talcs::AudioDriver", "VITA audio driver (SDL)")          },
                {"n3ds",        QT_TRANSLATE_NOOP("talcs::AudioDriver", "SDL N3DS audio driver (SDL)")      },
                {"emscripten",  QT_TRANSLATE_NOOP("talcs::AudioDriver", "SDL emscripten audio driver (SDL)")},
                {"DART",        QT_TRANSLATE_NOOP("talcs::AudioDriver", "OS/2 DART (SDL)")                  },
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
        qCInfo(lcSDLAudioDriver) << "SDL audio driver initialization started." << "driver:" << name() << "SDL driver:" << d->internalName;
        if (SDL_Init(SDL_INIT_AUDIO) == 0 && SDL_AudioInit(d->internalName.toLocal8Bit()) == 0) {
            QTimer::singleShot(0, [=] {
                SDL_FlushEvents(SDL_AUDIODEVICEADDED, SDL_AUDIODEVICEREMOVED);
                d->eventPollerThread->start();
            });
            qCInfo(lcSDLAudioDriver) << "SDL audio driver initialization completed." << "driver:" << name();
            return AudioDriver::initialize();
        } else {
            setErrorString(SDL_GetError());
            qCWarning(lcSDLAudioDriver) << "SDL audio driver initialization failed." << "driver:" << name() << "SDL driver:" << d->internalName << "error:" << errorString();
            return false;
        }
    }
    void SDLAudioDriver::finalize() {
        Q_D(SDLAudioDriver);
        if (isInitialized()) {
            qCInfo(lcSDLAudioDriver) << "SDL audio driver finalization started." << "driver:" << name();
        }
        d->eventPoller->quit();
        d->eventPollerThread->quit();
        d->eventPollerThread->wait();
        SDL_AudioQuit();
        SDL_Quit();
        AudioDriver::finalize();
    }
    QStringList SDLAudioDriver::devices() const {
        QStringList list;
        int cnt = SDL_GetNumAudioDevices(0);
        if (cnt < 0) {
            qCWarning(lcSDLAudioDriver) << "SDL audio device enumeration failed." << "driver:" << name() << "error:" << SDL_GetError();
        }
        for (int i = 0; i < cnt; i++) {
            const auto *deviceName = SDL_GetAudioDeviceName(i, 0);
            if (!deviceName) {
                qCWarning(lcSDLAudioDriver) << "SDL audio device name retrieval failed." << "driver:" << name() << "index:" << i << "error:" << SDL_GetError();
            }
            list.append(QString::fromUtf8(deviceName));
        }
        qCDebug(lcSDLAudioDriver) << "SDL audio devices enumerated." << "driver:" << name() << "devices:" << list;
        return list;
    }
    QString SDLAudioDriver::defaultDevice() const {
        char *name;
        SDL_AudioSpec spec;
        if (SDL_GetDefaultAudioInfo(&name, &spec, 0) == 0) {
            auto s = QString::fromUtf8(name);
            SDL_free(name);
            qCDebug(lcSDLAudioDriver) << "SDL default audio device resolved." << "driver:" << this->name() << "device:" << s;
            return s;
        } else {
            qCWarning(lcSDLAudioDriver) << "SDL default audio device retrieval failed." << "driver:" << this->name() << "error:" << SDL_GetError();
            return {};
        }
    }

    AudioDevice *SDLAudioDriver::createDevice(const QString &name) {
        qCInfo(lcSDLAudioDriver) << "SDL audio device creation started." << "driver:" << this->name() << "device:" << name;
        if (name.isEmpty() || !devices().contains(name)) {
            qCWarning(lcSDLAudioDriver) << "SDL audio device creation failed because the device is unavailable." << "driver:" << this->name() << "device:" << name;
            return nullptr;
        }
        return new SDLAudioDevice(name, this);
    }

    void SDLAudioDriver::addOpenedDevice(quint32 devId, SDLAudioDevice * dev) {
        Q_D(SDLAudioDriver);
        d->openedDevices.insert(devId, dev);
    }

    void SDLAudioDriver::removeOpenedDevice(quint32 devId) {
        Q_D(SDLAudioDriver);
        d->openedDevices.remove(devId);
    }
    QList<SDLAudioDriver *> SDLAudioDriver::getDrivers() {
        int cnt = SDL_GetNumAudioDrivers();
        qCInfo(lcSDLAudioDriver) << "SDL audio driver enumeration started." << "driver count:" << cnt;
        QList<SDLAudioDriver *> list;
        for (int i = 0; i < cnt; i++) {
            auto name = QString::fromLatin1(SDL_GetAudioDriver(i));
            if (name == "disk" || name == "dummy") {
                qCDebug(lcSDLAudioDriver) << "SDL nonhardware audio driver skipped." << "SDL driver:" << name;
                continue;
            }
            auto drv = new SDLAudioDriver;
            drv->d_func()->internalName = name;
            drv->setName(getDisplayName(name));
            list.append(drv);
            qCInfo(lcSDLAudioDriver) << "SDL audio driver enumerated." << "SDL driver:" << name << "driver:" << drv->name();
        }
        return list;
    }

    void SDLAudioDriverPrivate::handleSDLEvent(const QByteArray &sdlEventData) {
        Q_Q(SDLAudioDriver);
        auto *e = reinterpret_cast<const SDL_Event *>(sdlEventData.data());
        if (e->type == SDL_AUDIODEVICEADDED && e->adevice.iscapture == 0) {
            qCInfo(lcSDLAudioDriver) << "SDL audio device added." << "driver:" << q->name() << "device index:" << e->adevice.which
                                     << "device:" << SDL_GetAudioDeviceName(static_cast<int>(e->adevice.which), 0);
            emit q->deviceChanged();
        } else if (e->type == SDL_AUDIODEVICEREMOVED && e->adevice.iscapture == 0) {
            qCInfo(lcSDLAudioDriver) << "SDL audio device removed." << "driver:" << q->name() << "device ID:" << e->adevice.which;
            handleDeviceRemoved(e->adevice.which);
            emit q->deviceChanged();
        }
    }
    void SDLAudioDriverPrivate::handleDeviceRemoved(quint32 devId) {
        auto it = openedDevices.find(devId);
        if (it == openedDevices.end())
            return;
        auto dev = it.value();
        qCWarning(lcSDLAudioDriver) << "An open SDL audio device was removed." << "device:" << dev->name() << "device ID:" << devId;
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
