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

#include "RemoteEditor.h"
#include "RemoteEditor_p.h"
#include "RemoteSocket.h"

namespace talcs {

    /**
     * @class RemoteEditor
     * @brief The editor functionalities of remote bridge.
     */

    /**
     * Constructor
     * @param socket the remote socket where this object runs
     * @param getFunc the callback function called when the remote bridge wants to get data from the editor
     * @param setFunc the callback function called when the remote bridge wants to set data to the editor
     */
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

    /**
     * Destructor
     */
    RemoteEditor::~RemoteEditor() = default;

    /**
     * Notifies the remote bridge that something has been changed in the editor.
     */
    void RemoteEditor::setDirty() {
        d->socket->call("editor", "requestMarkDirty");
    }

    /**
     * @fn void RemoteEditor::showEditorRequested()
     * Emitted when the remote bridge requests to show the editor window.
     */

    /**
     * @fn void RemoteEditor::hideEditorRequested()
     * Emitted when the remote bridge requests to hide the editor window.
     */
}
