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

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFileDialog>

#include <TalcsDevice/AudioDriverManager.h>
#include <TalcsDevice/AudioDriver.h>
#include <TalcsDevice/AudioDevice.h>
#include <TalcsDevice/AudioSourcePlayback.h>
#include <TalcsFormat/AudioFormatIO.h>
#include <TalcsFormat/AudioFormatInputSource.h>

using namespace talcs;

class Filter : public AudioSource {
protected:
    qint64 processReading(const AudioSourceReadData &readData) override {
        for (int i = 0; i < readData.length; i++) {
            for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
                float v = readData.buffer->sample(ch, readData.startPos + i);
                v += 1.0f;
                if (v > 1.0f)
                    v -= 2.0f;
                readData.buffer->setSample(ch, readData.startPos + i, v);
            }
        }
        return readData.length;
    }
};

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    QFile f(QFileDialog::getOpenFileName());
    AudioFormatIO io(&f);
    f.open(QIODevice::ReadOnly);
    AudioFormatInputSource src(&io);
    src.setReadingFilter(new Filter);
    auto mgr = AudioDriverManager::createBuiltInDriverManager();
    auto drv = mgr->driver(mgr->drivers()[0]);
    drv->initialize();
    auto dev = drv->createDevice(drv->defaultDevice());
    qDebug() << dev->name();
    dev->open(1024, 48000);
    AudioSourcePlayback playback(&src);
    dev->start(&playback);

    return a.exec();
}
