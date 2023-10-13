#include <iostream>
#include <thread>
#include "remote/RemoteSocket.h"

using namespace talcs;

RemoteSocket *rs;

int main() {
    rs = new RemoteSocket(28082, 28081);
    QObject::connect(rs, &RemoteSocket::socketStatusChanged, [](RemoteSocket::Status newStatus) {
        qDebug() << newStatus;
    });
    std::cout << rs->startServer() << std::endl;
    std::cout << rs->startClient() << std::endl;
    std::cout << "started" << std::endl;
    for(;;);
    return 0;
}
