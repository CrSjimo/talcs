#include "RemoteEditor.h"
#include "RemoteEditor_p.h"

#include "RemoteSocket.h"

namespace talcs {
    RemoteEditor::RemoteEditor(RemoteSocket *socket, const RemoteEditor::GetFunc &getFunc,
                               const RemoteEditor::SetFunc &setFunc, QObject *parent)
        : QObject(parent), d(new RemoteEditorPrivate{socket, getFunc, setFunc}) {
        socket->bind("editor", "putDataToEditor", [=](const std::vector<char> &data) {
            QByteArray a(data.data(), data.size());
            return d->setFunc(a);
        });
        socket->bind("editor", "getDataFromEditor", [=]() {
            bool ok = false;
            auto data = d->getFunc(&ok);
            if (!ok)
                throw std::runtime_error("Cannot get data from editor.");
            return std::vector<char>(data.begin(), data.end());
        });
        socket->bind("editor", "show", [=]() { emit showEditorRequested(); });
        socket->bind("editor", "hide", [=]() { emit hideEditorRequested(); });
    }

    RemoteEditor::~RemoteEditor() {
        d->socket->unbind("editor", "putDataToEditor");
        d->socket->unbind("editor", "getDataFromEditor");
        d->socket->unbind("editor", "show");
        d->socket->unbind("editor", "hide");
    }

    void RemoteEditor::setDirty() {
        d->socket->call("editor", "requestMarkDirty");
    }
} // talcs