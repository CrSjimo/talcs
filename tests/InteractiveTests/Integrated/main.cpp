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

#include <TalcsCore/AudioBuffer.h>
#include <TalcsCore/InterleavedAudioDataWrapper.h>
#include <TalcsDevice/AudioDevice.h>
#include <TalcsDevice/AudioDriverManager.h>
#include <TalcsDevice/AudioSourcePlayback.h>
#include <TalcsDevice/AudioDriver.h>
#include <TalcsCore/MemoryAudioSource.h>
#include <TalcsCore/PositionableMixerAudioSource.h>
#include <TalcsCore/SineWaveAudioSource.h>
#include <TalcsCore/TransportAudioSource.h>
#include <QApplication>
#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QCheckBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QThread>
#include <cmath>

#include <TalcsFormat/AudioFormatIO.h>
#include <TalcsFormat/AudioFormatInputSource.h>
#include <TalcsFormat/AudioSourceWriter.h>
#include <TalcsCore/AudioSourceClipSeries.h>

#include <QProgressBar>

using namespace talcs;

int main(int argc, char **argv) {
    qputenv("QT_ENABLE_HIGHDPI_SCALING", "1");
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

    QApplication a(argc, argv);
    auto font = QFont("Microsoft Yahei UI", 9);
    font.setHintingPreference(QFont::PreferNoHinting);
    a.setFont(font);

    QMainWindow mainWindow;

    auto mainWidget = new QWidget;

    auto layout = new QFormLayout;

    auto driverComboBox = new QComboBox;

    auto deviceComboBox = new QComboBox;

    auto deviceSpecLabel = new QLabel;

    auto bufferSizeComboBox = new QComboBox;

    auto sampleRateComboBox = new QComboBox;

    auto fileNameLabel = new QLabel;

    auto transportSlider = new QSlider;
    transportSlider->setOrientation(Qt::Horizontal);
    transportSlider->setMinimum(0);
    transportSlider->setMaximum(100);

    auto playPauseButton = new QPushButton("Pause");
    playPauseButton->setDisabled(true);

    auto enableLoopingCheckBox = new QCheckBox("Looping");

    auto loopingStartSlider = new QSlider;
    loopingStartSlider->setOrientation(Qt::Horizontal);
    loopingStartSlider->setDisabled(true);

    auto loopingEndSlider = new QSlider;
    loopingEndSlider->setOrientation(Qt::Horizontal);
    loopingEndSlider->setDisabled(true);

    auto fileSpecLabel = new QLabel;

    auto leftLevelMeter = new QProgressBar;
    leftLevelMeter->setRange(0, 60);
    leftLevelMeter->setTextVisible(false);


    auto rightLevelMeter = new QProgressBar;
    rightLevelMeter->setRange(0, 60);
    rightLevelMeter->setTextVisible(false);

    auto browseFileButton = new QPushButton("Browse");

    auto startButton = new QPushButton("Start");

    auto resetPosButton = new QPushButton("Reset Position");

    auto stopButton = new QPushButton("Stop");
    stopButton->setDisabled(true);

    auto exportButton = new QPushButton("Export Audio");

    auto cplButton = new QPushButton("Open Control Panel");

    layout->addRow("Driver", driverComboBox);
    layout->addRow("Device", deviceComboBox);
    layout->addRow(cplButton);
    layout->addRow(deviceSpecLabel);
    layout->addRow("Buffer Size", bufferSizeComboBox);
    layout->addRow("Sample Rate", sampleRateComboBox);
    layout->addRow(fileNameLabel);
    layout->addRow(fileSpecLabel);
    layout->addRow(browseFileButton);
    layout->addRow("Level (L)", leftLevelMeter);
    layout->addRow("Level (R)", rightLevelMeter);
    layout->addRow("Transport", transportSlider);
    layout->addRow(playPauseButton);
    layout->addRow(enableLoopingCheckBox);
    layout->addRow("Loop Start", loopingStartSlider);
    layout->addRow("Loop End", loopingEndSlider);
    layout->addRow(startButton);
    layout->addRow(resetPosButton);
    layout->addRow(stopButton);
    layout->addRow(exportButton);

    mainWidget->setLayout(layout);
    mainWindow.setCentralWidget(mainWidget);

    AudioDriverManager *drvMgr = AudioDriverManager::createBuiltInDriverManager();

    driverComboBox->addItems(drvMgr->drivers());

    AudioDriver *driver = nullptr;
    AudioDevice *device = nullptr;

    QObject *deviceComboBoxCtx = nullptr;
    QObject *driverComboBoxCtx = nullptr;

    QList<QFile *> srcFileList;
    QList<AudioFormatIO *> srcIoList;
    QList<AudioFormatInputSource *> srcList;
    QList<PositionableMixerAudioSource *> trackSrcList;
    PositionableMixerAudioSource mixer;

    TransportAudioSource transportSrc;
    transportSrc.setSource(&mixer);
    AudioSourcePlayback playback(&transportSrc);

    qint64 effectiveLength = 0;

    QDialog *trackControlDlg = nullptr;

    auto reloadFile = [&](const QString &fileName) {
        if(fileName.isEmpty()) return;
        device->lock();
        mixer.removeAllSources();
        for(auto ptr: trackSrcList) delete ptr;
        for(auto ptr: srcIoList) delete ptr;
        for(auto ptr: srcFileList) delete ptr;
        srcFileList.clear();
        srcIoList.clear();
        trackSrcList.clear();

        if(trackControlDlg)
            trackControlDlg->accept();
        trackControlDlg = new QDialog;
        trackControlDlg->setWindowFlag(Qt::WindowContextHelpButtonHint, false);
        QObject::connect(trackControlDlg, &QDialog::finished, trackControlDlg, &QObject::deleteLater);
        auto trackControlDlgLayout = new QVBoxLayout;
        trackControlDlg->setLayout(trackControlDlgLayout);

        fileNameLabel->setText(fileName);
        QFile f(fileName);
        f.open(QFile::ReadOnly);
        auto doc = QJsonDocument::fromJson(f.readAll());
        for(const auto &audioFileNameJsonVal: doc.object().value("audioFiles").toArray()) {
            auto audioFile = new QFile(audioFileNameJsonVal.toString());
            qDebug() << audioFileNameJsonVal.toString();
            srcFileList.append(audioFile);
            audioFile->open(QIODevice::ReadOnly);
            srcIoList.append(new AudioFormatIO(audioFile));
        }
        int trkCnt = 0;
        for(const auto &trackSpec: doc.object().value("tracks").toArray()) {
            auto clipSeries = new AudioSourceClipSeries;
            auto trackSrc = new PositionableMixerAudioSource;
            trackSrc->addSource(clipSeries, true);
            trackSrcList.append(trackSrc);
            trackSrc->setGain(trackSpec.toObject().value("gain").toDouble(1.0));
            trackSrc->setPan(trackSpec.toObject().value("pan").toDouble(0.0));
            for(const auto &clipSpec: trackSpec.toObject().value("clips").toArray()) {
                auto audioId = clipSpec.toObject().value("audio").toInt();
                auto positionSec = clipSpec.toObject().value("pos").toDouble();
                auto startPosSec = clipSpec.toObject().value("start").toDouble();
                auto lengthSec = clipSpec.toObject().value("length").toDouble();
                qint64 position = positionSec * device->sampleRate();
                qint64 startPos = startPosSec * device->sampleRate();
                qint64 length = lengthSec * device->sampleRate();
                auto src = new AudioFormatInputSource(srcIoList[audioId]);
                srcList.append(src);
                if(!clipSeries->insertClip(src, position, startPos, length).isValid()) {
                    QMessageBox::critical(&mainWindow, "Mixer", "Cannot add clip.");
                }
            }
            effectiveLength = qMax(effectiveLength, clipSeries->effectiveLength());
            mixer.addSource(trackSrc);

            auto trackLayout = new QHBoxLayout;
            auto trackNameLabel = new QLabel("Track" + QString::number(trkCnt++));
            auto trackMute = new QCheckBox("Mute");
            auto trackSolo = new QCheckBox("Solo");
            QObject::connect(trackMute, &QCheckBox::clicked, trackSrc, [=](bool clicked){
                trackSrc->setSilentFlags(clicked ? -1 : 0);
            });
            QObject::connect(trackSolo, &QCheckBox::clicked, trackSrc, [=, &mixer](bool clicked){
                mixer.setSourceSolo(trackSrc, clicked);
            });
            trackLayout->addWidget(trackNameLabel);
            trackLayout->addWidget(trackMute);
            trackLayout->addWidget(trackSolo);
            trackControlDlgLayout->addLayout(trackLayout);
        }
        device->unlock();
        qint64 audioLength = effectiveLength;
        transportSlider->setRange(0, audioLength - 1);
        loopingStartSlider->setRange(0, audioLength - 1);
        loopingEndSlider->setRange(0, audioLength);
        loopingEndSlider->setValue(audioLength);
        trackControlDlg->show();
    };

    auto restartDevice = [&](){
        if(!device) return;
        device->close();
        if(!device->open(bufferSizeComboBox->currentText().toULongLong(), sampleRateComboBox->currentText().toDouble())) {
            QMessageBox::critical(&mainWindow, "Device Error", device->errorString());
        }
        reloadFile(fileNameLabel->text());
    };

    QObject::connect(driverComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), [&](int index){
        if(device) {
            device->close();
        }
        if(driver) {
            driver->finalize();
        }
        delete driverComboBoxCtx;
        if(driverComboBox->itemText(index).isEmpty()) return;
        driver = drvMgr->driver(driverComboBox->itemText(index));
        deviceComboBox->clear();
        if(!driver->initialize()) {
            QMessageBox::critical(&mainWindow, "Driver Error", driver->errorString());
            return;
        }
        auto defaultDev = driver->defaultDevice();
        auto deviceList = driver->devices();
        auto defaultDevIndex = deviceList.indexOf(defaultDev);
        deviceComboBox->addItems(deviceList);
        if(defaultDevIndex != -1) deviceComboBox->setCurrentIndex(defaultDevIndex);
        driverComboBoxCtx = new QObject;
        QObject::connect(deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), driverComboBoxCtx, [&](int index){
            if(device) {
                device->close();
                delete device;
            }
            delete deviceComboBoxCtx;
            if(deviceComboBox->itemText(index).isEmpty()) return;
            if(!driver) device = nullptr;
            device = driver->createDevice(deviceComboBox->itemText(index));
            deviceSpecLabel->setText(QString("preferred buffer size: %1, preferred sample rate: %2").arg(device->preferredBufferSize()).arg(device->preferredSampleRate()));
            auto availableBufferSizes = device->availableBufferSizes();
            bufferSizeComboBox->clear();
            for(int i = 0; i < availableBufferSizes.size(); i++) {
                bufferSizeComboBox->addItem(QString::number(availableBufferSizes[i]));
                if(availableBufferSizes[i] == device->preferredBufferSize()) bufferSizeComboBox->setCurrentIndex(i);
            }
            auto availableSampleRates = device->availableSampleRates();
            sampleRateComboBox->clear();
            for(int i = 0; i < availableSampleRates.size(); i++) {
                sampleRateComboBox->addItem(QString::number(availableSampleRates[i]));
                if(availableSampleRates[i] == device->preferredSampleRate()) sampleRateComboBox->setCurrentIndex(i);
            }
            deviceComboBoxCtx = new QObject;
            QObject::connect(bufferSizeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), deviceComboBoxCtx, restartDevice);
            QObject::connect(sampleRateComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), deviceComboBoxCtx, restartDevice);
            restartDevice();
        });
        if(deviceComboBox->count()) emit deviceComboBox->currentIndexChanged(deviceComboBox->currentIndex());
    });

    QObject::connect(cplButton, &QPushButton::clicked, [&](){
        if(device)
            device->openControlPanel();
    });

    if(driverComboBox->count()) emit driverComboBox->currentIndexChanged(driverComboBox->currentIndex());

    auto availableFormats = AudioFormatIO::availableFormats();

    mixer.setLevelMeterChannelCount(2);
    QObject::connect(&mixer, &PositionableMixerAudioSource::levelMetered, fileSpecLabel, [=](const QVector<float> &values){
        auto ml = values[0];
        auto mr = values[1];
        QString text = "bm: ";
        if(ml == 0) {
            text += "-inf dB, ";
            leftLevelMeter->reset();
        } else {
            double db = 10 * log(ml/1.0) / log(10);
            text += QString::number(db, 'f', 1) + " dB, ";
            leftLevelMeter->setValue(60 + db);
        }
        if(mr == 0) {
            text += "-inf dB, ";
            rightLevelMeter->reset();
        } else {
            double db = 10 * log(mr/1.0) / log(10);
            text += QString::number(db, 'f', 1) + " dB, ";
            rightLevelMeter->setValue(60 + db);
        }
        fileSpecLabel->setText(text);
    });

    QObject::connect(browseFileButton, &QPushButton::clicked, [&](){
        auto fileName = QFileDialog::getOpenFileName(&mainWindow, {}, {}, "*.json");
        reloadFile(fileName);
    });

    QObject::connect(startButton, &QPushButton::pressed, [&](){
        if(!device) return;
        if(!device->start(&playback)) {
            QMessageBox::critical(&mainWindow, "Playback Error", device->errorString());
        }
        deviceComboBox->setDisabled(true);
        driverComboBox->setDisabled(true);
        bufferSizeComboBox->setDisabled(true);
        sampleRateComboBox->setDisabled(true);
        playPauseButton->setDisabled(false);
        playPauseButton->setText("Pause");
        startButton->setDisabled(true);
        stopButton->setDisabled(false);
        transportSrc.play();
    });

    QObject::connect(resetPosButton, &QPushButton::clicked, [&](){
        transportSrc.setPosition(0);
    });

    QObject::connect(&transportSrc, &TransportAudioSource::positionAboutToChange, transportSlider, [&](qint64 value){
        QSignalBlocker blocker(transportSlider);
        transportSlider->setValue(value);
    });
    //    QObject::connect(transportSlider, &QSlider::valueChanged, &transportSrc, &TransportAudioSource::setPosition);
    QObject::connect(transportSlider, &QSlider::valueChanged, &transportSrc, &TransportAudioSource::setPosition);

    QObject::connect(playPauseButton, &QPushButton::clicked, [&](){
        if(transportSrc.isPlaying()) {
            transportSrc.pause();
            playPauseButton->setText("Play");
        } else {
            transportSrc.play();
            playPauseButton->setText("Pause");
        }
    });

    auto updateLoopingRange = [&](){
        if(enableLoopingCheckBox->isChecked()) {
            transportSrc.setLoopingRange(loopingStartSlider->value(), loopingEndSlider->value());
        } else {
            transportSrc.setLoopingRange(-1, -1);
        }
    };

    QObject::connect(enableLoopingCheckBox, &QCheckBox::clicked, [&](bool checked){
        loopingStartSlider->setDisabled(!checked);
        loopingEndSlider->setDisabled(!checked);
    });

    QObject::connect(enableLoopingCheckBox, &QCheckBox::clicked, updateLoopingRange);

    QObject::connect(loopingStartSlider, &QSlider::valueChanged, updateLoopingRange);

    QObject::connect(loopingEndSlider, &QSlider::valueChanged, updateLoopingRange);

    QObject::connect(stopButton, &QPushButton::clicked, [&](){
        device->stop();
        deviceComboBox->setDisabled(false);
        driverComboBox->setDisabled(false);
        bufferSizeComboBox->setDisabled(false);
        sampleRateComboBox->setDisabled(false);
        playPauseButton->setDisabled(true);
        playPauseButton->setText("Pause");
        startButton->setDisabled(false);
        stopButton->setDisabled(true);
    });

    QObject::connect(exportButton, &QPushButton::clicked, [&](){
        auto exportFileName = QFileDialog::getSaveFileName(&mainWindow, {}, {}, "*.wav");
        if(exportFileName.isEmpty()) return;

        emit stopButton->clicked();

        auto curBufSize = transportSrc.bufferSize();
        auto curSampleRate = device->sampleRate();

        transportSrc.close();

        TransportAudioSourceStateSaver saver(&transportSrc);
        transportSrc.setPosition(0);
        transportSrc.setLoopingRange(-1, -1);

        QFile exportFile(exportFileName);
        AudioFormatIO exportIO(&exportFile);
        exportFile.open(QIODevice::WriteOnly);
        exportIO.open(AbstractAudioFormatIO::Write, AudioFormatIO::WAV | AudioFormatIO::PCM_24, 2, curSampleRate);

        transportSrc.open(65536, curSampleRate);
        transportSrc.setPosition(0);
        QDialog dlg;
        dlg.setWindowTitle("Exporting...");
        auto dlgLayout = new QVBoxLayout;
        auto exportProgressBar = new QProgressBar;
        dlgLayout->addWidget(exportProgressBar);
        dlg.setLayout(dlgLayout);
        QThread thread;
        AudioSourceWriter writer(&transportSrc, &exportIO, effectiveLength);
        writer.moveToThread(&thread);
        QObject::connect(&thread, &QThread::started, &writer, &AudioSourceWriter::start);
        QObject::connect(&writer, &AudioSourceWriter::blockProcessed, exportProgressBar, [=](qint64 sampleCountProcessed){
            exportProgressBar->setValue(sampleCountProcessed * 100 / effectiveLength);
        });
        QObject::connect(&dlg, &QDialog::rejected, &thread, [&](){
            writer.interrupt();
        });
        QObject::connect(&writer, &AudioSourceWriter::finished, &mainWindow, [&]{
            if(writer.status() == talcs::AudioSourceProcessorBase::Interrupted)
                QMessageBox::warning(&mainWindow, "Export", "Exporting is interrupted.");
            else
                dlg.accept();
        });
        transportSrc.play();
        thread.start();
        dlg.exec();
        transportSrc.close();
        leftLevelMeter->reset();
        rightLevelMeter->reset();
        thread.quit();
        thread.wait();
    });

    mainWindow.show();

    return a.exec();
}