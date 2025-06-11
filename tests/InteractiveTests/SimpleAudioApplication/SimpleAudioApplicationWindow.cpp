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

#include "SimpleAudioApplicationWindow.h"

#include <TalcsDevice/AudioDriverManager.h>
#include <TalcsDevice/AudioDriver.h>
#include <TalcsDevice/AudioDevice.h>
#include <TalcsDevice/AudioSourcePlayback.h>
#include <TalcsCore/TransportAudioSource.h>
#include <TalcsFormat/AudioFormatInputSource.h>
#include <TalcsFormat/AudioFormatIO.h>
#include <TalcsCore/BufferingAudioSource.h>

#include <QBoxLayout>
#include <QDebug>
#include <QFileDialog>
#include <QPushButton>
#include <QSlider>
#include <QLabel>
#include <QComboBox>
#include <QFile>
#include <QTimer>
#include <QMessageBox>

using namespace talcs;

static QString posToStr(qint64 position, double sampleRate) {
    int sec = position / sampleRate;
    int minute = sec / 60;
    sec %= 60;
    return QString("%1:%2")
            .arg(minute, 2, 10, QChar('0'))
            .arg(sec, 2, 10, QChar('0'));
}

SimpleAudioApplicationWindow::SimpleAudioApplicationWindow(QWidget *parent) : QMainWindow(parent) {
    auto mainWidget = new QWidget;
    auto mainLayout = new QHBoxLayout;
    auto fileLayout = new QVBoxLayout;
    auto browseButton = new QPushButton("Browse");
    m_driverComboBox = new QComboBox;
    m_deviceComboBox = new QComboBox;
    fileLayout->addWidget(browseButton);
    fileLayout->addWidget(m_driverComboBox);
    fileLayout->addWidget(m_deviceComboBox);
    mainLayout->addLayout(fileLayout, 1);
    auto audioLayout = new QVBoxLayout;
    m_fileNameLabel = new QLabel;
    audioLayout->addWidget(m_fileNameLabel);
    auto transportLayout = new QHBoxLayout;
    m_transportSlider = new QSlider;
    m_transportSlider->setEnabled(false);
    m_transportSlider->setOrientation(Qt::Horizontal);
    m_positionLabel = new QLabel("00:00/00:00");
    transportLayout->addWidget(m_transportSlider);
    transportLayout->addWidget(m_positionLabel);
    audioLayout->addLayout(transportLayout);
    auto controlLayout = new QHBoxLayout;
    m_playPauseButton = new QPushButton("Play");
    m_playPauseButton->setEnabled(false);
    m_stopButton = new QPushButton("Stop");
    controlLayout->addWidget(m_playPauseButton);
    controlLayout->addWidget(m_stopButton);
    audioLayout->addLayout(controlLayout);
    mainLayout->addLayout(audioLayout, 2);
    mainWidget->setLayout(mainLayout);
    setCentralWidget(mainWidget);

    m_fileDialog = new QFileDialog(this);
    setFileDialogFilters();

    connect(browseButton, &QPushButton::clicked, this, &SimpleAudioApplicationWindow::openFile);
    connect(m_playPauseButton, &QPushButton::clicked, this, &SimpleAudioApplicationWindow::toggleAudioState);
    connect(m_stopButton, &QPushButton::clicked, this, &SimpleAudioApplicationWindow::stopAudio);
    connect(m_driverComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        updateDriverAndDevice(m_driverComboBox->itemText(index), {});
    });
    connect(m_deviceComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
        updateDriverAndDevice({}, m_deviceComboBox->itemText(index));
    });

    initializeAudioDevice();
    initializeAudioSource();
}

SimpleAudioApplicationWindow::~SimpleAudioApplicationWindow() = default;

void SimpleAudioApplicationWindow::setFileDialogFilters() {
    QSet<QString> allExtensions;
    QStringList filters;
    auto types = AudioFormatIO::availableFormats();
    for (const auto &type: types) {
        QStringList extensions = {"*." + type.extension};
        allExtensions.insert("*." + type.extension);
        for (const auto &subtype: type.subtypes) {
            for (const auto &extension : subtype.extensions) {
                extensions.append("*." + extension);
                allExtensions.insert("*." + extension);
            }
        }
        filters.append(QString("%1 (%2)").arg(type.name, extensions.join(" ")));
    }
    filters.prepend(QString("All supported files (%1)").arg(allExtensions.values().join(" ")));
    filters.append("All files (*)");
    m_fileDialog->setNameFilters(filters);
}

