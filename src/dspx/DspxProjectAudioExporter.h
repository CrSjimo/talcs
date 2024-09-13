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

#ifndef TALCS_DSPXPROJECTAUDIOEXPORTER_H
#define TALCS_DSPXPROJECTAUDIOEXPORTER_H

#include <QObject>

#include <TalcsDspx/TalcsDspxGlobal.h>

namespace talcs {

    class AbstractAudioFormatIO;

    class DspxProjectContext;
    class DspxTrackContext;

    class DspxProjectAudioExporterPrivate;

    class TALCSDSPX_EXPORT DspxProjectAudioExporter : public QObject {
        Q_OBJECT
        Q_DECLARE_PRIVATE(DspxProjectAudioExporter)
    public:
        explicit DspxProjectAudioExporter(DspxProjectContext *context, QObject *parent = nullptr);
        ~DspxProjectAudioExporter() override;

        void setMuteSoloEnabled(bool);
        bool isMuteSoloEnabled() const;

        void setRange(int startTick, int lengthTick);
        int start() const;
        int length() const;

        void setThruMaster(bool);
        bool thruMaster() const;

        void addSeparatedTask(DspxTrackContext *track, AbstractAudioFormatIO *outFile);
        void setMixedTask(const QList<DspxTrackContext *> &tracks, AbstractAudioFormatIO *outFile);
        void clearTask();

        void setClippingCheckEnabled(bool);
        bool isClippingCheckEnabled() const;

        enum ResultFlag {
            OK = 0x0000,
            Fail = 0x0001,
            Interrupted = 0x0002,

            ClippingDetected = 0x0010,
        };
        Q_DECLARE_FLAGS(Result, ResultFlag)
        Result exec();

        void interrupt(bool isFail = false);

    signals:
        void clippingDetected(DspxTrackContext *track);
        void progressChanged(double progressRatio, DspxTrackContext *track);

    private:
        QScopedPointer<DspxProjectAudioExporterPrivate> d_ptr;

    };

} // talcs

#endif //TALCS_DSPXPROJECTAUDIOEXPORTER_H
