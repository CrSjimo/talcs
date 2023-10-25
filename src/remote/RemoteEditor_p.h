#ifndef TALCS_REMOTEEDITOR_P_H
#define TALCS_REMOTEEDITOR_P_H

#include <TalcsRemote/RemoteEditor.h>

namespace talcs {

    class RemoteEditorPrivate {
    public:
        RemoteSocket *socket;
        RemoteEditor::GetFunc getFunc;
        RemoteEditor::SetFunc setFunc;
    };

}

#endif // TALCS_REMOTEEDITOR_P_H