void SimpleAudioApplicationWindow::initializeAudioDevice() {
    m_drvMgr = AudioDriverManager::createBuiltInDriverManager(this);
    if (m_drvMgr->drivers().isEmpty()) {
        QMessageBox::critical(this, {}, "No audio driver!");
        return;
    }
    {
        QSignalBlocker sb(m_driverComboBox);
        m_driverComboBox->addItems(m_drvMgr->drivers());
    }
    m_drv = m_drvMgr->driver(m_drvMgr->drivers()[0]);
    if (!m_drv || !m_drv->initialize()) {
        QMessageBox::critical(this, {}, "Cannot initialize audio driver: " + m_drv->name());
        return;
    }
    if (m_drv->devices().isEmpty()) {
        QMessageBox::critical(this, {}, "No audio device in audio driver: " + m_drv->name());
        return;
    }
    connect(m_drv, &AudioDriver::deviceChanged, this, &SimpleAudioApplicationWindow::handleDeviceChange);
    {
        QSignalBlocker sb(m_deviceComboBox);
        m_deviceComboBox->addItems(m_drv->devices());
        if (!m_drv->defaultDevice().isEmpty()) {
            m_deviceComboBox->setCurrentIndex(m_drv->devices().indexOf(m_drv->defaultDevice()));
        }
    }
    m_dev = m_drv->createDevice(m_drv->defaultDevice().isEmpty() ? m_drv->devices()[0] : m_drv->defaultDevice());
    if (!m_dev || !m_dev->isInitialized() || !m_dev->open(m_dev->preferredBufferSize(), m_dev->preferredSampleRate())) {
        QMessageBox::critical(this, {}, "Cannot initialize audio device: " + m_dev->name());
        return;
    }
}

void SimpleAudioApplicationWindow::initializeAudioSource() {
    m_formatIo = new AudioFormatIO;
    auto fmtSrc = new AudioFormatInputSource(m_formatIo, true);
    m_bufSrc = new BufferingAudioSource(fmtSrc, true, 2, 114514);
    m_tpSrc = new TransportAudioSource(m_bufSrc, true, this);
    m_playback = new AudioSourcePlayback(m_tpSrc);
    m_audioFile = new QFile(m_tpSrc);
    m_formatIo->setStream(m_audioFile);

    connect(m_transportSlider, &QSlider::valueChanged, m_tpSrc, &TransportAudioSource::setPosition);
    connect(m_tpSrc, &TransportAudioSource::positionAboutToChange, this, [=](qint64 pos) {
        m_positionLabel->setText(posToStr(pos, m_dev->sampleRate()) + m_positionLabel->text().mid(m_positionLabel->text().indexOf("/")));
        QSignalBlocker sb(m_transportSlider);
        m_transportSlider->setValue(pos);
    });
}

void SimpleAudioApplicationWindow::openFile() {
    if (m_fileDialog->exec() == QDialog::Rejected)
        return;
    stopAudio();
    m_playPauseButton->setEnabled(false);
    m_transportSlider->setEnabled(false);
    m_audioFile->setFileName(m_fileDialog->selectedFiles()[0]);
    m_audioFile->open(QIODevice::ReadOnly);
    if (!m_dev)
        return;
    if (!m_tpSrc->open(m_dev->bufferSize(), m_dev->sampleRate())) {
        QMessageBox::critical(this, {}, "Cannot open audio file:\n" + m_audioFile->fileName());
        m_tpSrc->close();
        m_audioFile->setFileName({});
        m_fileNameLabel->clear();
        QTimer::singleShot(0, [=] {
            // idk why, but it cannot work without QTimer::singleShot.
            m_transportSlider->setValue(0);
            m_positionLabel->setText("00:00/00:00");
        });
        return;
    }
    m_transportSlider->setMaximum(m_tpSrc->length());
    m_tpSrc->setPosition(0);
    QTimer::singleShot(0, [=] {
        m_transportSlider->setValue(0);
        m_positionLabel->setText("00:00/" + posToStr(m_tpSrc->length(), m_tpSrc->sampleRate()));
    });
    m_fileNameLabel->setText(m_audioFile->fileName());
    m_playPauseButton->setEnabled(true);
    m_transportSlider->setEnabled(true);
}

