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

#include <QApplication>
#include <QDebug>

#include <TalcsMidi/MidiMessage.h>
#include <TalcsMidi/MidiInputDevice.h>
#include <TalcsMidi/MidiSineWaveSynthesizer.h>

#include <TalcsDevice/AudioDriverManager.h>
#include <TalcsDevice/AudioDriver.h>
#include <TalcsDevice/AudioDevice.h>
#include <TalcsDevice/AudioSourcePlayback.h>

using namespace talcs;


int main(int argc, char **argv) {
    QApplication a(argc, argv);
    auto msg = MidiMessage::noteOn(1, 60, quint8(100));
    qDebug() << msg.getDescription();
    msg = MidiMessage::midiMachineControlCommand(talcs::MidiMessage::mmc_pause);
    qDebug() << msg.getRawData() << msg.getRawDataSize();
    qDebug() << MidiInputDevice::devices();
    MidiInputDevice dev(1);

    auto mgr = AudioDriverManager::createBuiltInDriverManager();
    auto drv = mgr->driver(mgr->drivers()[0]);
    drv->initialize();
    auto audioDev = drv->defaultDevice().isEmpty() ? drv->createDevice(drv->devices()[0]) : drv->createDevice(drv->defaultDevice());

    auto midiSynth = new MidiSineWaveSynthesizer;
    auto playback = new AudioSourcePlayback(midiSynth);
    dev.addListener(midiSynth);

    audioDev->open(audioDev->preferredBufferSize(), audioDev->preferredSampleRate());
    audioDev->start(playback);
    dev.open();
    return a.exec();
}