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

#include <QHash>

#include <TalcsCore/AudioSource.h>

#include <TalcsFormat/AudioSourceWriter.h>

namespace talcs {
    class PositionableMixerAudioSource;
}

namespace talcs {

    class DspxProjectAudioExporterSourceWriter : public AudioSourceWriter {
        Q_OBJECT
    public:
        DspxProjectAudioExporterSourceWriter(DspxProjectAudioExporterPrivate *d, DspxTrackContext *trackContext, AudioSource *src, AbstractAudioFormatIO *outFile, qint64 length);
    protected:
        IAudioSampleContainer *prepareBuffer() override {
            return m_buf = AudioSourceWriter::prepareBuffer();
        }
        bool processBlock(qint64 processedSampleCount, qint64 samplesToProcess) override;

    signals:
        void clippingDetected(qint64 position);

    private:
        DspxProjectAudioExporterPrivate *d;
        IAudioSampleContainer *m_buf{};

    };

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
        struct MixerSourceData {
            DspxTrackContext *trackContext;
            PositionableMixerAudioSource *source;
            int silentFlags;
            bool solo;
            int actualSilentFlags;
        };
        QList<MixerSourceData> savedMixerSourceDataList;
        int savedMixerSilentFlags;

        QHash<PositionableMixerAudioSource *, AbstractAudioFormatIO *> taskSources;
        QList<MixerSourceData>::const_iterator savedMixerSourceDataIt;

        void saveMixerState();
        void restoreMixerState();

        void makeMixedTaskMixerLayout();
        QPair<DspxTrackContext *, AbstractAudioFormatIO *> makeNextSeparatedThruMasterTaskMixerLayoutAndGetCorrespondingData();

        DspxProjectAudioExporter::Result executeMixedTask();
        DspxProjectAudioExporter::Result executeSeparatedTask();

        DspxProjectAudioExporter::Result executeTaskImpl(PositionableMixerAudioSource *src, DspxTrackContext *trackContext, AbstractAudioFormatIO *io);

        DspxProjectAudioExporterSourceWriter *currentWriter{};
        bool interruptionFlagIsFail{};

    };
}

#endif //TALCS_DSPXPROJECTAUDIOEXPORTER_P_H
