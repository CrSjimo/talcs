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
#include <TalcsCore/MemoryAudioSource.h>
#include <QPointer>
#include <QSignalSpy>

using namespace talcs;

class DummyAudioSource: public QObject, public SineWaveAudioSource {
    Q_OBJECT
public:
    DummyAudioSource() : QObject(nullptr), SineWaveAudioSource(440) {
    }

    ~DummyAudioSource() override = default;
};

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
        mixer.removeAllSources();
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
        mixer.removeAllSources();
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
        mixer.removeAllSources();
    }

    void readingRange_MixerAudioSource() {
        MixerAudioSource mixer;
        AudioBuffer buf[3] = {AudioBuffer(2, 4096), AudioBuffer(2, 2048), AudioBuffer(2,4096)};
        MemoryAudioSource src[3] = {MemoryAudioSource(buf), MemoryAudioSource(buf + 1), MemoryAudioSource(buf + 2)};
        for (int i = 0; i < 3; i++)
            mixer.addSource(src + i);
        AudioBuffer tmpBuf(2, 2048);
        mixer.open(1536, 48000);

        QCOMPARE(mixer.read({&tmpBuf, 0, 1536}), 1536);
        QCOMPARE(mixer.read({&tmpBuf, 0, 1024}), 1024);
        QCOMPARE(src[0].nextReadPosition(), 2560);
        QCOMPARE(src[1].nextReadPosition(), 2048);
        QCOMPARE(src[2].nextReadPosition(), 2560);

        mixer.removeSource(src);
        mixer.removeSource(src + 2);
        src[1].setNextReadPosition(1024);
        QCOMPARE(mixer.read({&tmpBuf, 0, 1536}), 1024);
        mixer.removeAllSources();
    }

    void readingRange_PositionableMixerAudioSource() {
        PositionableMixerAudioSource mixer;
        AudioBuffer buf[3] = {AudioBuffer(2, 4096), AudioBuffer(2, 2048), AudioBuffer(2,4096)};
        MemoryAudioSource src[3] = {MemoryAudioSource(buf), MemoryAudioSource(buf + 1), MemoryAudioSource(buf + 2)};
        for (int i = 0; i < 3; i++)
            mixer.addSource(src + i);
        QCOMPARE(mixer.length(), 2048);
        AudioBuffer tmpBuf(2, 2048);
        mixer.open(1536, 48000);

        QCOMPARE(mixer.read({&tmpBuf, 0, 1536}), 1536);
        QCOMPARE(mixer.read({&tmpBuf, 0, 1024}), 512);
        QCOMPARE(src[0].nextReadPosition(), 2048);
        QCOMPARE(src[1].nextReadPosition(), 2048);
        QCOMPARE(src[2].nextReadPosition(), 2048);
        mixer.removeAllSources();
    }

    void mixing() {
        PositionableMixerAudioSource mixer;
        AudioBuffer buf[3] = {AudioBuffer(3, 1024), AudioBuffer(3, 1024), AudioBuffer(3,1024)};
        MemoryAudioSource src[3] = {MemoryAudioSource(buf), MemoryAudioSource(buf + 1), MemoryAudioSource(buf + 2)};
        for (int i = 0; i < 3; i++) {
            buf[i].data(0)[0] = 00.0f + i;
            buf[i].data(1)[0] = 10.0f + i;
            buf[i].data(2)[0] = 20.0f + i;
            mixer.addSource(src + i);
        }
        mixer.open(1024, 48000);
        AudioBuffer tmpBuf(4, 1024);
        tmpBuf.data(3)[0] = 114514.0f;
        mixer.read(&tmpBuf);
        for (int i = 0; i < 3; i++) {
            QCOMPARE(tmpBuf.data(i)[0], 30 * i + 3);
        }
        QCOMPARE(tmpBuf.data(3)[0], 0);
        mixer.removeAllSources();
    }

    void channelRouting() {
        PositionableMixerAudioSource mixer;
        AudioBuffer buf[3] = {AudioBuffer(3, 1024), AudioBuffer(3, 1024), AudioBuffer(3,1024)};
        MemoryAudioSource src[3] = {MemoryAudioSource(buf), MemoryAudioSource(buf + 1), MemoryAudioSource(buf + 2)};
        for (int i = 0; i < 3; i++) {
            buf[i].data(0)[0] = 00.0f + i;
            buf[i].data(1)[0] = 10.0f + i;
            buf[i].data(2)[0] = 20.0f + i;
            mixer.addSource(src + i);
        }
        mixer.setRouteChannels(true);
        mixer.open(1024, 48000);
        AudioBuffer tmpBuf(8, 1024);
        tmpBuf.data(6)[0] = 114514.0f;
        tmpBuf.data(7)[0] = 1919810.0f;
        mixer.read(&tmpBuf);
        for (int i = 0; i < 3; i++) {
            QCOMPARE(tmpBuf.data(i * 2)[0], i);
            QCOMPARE(tmpBuf.data(i * 2 + 1)[0], 10 + i);
        }
        QCOMPARE(tmpBuf.data(6)[0], 0);
        QCOMPARE(tmpBuf.data(7)[0], 0);
        mixer.removeSource(src + 1);
        mixer.setNextReadPosition(0);
        mixer.read(&tmpBuf);
        QCOMPARE(tmpBuf.data(2)[0], 2);
        mixer.removeAllSources();
    }

    void addingAndRemovingSources() {
        QScopedPointer<PositionableMixerAudioSource> mixer(new PositionableMixerAudioSource);
        SineWaveAudioSource src1(440);
        mixer->addSource(&src1);
        mixer->open(1024, 48000);
        QVERIFY(src1.isOpen());
        QPointer<DummyAudioSource> src2 = new DummyAudioSource;
        mixer->addSource(src2.data(), true);
        QVERIFY(src2->isOpen());
        QPointer<DummyAudioSource> src3 = new DummyAudioSource;
        mixer->addSource(src3.data(), true);
        SineWaveAudioSource src4(440);
        mixer->addSource(&src4);
        QList<PositionableAudioSource *> expectedSrcList = {&src1, src2.data(), src3.data(), &src4};
        QCOMPARE(mixer->sources(), expectedSrcList);
        mixer->removeSource(src3.data());
        expectedSrcList.removeOne(src3.data());
        QCOMPARE(mixer->sources(), expectedSrcList);
        mixer.reset();
        QVERIFY(src2.isNull());
        QVERIFY(!src3.isNull());
        src3.clear();
    }

    void sourceIterating() {
        PositionableMixerAudioSource mixer;
        SineWaveAudioSource src[] = {SineWaveAudioSource(440), SineWaveAudioSource(440), SineWaveAudioSource(440), SineWaveAudioSource(440)};
        QList<PositionableMixerAudioSource::SourceIterator> itList;
        for (int i = 0; i < 4; i++)
            itList.append(mixer.appendSource(src + i));
        for (int i = 0; i < 4; i++)
            QCOMPARE(itList[i].data(), src + i);
        QCOMPARE(mixer.firstSource(), itList[0]);
        QCOMPARE(mixer.lastSource(), itList[3]);
        QCOMPARE(mixer.findSource(src + 2), itList[2]);

        mixer.moveSource(itList[1], itList[3]);
        QList<PositionableAudioSource *> expectedSrcList = {src, src + 3, src + 1, src + 2};
        QCOMPARE(mixer.sources(), expectedSrcList);

        mixer.swapSource(itList[2], itList[3]);
        expectedSrcList = {src, src + 2, src + 1, src + 3};
        QCOMPARE(mixer.sources(), expectedSrcList);

        QCOMPARE(mixer.findSource(nullptr).data(), nullptr);
        QCOMPARE(mixer.findSource(src + 1), itList[1]);
        mixer.removeAllSources();
    }

    void soloAndMuteTest() {
        PositionableMixerAudioSource mixer;
        AudioBuffer buf[3] = {AudioBuffer(2, 1024), AudioBuffer(2, 1024), AudioBuffer(2,1024)};
        MemoryAudioSource src[3] = {MemoryAudioSource(buf), MemoryAudioSource(buf + 1), MemoryAudioSource(buf + 2)};
        for (int i = 0; i < 3; i++) {
            buf[i].data(0)[0] = buf[i].data(1)[0] = 10 + i;
            mixer.addSource(src + i);
        }
        mixer.open(1024, 48000);
        AudioBuffer tmpBuf(6, 1024);
        mixer.setSourceSolo(src + 1, true);
        mixer.read(&tmpBuf);
        QCOMPARE(tmpBuf.data(0)[0], 11);
        mixer.setNextReadPosition(0);

        mixer.setSourceSolo(src, true);
        mixer.read(&tmpBuf);
        QCOMPARE(tmpBuf.data(0)[0], 21);
        mixer.setNextReadPosition(0);

        mixer.setRouteChannels(true);
        mixer.read(&tmpBuf);
        QCOMPARE(tmpBuf.data(4)[0], 0);
        mixer.setNextReadPosition(0);

        mixer.setSilentFlags(2);
        mixer.read(&tmpBuf);
        QCOMPARE(tmpBuf.data(0)[0], 10);
        QCOMPARE(tmpBuf.data(1)[0], 0);
        mixer.setNextReadPosition(0);

        mixer.setRouteChannels(false);
        mixer.read(&tmpBuf);
        QCOMPARE(tmpBuf.data(0)[0], 21);
        QCOMPARE(tmpBuf.data(1)[0], 0);
        mixer.removeAllSources();
    }

    void gainAndPanTest() {
        PositionableMixerAudioSource mixer;
        AudioBuffer buf[3] = {AudioBuffer(2, 1024), AudioBuffer(2, 1024), AudioBuffer(2,1024)};
        MemoryAudioSource src[3] = {MemoryAudioSource(buf), MemoryAudioSource(buf + 1), MemoryAudioSource(buf + 2)};
        for (int i = 0; i < 3; i++) {
            buf[i].data(0)[0] = buf[i].data(1)[0] = 10 + i;
            mixer.addSource(src + i);
        }
        mixer.setGain(2);
        mixer.open(1024, 48000);
        AudioBuffer tmpBuf(2, 1024);
        mixer.read(&tmpBuf);
        QCOMPARE(tmpBuf.data(0)[0], 66);
        QCOMPARE(tmpBuf.data(1)[0], 66);
        mixer.setNextReadPosition(0);

        mixer.setPan(-0.5);
        mixer.read(&tmpBuf);
        QCOMPARE(tmpBuf.data(0)[0], 66);
        QCOMPARE(tmpBuf.data(1)[0], 33);
        mixer.setNextReadPosition(0);

        mixer.setPan(0.5);
        mixer.read(&tmpBuf);
        QCOMPARE(tmpBuf.data(0)[0], 33);
        QCOMPARE(tmpBuf.data(1)[0], 66);
        mixer.removeAllSources();
    }

    void metering() {
        PositionableMixerAudioSource mixer;
        AudioBuffer buf[3] = {AudioBuffer(2, 1024), AudioBuffer(2, 1024), AudioBuffer(2,1024)};
        MemoryAudioSource src[3] = {MemoryAudioSource(buf), MemoryAudioSource(buf + 1), MemoryAudioSource(buf + 2)};
        for (int i = 0; i < 3; i++) {
            buf[i].data(0)[0] = i;
            buf[i].data(1)[0] = 10 + i;
            mixer.addSource(src + i);
        }
        QSignalSpy spy(&mixer, &PositionableMixerAudioSource::levelMetered);
        mixer.open(1024, 48000);
        AudioBuffer tmpBuf(2, 1024);
        mixer.setLevelMeterChannelCount(3);
        mixer.read(&tmpBuf);
        auto vec = spy[0][0].value<QVector<float>>();
        QCOMPARE(vec[0], 3);
        QCOMPARE(vec[1], 33);
        QCOMPARE(vec[2], 0);
        mixer.removeAllSources();
    }

};

QTEST_MAIN(TestIMixer)

#include "test.moc"
