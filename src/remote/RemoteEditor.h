#ifndef TALCS_REMOTEEDITOR_H
#define TALCS_REMOTEEDITOR_H

#include <QObject>

#include "global/TalcsGlobal.h"

namespace talcs {
    class RemoteSocket;

    class RemoteEditorPrivate;

    class TALCS_EXPORT RemoteEditor : public QObject {
        Q_OBJECT
        friend class RemoteEditorPrivate;
        using GetFunc = std::function<QByteArray (bool *)>;
        using SetFunc = std::function<bool (const QByteArray &)>;
    public:
        explicit RemoteEditor(RemoteSocket *socket, const GetFunc &getFunc, const SetFunc &setFunc, QObject *parent = nullptr);
        ~RemoteEditor() override;

        void setDirty();

    signals:
        void showEditorRequested();
        void hideEditorRequested();

    private:
        QScopedPointer<RemoteEditorPrivate> d;
    };

} // talcs

#endif // TALCS_REMOTEEDITOR_H
