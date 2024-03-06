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

#include <TalcsCore/BufferingAudioSource.h>
#include <TalcsCore/SineWaveAudioSource.h>
#include <TalcsCore/AudioBuffer.h>

using namespace talcs;

class TestBufferingAudioSource: public QObject {
    Q_OBJECT
private slots:
    void initializeObject() {
        SineWaveAudioSource src(440);
        BufferingAudioSource bufSrc(&src, 1, 65536);
        QCOMPARE(bufSrc.readAheadSize(), 65536);
        QCOMPARE(bufSrc.channelCount(), 1);
        bufSrc.setReadAheadSize(114514);
        bufSrc.setChannelCount(2);
        QCOMPARE(bufSrc.readAheadSize(), 114514);
        QCOMPARE(bufSrc.channelCount(), 2);
    }

    void openSource() {
        SineWaveAudioSource src(440);
        BufferingAudioSource bufSrc(&src, 2, 65536);
        bufSrc.close();
        bufSrc.open(1024, 48000);
        QVERIFY(src.isOpen());
        QCOMPARE(src.bufferSize(), 1024);
        QCOMPARE(src.sampleRate(), 48000);
        QVERIFY(bufSrc.waitForBuffering(QDeadlineTimer(2000)));
        QCOMPARE(bufSrc.nextReadPosition(), 0);
        QCOMPARE(src.nextReadPosition(), 65536);

        bufSrc.open(1024, 48000);
        QVERIFY(bufSrc.waitForBuffering(QDeadlineTimer(2000)));
        QCOMPARE(bufSrc.nextReadPosition(), 0);
        QCOMPARE(src.nextReadPosition(), 65536);

        bufSrc.setReadAheadSize(114514);
        QVERIFY(bufSrc.waitForBuffering(QDeadlineTimer(2000)));
//        QCOMPARE(src.nextReadPosition(), 114514);

        bufSrc.setNextReadPosition(1919810);
        QVERIFY(bufSrc.waitForBuffering(QDeadlineTimer(2000)));
        QCOMPARE(bufSrc.nextReadPosition(), 1919810);
//        QCOMPARE(src.nextReadPosition(), 1919810 + 114514);

        SineWaveAudioSource src2(440);
        bufSrc.setSource(&src2);
        QVERIFY(src2.isOpen());
        QCOMPARE(src2.bufferSize(), 1024);
        QCOMPARE(src2.sampleRate(), 48000);
        QVERIFY(bufSrc.waitForBuffering(QDeadlineTimer(2000)));
        QCOMPARE(bufSrc.nextReadPosition(), 1919810);
//        QCOMPARE(src2.nextReadPosition(), 1919810 + 114514);
        bufSrc.setSource(&src);
    }

    void readIdentical() {
        SineWaveAudioSource src(440);
        BufferingAudioSource bufSrc(&src, 2, 65536);
        AudioBuffer buf(2, 65536);
        qint64 totalSize = 0;
        bufSrc.open(1024, 48000);
        QThread *t = nullptr;
        bool quitFlag = false;
        t = QThread::create([&] {
            for (;;) {
                if (quitFlag)
                    return;
                if (totalSize + 1024 >= buf.sampleCount())
                    buf.resize(-1, buf.sampleCount() + 65536);
                bufSrc.read({&buf, totalSize, 1024});
                totalSize += 1024;
            }
        });
        QTimer timer;
        timer.setInterval(0);
        int intervalCount = 0;
        timer.callOnTimeout([&] {
            intervalCount++;
            bufSrc.flush();
            if (bufSrc.nextReadPosition() > 131072 && intervalCount > 32) {
                quitFlag = true;
                t->quit();
                t->wait(2000);
                timer.stop();
            }
        });
        QEventLoop eventLoop;
        connect(t, &QThread::finished, &eventLoop, &QEventLoop::quit);
        timer.start();
        t->start();
        eventLoop.exec();
        qDebug() << "Interval count:" << intervalCount;
        AudioBuffer refBuf(buf.channelCount(), totalSize);
        SineWaveAudioSource refSrc(440);
        refSrc.open(totalSize, 48000);
        refSrc.read(&refBuf);

        for (qint64 i = 0; i < totalSize; i++) {
            QCOMPARE(buf.constData(0)[i], refBuf.constData(0)[i]);
            QCOMPARE(buf.constData(1)[i], refBuf.constData(1)[i]);
        }
    }


};

QTEST_MAIN(TestBufferingAudioSource)

#include "test.moc"