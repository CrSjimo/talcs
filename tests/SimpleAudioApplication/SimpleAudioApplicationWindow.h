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
};


#endif //TALCS_SIMPLEAUDIOAPPLICATIONWINDOW_H
