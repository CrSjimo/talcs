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

#ifndef AUDIODRIVERMANAGER_P_H
#define AUDIODRIVERMANAGER_P_H

#include <QList>

#include <TalcsDevice/AudioDriverManager.h>

namespace talcs {
    class AudioDriverManagerPrivate {
        Q_DECLARE_PUBLIC(AudioDriverManager)
    public:
        AudioDriverManager *q_ptr;
        QList<QPair<QString, AudioDriver *>> driverDict;
    };
}

#endif // AUDIODRIVERMANAGER_P_H
