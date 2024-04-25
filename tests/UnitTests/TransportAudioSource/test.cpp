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

#include <QtTest/QtTest>

#include <TalcsCore/TransportAudioSource.h>
#include <TalcsCore/SineWaveAudioSource.h>

using namespace talcs;

class DummyAudioSource : public QObject, public PositionableAudioSource {
    Q_OBJECT
public:
    bool open(qint64 bufferSize, double sampleRate) override {
        return AudioStreamBase::open(bufferSize, sampleRate);
    }

    void close() override {
        AudioStreamBase::close();
    }

    qint64 processReading(const AudioSourceReadData &readData) override {
        return readData.length;
    }

    ~DummyAudioSource() override {

    }

    qint64 length() const override {
        return 114514;
    }

    qint64 nextReadPosition() const override {
        return PositionableAudioSource::nextReadPosition();
    }

    void setNextReadPosition(qint64 pos) override {
        PositionableAudioSource::setNextReadPosition(pos);
    }
};

class TestTransportAudioSource: public QObject {
    Q_OBJECT
private slots:
    void initializeObject() {
        TransportAudioSource tpSrc;
        QCOMPARE(tpSrc.source(), nullptr);
        QCOMPARE(tpSrc.length(), 0);
        QCOMPARE(tpSrc.position(), 0);
    }

    void openAndCloseWithoutSource() {
        TransportAudioSource tpSrc;
        QVERIFY(tpSrc.open(0, 0));
        tpSrc.close();
    }

    void setSourceAndOpen() {
        TransportAudioSource tpSrc;
        auto src = new SineWaveAudioSource(440);
        tpSrc.setSource(src, true);
        QCOMPARE(tpSrc.source(), src);
        QVERIFY(tpSrc.open(1024, 48000));
        QVERIFY(tpSrc.open(512, 44100));
        QCOMPARE(tpSrc.bufferSize(), 512);
        QCOMPARE(tpSrc.sampleRate(), 44100);
    }

    void setPosition() {
        TransportAudioSource tpSrc;
        auto src = new SineWaveAudioSource(440);
        tpSrc.setSource(src, true);
        QSignalSpy spy(&tpSrc, &TransportAudioSource::positionAboutToChange);
        tpSrc.setPosition(114514);
        QCOMPARE(tpSrc.source()->nextReadPosition(), 114514);
        QCOMPARE(spy[0][0], 114514);
        spy.clear();
        tpSrc.setPosition(114514);
        QVERIFY(spy.isEmpty());
    }

    void resetSourceWhenOpen() {
        TransportAudioSource tpSrc;
        PositionableAudioSource *src = new SineWaveAudioSource(440);
        tpSrc.setSource(src);
        tpSrc.open(1024, 48000);
        tpSrc.setPosition(114514);
        delete src;
        src = new DummyAudioSource;
        tpSrc.setSource(src, true);
        QVERIFY(src->isOpen());
        QCOMPARE(src->nextReadPosition(), 114514);
    }

    void deleteOwnedSource() {
        QPointer p(new DummyAudioSource);
        auto tpSrc = new TransportAudioSource(p.data(), true);
        delete tpSrc;
        QVERIFY(p.isNull());
    }

    void closeOnDelete() {
        auto tpSrc = new TransportAudioSource;
        auto src = new SineWaveAudioSource(440);
        tpSrc->setSource(src);
        tpSrc->open(1024, 48000);
        delete tpSrc;
        QVERIFY(!src->isOpen());
    }
};

QTEST_MAIN(TestTransportAudioSource)

#include "test.moc"