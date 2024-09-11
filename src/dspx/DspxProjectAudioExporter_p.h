/******************************************************************************
 * Copyright (c) 2024 CrSjimo                                                 *
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

#ifndef TALCS_DSPXPROJECTAUDIOEXPORTER_P_H
#define TALCS_DSPXPROJECTAUDIOEXPORTER_P_H

#include <TalcsDspx/DspxProjectAudioExporter.h>

namespace talcs {
    class DspxProjectAudioExporterPrivate {
        Q_DECLARE_PUBLIC(DspxProjectAudioExporter)
    public:
        DspxProjectAudioExporter *q_ptr;

        DspxProjectContext *projectContext;

        bool isMuteSoloEnabled{};
        int startTick{};
        int lengthTick{};
        bool thruMaster{};
        bool isClippingCheckEnabled{};

        QList<QPair<DspxTrackContext *, AbstractAudioFormatIO *>> separatedTasks;
        QList<DspxTrackContext *> mixedTask;
        AbstractAudioFormatIO *mixedTaskOutFile{};

        qint64 savedMixerPosition{};

        void saveMixerState();
        void restoreMixerState();

        void makeMixedTaskMixerLayout();
        AbstractAudioFormatIO *makeNextSeparatedThruMasterTaskMixerLayoutAndGetCorrespondingIO();

    };
}

#endif //TALCS_DSPXPROJECTAUDIOEXPORTER_P_H
