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

#include <TalcsFormat/AudioResampler.h>
#include <TalcsCore/SineWaveAudioSource.h>
#include <TalcsCore/AudioDataWrapper.h>
#include <TalcsCore/AudioBuffer.h>
#include <TalcsCore/Decibels.h>

using namespace talcs;

class TestResampler1 : public AudioResampler {
public:
    TestResampler1() : AudioResampler(0.8, 1024), src(440) {
        src.open(1300, 48000);
    }
    void read(float *inputBlock, qint64 length) override {
        AudioDataWrapper buf(&inputBlock, 1, length);
        src.read(&buf);
    }

private:
    SineWaveAudioSource src;
};

class TestAudioResampler : public QObject {
    Q_OBJECT
private slots:
    void normalProcess() {
         TestResampler1 resampler;

        AudioBuffer buf(1, 32768);
        for (int i = 0; i < 32768; i += 1024) {
            resampler.process(buf.data(0) + i, 1024);
        }

        SineWaveAudioSource cmpSrc(440);
        cmpSrc.open(32768, 38400);
        AudioBuffer cmpBuf(1, 32768);
        cmpSrc.read(&cmpBuf);
        buf.addSampleRange(0, 0, 32768, cmpBuf, 0, 0, -1);

        QVERIFY(Decibels::gainToDecibels(buf.magnitude(0)) < -24);
        QVERIFY(Decibels::gainToDecibels(buf.rms(0)) < -72);
    }

    void variableReadLength() {
        static const quint32 MAGIC_SEED = 114514;
        QRandomGenerator g(MAGIC_SEED);

        TestResampler1 resampler;

        AudioBuffer buf(1, 32768 + 1024);
        for (int i = 0; i < 32768 + 1024;) {
            int len = g.bounded(1, 1025);
            resampler.process(buf.data(0) + i, len);
            i += len;
        }
        buf.resize(-1, 32768);

        SineWaveAudioSource cmpSrc(440);
        cmpSrc.open(32768, 38400);
        AudioBuffer cmpBuf(1, 32768);
        cmpSrc.read(&cmpBuf);
        buf.addSampleRange(0, 0, 32768, cmpBuf, 0, 0, -1);

        QVERIFY(Decibels::gainToDecibels(buf.magnitude(0)) < -24);
        QVERIFY(Decibels::gainToDecibels(buf.rms(0)) < -72);
    }

    void zeroReadLength() {
        static const quint32 MAGIC_SEED = 114514;
        QRandomGenerator g(MAGIC_SEED);

        TestResampler1 resampler;

        AudioBuffer buf(1, 32768 + 1024);
        for (int t = 0; t < 114514; t++) {
            resampler.process(nullptr, 0);
        }
        for (int i = 0; i < 32768 + 1024;) {
            int len = g.bounded(1, 1025);
            len = g.bounded(0, 2) ? len : 0;
            resampler.process(buf.data(0) + i, len);
            i += len;
        }

        buf.resize(-1, 32768);

        SineWaveAudioSource cmpSrc(440);
        cmpSrc.open(32768, 38400);
        AudioBuffer cmpBuf(1, 32768);
        cmpSrc.read(&cmpBuf);
        buf.addSampleRange(0, 0, 32768, cmpBuf, 0, 0, -1);

        QVERIFY(Decibels::gainToDecibels(buf.magnitude(0)) < -24);
        QVERIFY(Decibels::gainToDecibels(buf.rms(0)) < -72);

    }

};

QTEST_MAIN(TestAudioResampler)

#include "test.moc"
