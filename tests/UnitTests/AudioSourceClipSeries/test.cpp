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

#include <QtTest/QtTest>

#include <TalcsCore/AudioSourceClipSeries.h>
#include <TalcsCore/AudioBuffer.h>
#include <TalcsCore/MemoryAudioSource.h>

using namespace talcs;

class TestAudioSourceClipSeries : public QObject {
    Q_OBJECT
private slots:
    void initializeObject() {
        AudioSourceClipSeries series;
        series.close();
        series.open(1024, 48000);
        QVERIFY(series.isOpen());
        QCOMPARE(series.bufferSize(), 1024);
        QCOMPARE(series.sampleRate(), 48000);
        series.open(4096, 44100);
        QCOMPARE(series.bufferSize(), 4096);
        QCOMPARE(series.sampleRate(), 44100);
    }

    void clipOperations() {
        // TODO
    }

    void clipReading() {
        AudioBuffer buf1(1, 1024);
        MemoryAudioSource src1(&buf1);
        std::iota(buf1.data(0), buf1.data(0) + 1024, 0);
        AudioBuffer buf2(1, 1024);
        MemoryAudioSource src2(&buf2);
        std::iota(buf2.data(0), buf2.data(0) + 1024, 4096);
        AudioSourceClipSeries series;
        QVERIFY(series.insertClip(&src1, 0, 0, 1024).isValid());
        AudioBuffer tmpBuf(1, 768);
        series.open(768, 48000);
        QVERIFY(src1.isOpen());
        QVERIFY(series.insertClip(&src2, 1280, 0, 1024).isValid());
        QVERIFY(src2.isOpen());
        series.read(&tmpBuf);
        for (int i = 0; i < 768; i++) {
            QCOMPARE(tmpBuf.constSampleAt(0, i), i);
        }
        for (int t = 0; t < 2; t++) {
            series.read(&tmpBuf);
            for (int i = 0; i < 256; i++) {
                QCOMPARE(tmpBuf.constSampleAt(0, i), 768 + i);
                QCOMPARE(tmpBuf.constSampleAt(0, 256 + i), 0);
                QCOMPARE(tmpBuf.constSampleAt(0, 512 + i), 4096 + i);
            }
            series.read(&tmpBuf);
            for (int i = 0; i < 768; i++) {
                QCOMPARE(tmpBuf.constSampleAt(0, i), 4096 + 256 + i);
            }
            series.setNextReadPosition(768);
        }
    }

};

QTEST_MAIN(TestAudioSourceClipSeries)

#include "test.moc"