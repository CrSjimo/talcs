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

#ifndef TALCS_SIMPLEAUDIOAPPLICATIONWINDOW_H
#define TALCS_SIMPLEAUDIOAPPLICATIONWINDOW_H

#include <QMainWindow>

namespace talcs {
    class AudioDriverManager;
    class AudioDriver;
    class AudioDevice;
    class AudioSourcePlayback;
    class TransportAudioSource;
    class AudioFormatIO;
    class BufferingAudioSource;
}

class QLabel;
class QSlider;
class QComboBox;
class QPushButton;
class QFile;
class QFileDialog;

class SimpleAudioApplicationWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit SimpleAudioApplicationWindow(QWidget *parent = nullptr);
    ~SimpleAudioApplicationWindow() override;

private:
    void setFileDialogFilters();
    void initializeAudioDevice();
    void initializeAudioSource();
    void openFile();
    void stopAudio();
    void toggleAudioState();
    void updateDriverAndDevice(const QString &driver, const QString &device);
    void handleDeviceChange();

    QLabel *m_fileNameLabel;
    QSlider *m_transportSlider;
    QLabel *m_positionLabel;
    QComboBox *m_driverComboBox;
    QComboBox *m_deviceComboBox;
    QPushButton *m_playPauseButton;
    QPushButton *m_stopButton;
    QFileDialog *m_fileDialog;

    talcs::AudioDriverManager *m_drvMgr;
    talcs::AudioDriver *m_drv = nullptr;
    talcs::AudioDevice *m_dev = nullptr;

    QFile *m_audioFile;
    talcs::AudioSourcePlayback *m_playback;
    talcs::TransportAudioSource *m_tpSrc;
    talcs::AudioFormatIO *m_formatIo;
    talcs::BufferingAudioSource *m_bufSrc;
};


#endif //TALCS_SIMPLEAUDIOAPPLICATIONWINDOW_H
