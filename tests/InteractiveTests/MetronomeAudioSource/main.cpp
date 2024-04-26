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

class Detector : public MetronomeAudioSourceDetector {
public:
    void detectInterval(qint64 intervalLength) override {
        currentPos = pos / 24000 * 24000;
        if (currentPos < pos)
            currentPos += 24000;
        end = pos + intervalLength;
        if (currentPos >= end)
            currentPos = -1;
        posDelta = intervalLength;
    }

    MetronomeAudioSourceDetectorMessage nextMessage() override {
        if (currentPos == -1) {
            pos += posDelta;
            posDelta = 0;
            return {-1, false};
        }
        MetronomeAudioSourceDetectorMessage ret = {currentPos - pos, false};
        if (currentPos % 96000 == 0)
            ret.isMajor = true;
        currentPos += 24000;
        if (currentPos >= end)
            currentPos = -1;
        return ret;
    }

private:
    qint64 pos = 0;
    qint64 posDelta = 0;
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
