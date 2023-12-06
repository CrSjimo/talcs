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

#ifndef TALCS_REMOTEEDITOR_H
#define TALCS_REMOTEEDITOR_H

#include <QObject>

#include <TalcsRemote/TalcsRemoteGlobal.h>

namespace talcs {
    class RemoteSocket;

    class RemoteEditorPrivate;

    class TALCSREMOTE_EXPORT RemoteEditor : public QObject {
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

}

#endif // TALCS_REMOTEEDITOR_H
