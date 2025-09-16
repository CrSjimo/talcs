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
#include <QFileDialog>
#include <QFile>
#include <QDebug>
#include <QPainter>
#include <QGridLayout>
#include <QSlider>
#include <QElapsedTimer>

#include <TalcsFormat/AudioFormatIO.h>
#include <TalcsFormat/AudioFormatInputSource.h>
#include <TalcsGui/WaveformPainter.h>

using namespace talcs;

static WaveformPainter waveformPainter;

static int startPosSecond = 0;
static int lengthSecond = 1;

class DemoWidget : public QWidget {
protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        painter.setPen(Qt::NoPen);
        painter.setBrush(Qt::blue);
        QElapsedTimer timer;
        timer.start();
        waveformPainter.paint(&painter, rect(), startPosSecond, lengthSecond);
        qDebug() << timer.nsecsElapsed();
    }
};

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    auto fileName = QFileDialog::getOpenFileName();
    QFile f(fileName);
    f.open(QIODevice::ReadOnly);
    AudioFormatIO io(&f);
    io.open(AbstractAudioFormatIO::Read);
    AudioFormatInputSource src(&io);
    src.open(1024, io.sampleRate());

    waveformPainter.setSource(&src, 2, io.length(), true);
    waveformPainter.startLoad(0, io.length());

    QWidget widget;
    auto mainLayout = new QGridLayout;

    auto demoWidget = new DemoWidget;
    mainLayout->addWidget(demoWidget, 0, 0);

    auto startPosSlider = new QSlider;
    startPosSlider->setOrientation(Qt::Horizontal);
    mainLayout->addWidget(startPosSlider, 1, 0);

    auto lengthSlider = new QSlider;
    lengthSlider->setRange(1, 100);
    mainLayout->addWidget(lengthSlider, 0, 1);

    widget.setLayout(mainLayout);

    widget.show();

    QObject::connect(&waveformPainter, &WaveformPainter::loadFinished, &widget, [&](qint64 startPos, qint64 length) {
        qDebug() << startPos << length;
    });

    QObject::connect(startPosSlider, &QSlider::valueChanged, [=](int value) {
        startPosSecond = value;
        demoWidget->update();
    });
    QObject::connect(lengthSlider, &QSlider::valueChanged, [=](int value) {
        lengthSecond = value;
        demoWidget->update();
    });

    return a.exec();
}
