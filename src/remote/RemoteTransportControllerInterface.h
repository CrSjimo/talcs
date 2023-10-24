#ifndef TALCS_REMOTETRANSPORTCONTROLLERINTERFACE_H
#define TALCS_REMOTETRANSPORTCONTROLLERINTERFACE_H

#include <QtGlobal>

namespace talcs {
    class RemoteSocket;

    class RemoteTransportControllerInterface {
    public:
        explicit RemoteTransportControllerInterface(RemoteSocket *socket);
        ~RemoteTransportControllerInterface() = default;

        void play();
        void pause();

        void setPosition(qint64 position);

        void setLoopingRange(qint64 start, qint64 end);
        void toggleLooping(bool enabled);

    private:
        RemoteSocket *m_socket;
    };

}

#endif // TALCS_REMOTETRANSPORTCONTROLLERINTERFACE_H
