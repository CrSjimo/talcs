#ifndef TALCS_REMOTEAUDIODEVICE_H
#define TALCS_REMOTEAUDIODEVICE_H

#include <TalcsDevice/AudioDevice.h>
#include <TalcsRemote/TalcsRemoteGlobal.h>

namespace talcs {

    class RemoteAudioDevicePrivate;
    class RemoteSocket;

    class TALCSREMOTE_EXPORT RemoteAudioDevice : public AudioDevice {
        Q_OBJECT
        Q_DECLARE_PRIVATE(RemoteAudioDevice)
    public:
        struct ProcessInfo {
            int containsInfo;

            //== Playback Status Info ==//
            enum PlaybackStatus {
                NotPlaying,
                Playing,
                RealtimePlaying,
            };
            PlaybackStatus status;

            //== Timeline Info ==//
            int timeSignatureNumerator;
            int timeSignatureDenominator;
            double tempo;

            int64_t position;
        };

        class ProcessInfoCallback {
        public:
            virtual void onThisBlockProcessInfo(const ProcessInfo &processInfo) = 0;
        };

    public:
        explicit RemoteAudioDevice(RemoteSocket *socket, const QString &name, QObject *parent = nullptr);
        ~RemoteAudioDevice() override;

        bool open(qint64 bufferSize, double sampleRate) override;
        void close() override;

        void addProcessInfoCallback(ProcessInfoCallback *callback);
        void removeProcessInfoCallback(ProcessInfoCallback *callback);

        bool start(AudioDeviceCallback *audioDeviceCallback) override;
        void stop() override;

        void lock() override;
        void unlock() override;

    signals:
        void remoteOpened(qint64 bufferSize, double sampleRate, int maxChannelCount);
    };

}

#endif // TALCS_REMOTEAUDIODEVICE_H