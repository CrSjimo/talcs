/******************************************************************************
 * Copyright (c) 2024 CrSjimo                                                 *
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

#include <TalcsCore/MetronomeAudioSource.h>
#include <TalcsDevice/AudioDriverManager.h>
#include <TalcsDevice/AudioDriver.h>
#include <TalcsDevice/AudioDevice.h>
#include <TalcsDevice/AudioSourcePlayback.h>

using namespace talcs;

class Detector : public MetronomeAudioSourceBeatDetector {
public:
    void initialize() override {
        currentPos = -1;
        end = 0;
    }

    void detectInterval(qint64 intervalStart, qint64 intervalLength) override {
        currentPos = intervalStart / 24000 * 24000;
        if (currentPos < intervalStart)
            currentPos += 24000;
        end = intervalStart + intervalLength;
        if (currentPos >= end)
            currentPos = -1;
    }

    QPair<qint64, bool> nextBeat() override {
        if (currentPos == -1)
            return {-1, false};
        QPair<qint64, bool> ret = {currentPos, false};
        if (currentPos % 96000 == 0)
            ret.second = true;
        currentPos += 24000;
        if (currentPos >= end)
            currentPos = -1;
        return ret;
    }

private:
    qint64 currentPos = -1;
    qint64 end = 0;

};

int main(int argc, char **argv) {
    QCoreApplication a(argc, argv);

    auto drvMgr = AudioDriverManager::createBuiltInDriverManager();
    auto drv = drvMgr->driver(drvMgr->drivers()[0]);
    drv->initialize();
    auto dev = drv->defaultDevice().isEmpty() ? drv->createDevice(drv->devices()[0]) : drv->createDevice(drv->defaultDevice());
    qDebug() << dev->name();
    dev->open(dev->preferredBufferSize(), 48000);

    Detector detector;
    MetronomeAudioSource src;
    src.setMajorBeatSource(MetronomeAudioSource::builtInMajorBeatSource(), true);
    src.setMinorBeatSource(MetronomeAudioSource::builtInMinorBeatSource(), true);
    src.setDetector(&detector);
    AudioSourcePlayback playback(&src);
    dev->start(&playback);

    a.exec();

}
