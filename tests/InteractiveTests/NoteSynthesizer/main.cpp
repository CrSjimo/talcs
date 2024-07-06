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

#include <cmath>

#include <QCoreApplication>
#include <QDebug>

#include <TalcsCore/NoteSynthesizer.h>
#include <TalcsDevice/AudioDriverManager.h>
#include <TalcsDevice/AudioDriver.h>
#include <TalcsDevice/AudioDevice.h>
#include <TalcsDevice/AudioSourcePlayback.h>

using namespace talcs;

static inline double calcFreq(int key) {
    return 440.0 * std::pow(2.0, (key - 69) / 12.0);
}

class Detector : public NoteSynthesizerDetector {
public:
    void detectInterval(qint64 intervalLength) override {
        currentPos = pos / 12000 * 12000;
        if (currentPos < pos)
            currentPos += 12000;
        end = pos + intervalLength;
        if (currentPos >= end)
            currentPos = -1;
        posDelta = intervalLength;
    }

    NoteSynthesizerDetectorMessage nextMessage() override {
        if (!retQueue.isEmpty()) {
            return retQueue.takeFirst();
        }
        if (currentPos == -1) {
            pos += posDelta;
            posDelta = 0;
            return {-1, false};
        }
        NoteSynthesizerDetectorMessage ret = {currentPos - pos, 0, 0.5, true};
        switch (currentPos % (12000 * 8)) {
            case 12000 * 0:
                ret.frequency = calcFreq(60);
                ret.isNoteOn = true;
                break;
            case 12000 * 1:
                ret.frequency = calcFreq(60);
                ret.isNoteOn = false;
                retQueue.append({currentPos - pos, calcFreq(60), 0.5, true});
                break;
            case 12000 * 2:
                ret.frequency = calcFreq(60);
                ret.isNoteOn = false;
                retQueue.append({currentPos - pos, calcFreq(67), 0.5, true});
                break;
            case 12000 * 3:
                ret.frequency = calcFreq(67);
                ret.isNoteOn = false;
                retQueue.append({currentPos - pos, calcFreq(67), 0.5, true});
                break;
            case 12000 * 4:
                ret.frequency = calcFreq(67);
                ret.isNoteOn = false;
                retQueue.append({currentPos - pos, calcFreq(69), 0.5, true});
                break;
            case 12000 * 5:
                ret.frequency = calcFreq(69);
                ret.isNoteOn = false;
                retQueue.append({currentPos - pos, calcFreq(69), 0.5, true});
                break;
            case 12000 * 6:
                ret.frequency = calcFreq(69);
                ret.isNoteOn = false;
                retQueue.append({currentPos - pos, calcFreq(67), 0.5, true});
                break;
            case 12000 * 7:
                ret.frequency = calcFreq(67);
                ret.isNoteOn = false;
                break;
            default:
                Q_UNREACHABLE();
        }
        currentPos += 12000;
        if (currentPos >= end)
            currentPos = -1;
        return ret;
    }

    QList<NoteSynthesizerDetectorMessage> retQueue;

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
    NoteSynthesizer src;
    src.setAttackRate(std::pow(0.99, 20000.0 / 48000.0));
    src.setReleaseRate(std::pow(0.99, 20000.0 / 48000.0));
    src.setGenerator(NoteSynthesizer::Square);
    src.setDetector(&detector);
    AudioSourcePlayback playback(&src);
    dev->start(&playback);

    a.exec();

}