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

#include <QCoreApplication>
#include <QDebug>
#include <TalcsDevice/AudioDriverManager.h>
#include <TalcsDevice/AudioDriver.h>
#include <TalcsDevice/AudioDevice.h>
#include <TalcsDevice/AudioSourcePlayback.h>
#include <TalcsCore/SineWaveAudioSource.h>

int main(int argc, char **argv) {
    QCoreApplication a(argc, argv);

    auto mgr = talcs::AudioDriverManager::createBuiltInDriverManager();
    qInfo() << "Built-in drivers:" << mgr->drivers();
    auto wasapiDrv = mgr->driver("wasapi");

    if (!wasapiDrv->initialize()) {
        qCritical() << "Cannot initialize WASAPI driver!";
        return 1;
    }
    qInfo() << "WASAPI devices:" << wasapiDrv->devices();
    qInfo() << "WASAPI default device:" << wasapiDrv->defaultDevice();
    auto dev = wasapiDrv->createDevice(wasapiDrv->defaultDevice());

    if (!dev || !dev->isInitialized()) {
        qCritical() << "Cannot create WASAPI audio device!";
        return 1;
    }
    qInfo() << "Preferred buffer size:" << dev->preferredBufferSize();
    qInfo() << "Preferred sample rate:" << dev->preferredSampleRate();
    if (!dev->open(dev->preferredBufferSize(), dev->preferredSampleRate())) {
        qCritical() << "Cannot open WASAPI audio device!";
        return 1;
    }
    auto src = new talcs::SineWaveAudioSource(440);
    auto srcPlayback = new talcs::AudioSourcePlayback(src);
    if (!dev->start(srcPlayback)) {
        qCritical() << "Cannot start playing with WASAPI audio device!";
        return 1;
    }

    a.exec();
}
