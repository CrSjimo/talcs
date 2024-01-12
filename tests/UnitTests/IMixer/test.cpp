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

#include <QtTest/QTest>

#include <TalcsCore/AudioBuffer.h>
#include <TalcsCore/MixerAudioSource.h>
#include <TalcsCore/PositionableMixerAudioSource.h>
#include <TalcsCore/SineWaveAudioSource.h>

using namespace talcs;

class TestIMixer: public QObject {
    Q_OBJECT
private slots:
    void openAndCloseMixer_MixerAudioSource() {
        MixerAudioSource mixer;
        mixer.close();
        QVERIFY(mixer.open(1024, 48000));
        QVERIFY(mixer.isOpen());
        QCOMPARE(mixer.bufferSize(), 1024);
        QCOMPARE(mixer.sampleRate(), 48000);
        QVERIFY(mixer.open(2048, 48000));
        QCOMPARE(mixer.bufferSize(), 2048);
        SineWaveAudioSource src(440);
        mixer.addSource(&src);
        QVERIFY(src.isOpen());
        AudioBuffer buf(2, 2048);
        mixer.read(&buf);
        mixer.open(4096, 44100);
        QCOMPARE(src.bufferSize(), 4096);
        QCOMPARE(src.sampleRate(), 44100);
        mixer.close();
        QVERIFY(!src.isOpen());
    }

    void openAndCloseMixer_PositionableMixerAudioSource() {
        PositionableMixerAudioSource mixer;
        mixer.close();
        QVERIFY(mixer.open(1024, 48000));
        QVERIFY(mixer.isOpen());
        QCOMPARE(mixer.bufferSize(), 1024);
        QCOMPARE(mixer.sampleRate(), 48000);
        QVERIFY(mixer.open(2048, 48000));
        QCOMPARE(mixer.bufferSize(), 2048);
        SineWaveAudioSource src(440);
        mixer.addSource(&src);
        QVERIFY(src.isOpen());
        AudioBuffer buf(2, 2048);
        mixer.read(&buf);
        mixer.open(4096, 44100);
        QCOMPARE(src.bufferSize(), 4096);
        QCOMPARE(src.sampleRate(), 44100);
        mixer.close();
        QVERIFY(!src.isOpen());
    }

    void syncPosition() {
        PositionableMixerAudioSource mixer;
        mixer.setNextReadPosition(114514);
        SineWaveAudioSource src(440);
        mixer.addSource(&src);
        QCOMPARE(src.nextReadPosition(), 114514);
        mixer.setNextReadPosition(1919810);
        QCOMPARE(src.nextReadPosition(), 1919810);
        src.open(1024, 44100);
        src.setNextReadPosition(19260817);
        QCOMPARE(src.nextReadPosition(), 19260817);
    }

    // TODO: test of reading functions

};

QTEST_MAIN(TestIMixer)

#include "test.moc"
