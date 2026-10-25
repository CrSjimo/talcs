// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef TALCS_WASAPIAUDIODEVICE_P_H
#define TALCS_WASAPIAUDIODEVICE_P_H

#include <QMutex>
#include <QSet>
#include <QThread>

#include <TalcsCore/AudioBuffer.h>
#include <TalcsDevice/private/AudioDevice_p.h>
#include <TalcsDevice/private/WASAPIAudioDriver_p.h>
#include <TalcsDevice/private/WASAPIAudioUtils_p.h>

QT_BEGIN_NAMESPACE
class QTimer;
class QWinEventNotifier;
QT_END_NAMESPACE

namespace talcs {

    struct WASAPIDeviceCapabilities {
        int channelCount = 0;
        QList<double> sampleRates;
        double preferredSampleRate = 0;
        QList<qint64> bufferSizes;
        qint64 preferredBufferSize = 0;
    };

    class WASAPIAudioDevicePrivate;

    class WASAPIAudioDevice : public AudioDevice {
        Q_OBJECT
        Q_DECLARE_PRIVATE(WASAPIAudioDevice)
    public:
        WASAPIAudioDevice(const QString &name, const QString &endpointId, WASAPIAudioDriver *driver);
        ~WASAPIAudioDevice() override;

        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;
        bool start(AudioDeviceCallback *callback) override;
        void stop() override;
        void lock() override;
        void unlock() override;
        bool openControlPanel() override;

        void handleEndpointChange(const QList<WASAPIEndpoint> &endpoints, const QSet<QString> &changedFormats);

    private:
        bool updateCapabilities();
        void handleStreamFailure(quint64 generation, HRESULT result);
    };

    // All endpoint and stream COM interfaces remain in this object's COM apartment.
    class WASAPIAudioWorker : public QObject {
        Q_OBJECT
    public:
        explicit WASAPIAudioWorker(WASAPIAudioDriver::ShareMode mode);
        HRESULT initialize();
        void finalize();
        HRESULT probe(const QString &id, WASAPIDeviceCapabilities &capabilities);
        HRESULT open(const QString &id, qint64 blockSize, double sampleRate, int activeChannels, quint64 generation);
        void close();
        HRESULT start(AudioDeviceCallback *callback, quint64 generation);
        void stop();
        qint64 bufferSize() const;
        void lock();
        void unlock();

    Q_SIGNALS:
        void streamFailed(quint64 generation, long result);

    private:
        HRESULT activate(const QString &id);
        HRESULT selectFormat(double sampleRate);
        HRESULT render();
        void processAudio();

        WASAPIAudioDriver::ShareMode m_mode;
        QMutex m_callbackMutex;
        bool m_ownsCom = false;
        Microsoft::WRL::ComPtr<IMMDeviceEnumerator> m_enumerator;
        Microsoft::WRL::ComPtr<IMMDevice> m_endpoint;
        Microsoft::WRL::ComPtr<IAudioClient3> m_client;
        Microsoft::WRL::ComPtr<IAudioRenderClient> m_renderClient;
        WAVEFORMATEXTENSIBLE m_format{};
        HANDLE m_audioEvent = nullptr;
        HANDLE m_mmcssTask = nullptr;
        QWinEventNotifier *m_notifier = nullptr;
        QTimer *m_watchdog = nullptr;
        UINT32 m_endpointFrames = 0;
        AudioBuffer m_buffer;
        qint64 m_bufferPosition = 0;
        AudioDeviceCallback *m_callback = nullptr;
        quint64 m_generation = 0;
        bool m_floatFormat = false;
        bool m_running = false;
    };

    class WASAPIAudioDevicePrivate : public AudioDevicePrivate {
        Q_DECLARE_PUBLIC(WASAPIAudioDevice)
    public:
        QThread workerThread;
        WASAPIAudioWorker *worker = nullptr;
        QString endpointId;
        AudioDeviceCallback *callback = nullptr;
        quint64 generation = 0;
        bool workerInitialized = false;
    };

}

#endif // TALCS_WASAPIAUDIODEVICE_P_H
