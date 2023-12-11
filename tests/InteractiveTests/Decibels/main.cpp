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

#include <cmath>

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>
#include <QPushButton>
#include <QTimer>

#include <TalcsDevice/AudioDevice.h>
#include <TalcsDevice/AudioDriver.h>
#include <TalcsDevice/AudioDriverManager.h>
#include <TalcsDevice/AudioSourcePlayback.h>
#include <TalcsFormat/AudioFormatIO.h>
#include <TalcsFormat/AudioFormatInputSource.h>
#include <TalcsCore/MixerAudioSource.h>
#include <TalcsCore/PositionableMixerAudioSource.h>
#include <TalcsCore/TransportAudioSource.h>
#include <TalcsCore/Decibels.h>
#include <TalcsCore/SmoothedFloat.h>

using namespace talcs;

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    QFile f(QFileDialog::getOpenFileName());
    AudioFormatIO audioFormatIo(&f);
    AudioFormatInputSource src(&audioFormatIo);
    TransportAudioSource tpSrc(&src);
    MixerAudioSource mixer;
    mixer.addSource(&tpSrc);

    auto drvMgr = AudioDriverManager::createBuiltInDriverManager();
    auto drv = drvMgr->driver(drvMgr->drivers()[0]);
    drv->initialize();
    auto dev = drv->createDevice(drv->devices()[0]);
    dev->open(dev->preferredBufferSize(), dev->preferredSampleRate());
    AudioSourcePlayback playback(&mixer);
    dev->start(&playback);

    auto win = new QMainWindow;
    auto mainWidget = new QWidget;
    auto mainLayout = new QVBoxLayout;

    auto layoutL = new QHBoxLayout;
    auto meterL = new QProgressBar;
    meterL->setTextVisible(false);
    meterL->setRange(0, 300);
    auto labelL = new QLabel;
    layoutL->addWidget(meterL);
    layoutL->addWidget(labelL);

    auto layoutR = new QHBoxLayout;
    auto meterR = new QProgressBar;
    meterR->setTextVisible(false);
    meterR->setRange(0, 300);
    auto labelR = new QLabel;
    layoutR->addWidget(meterR);
    layoutR->addWidget(labelR);

    auto transportLayout = new QHBoxLayout;
    auto playStopButton = new QPushButton("Play");
    auto resetButton = new QPushButton("Reset");
    transportLayout->addWidget(playStopButton);
    transportLayout->addWidget(resetButton);

    mainLayout->addLayout(layoutL);
    mainLayout->addLayout(layoutR);
    mainLayout->addLayout(transportLayout);
    mainWidget->setLayout(mainLayout);
    win->setCentralWidget(mainWidget);

    win->show();

    QTimer timer;
    timer.setInterval(50);
    SmoothedFloat valueL;
    valueL.setRampLength(5);
    SmoothedFloat valueR;
    valueR.setRampLength(5);

    QObject::connect(playStopButton, &QPushButton::clicked, [&]() {
        if (!tpSrc.isPlaying()) {
            tpSrc.play();
            playStopButton->setText("Stop");
        } else {
            tpSrc.pause();
            playStopButton->setText("Play");
        }
    });

    QObject::connect(resetButton, &QPushButton::clicked, [&]() {
        tpSrc.setPosition(0);
    });

    mixer.setMeterEnabled(true);

    QObject::connect(&mixer, &MixerAudioSource::meterUpdated, win, [&](float ml, float mr) {
        float dBL = Decibels::gainToDecibels(ml, -30);
        if (dBL < valueL.currentValue())
            valueL.setTargetValue(dBL);
        else
            valueL.setCurrentAndTargetValue(dBL);

        float dBR = Decibels::gainToDecibels(mr, -30);
        if (dBR < valueR.currentValue())
            valueR.setTargetValue(dBR);
        else
            valueR.setCurrentAndTargetValue(dBR);
    });

    QObject::connect(&timer, &QTimer::timeout, win, [&]() {
        meterL->setValue(qRound(300 + 10 * valueL.nextValue()));
        meterR->setValue(qRound(300 + 10 * valueR.nextValue()));
    });
    timer.start();

    return a.exec();
}