void SimpleAudioApplicationWindow::stopAudio() {
    if (m_dev)
        m_dev->stop();
    m_tpSrc->pause();
    m_playPauseButton->setText("Play");
}

void SimpleAudioApplicationWindow::toggleAudioState() {
    if (!m_dev)
        return;
    if (!m_dev->isStarted()) {
        if (!m_dev->start(m_playback)) {
            QMessageBox::critical(this, {}, "Cannot start audio playback!");
            return;
        }
        m_tpSrc->play();
        m_playPauseButton->setText("Pause");
    } else {
        if (m_tpSrc->isPlaying()) {
            m_tpSrc->pause();
            m_playPauseButton->setText("Play");
        } else {
            m_tpSrc->play();
            m_playPauseButton->setText("Pause");
        }
    }
}

void SimpleAudioApplicationWindow::updateDriverAndDevice(const QString &driver, const QString &device) {
    bool wasStarted = m_dev && m_dev->isStarted();
    stopAudio();

    if (!driver.isEmpty()) {
        if (m_drv) {
            m_drv->finalize();
            disconnect(m_drv, nullptr, this, nullptr);
        }
        m_dev = nullptr;
        m_drv = m_drvMgr->driver(driver);
        if (!m_drv || !m_drv->initialize()) {
            QMessageBox::critical(this, {}, "Cannot initialize audio driver: " + m_drv->name());
            return;
        }
        if (m_drv->devices().isEmpty()) {
            QMessageBox::critical(this, {}, "No audio device in audio driver: " + m_drv->name());
            return;
        }
        connect(m_drv, &AudioDriver::deviceChanged, this, &SimpleAudioApplicationWindow::handleDeviceChange);
        {
            QSignalBlocker sb(m_deviceComboBox);
            m_deviceComboBox->clear();
            m_deviceComboBox->addItems(m_drv->devices());
            if (!m_drv->defaultDevice().isEmpty()) {
                m_deviceComboBox->setCurrentIndex(m_drv->devices().indexOf(m_drv->defaultDevice()));
            }
        }
        m_dev = m_drv->createDevice(m_drv->defaultDevice().isEmpty() ? m_drv->devices()[0] : m_drv->defaultDevice());
        if (!m_dev || !m_dev->isInitialized() || !m_dev->open(m_dev->preferredBufferSize(), m_dev->preferredSampleRate())) {
            QMessageBox::critical(this, {}, "Cannot initialize audio device: " + m_dev->name());
            return;
        }
    }

    if (!device.isEmpty() && m_drv) {
        delete m_dev;
        m_dev = m_drv->createDevice(device);
        if (!m_dev || !m_dev->isInitialized() || !m_dev->open(m_dev->preferredBufferSize(), m_dev->preferredSampleRate())) {
            QMessageBox::critical(this, {}, "Cannot initialize audio device: " + m_dev->name());
            return;
        }
    }

    if (wasStarted) {
        m_dev->start(m_playback);
        m_tpSrc->play();
        m_playPauseButton->setText("Pause");
    }
}

void SimpleAudioApplicationWindow::handleDeviceChange() {
    auto devList = m_drv->devices();
    if (devList.isEmpty()) {
        QMessageBox::critical(this, {}, "No audio device in audio driver: " + m_drv->name());
        return;
    }
    {
        QSignalBlocker sb(m_deviceComboBox);
        m_deviceComboBox->clear();
        m_deviceComboBox->addItems(m_drv->devices());
    }
    if (m_dev && !devList.contains(m_dev->name())) {
        stopAudio();
        updateDriverAndDevice({}, m_drv->defaultDevice().isEmpty() ? devList[0] : m_drv->defaultDevice());
    } else {
        m_deviceComboBox->setCurrentIndex(devList.indexOf(m_dev->name()));
    }
}
