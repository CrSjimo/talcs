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

#include "DspxProjectAudioExporter.h"
#include "DspxProjectAudioExporter_p.h"

#include <QSet>
#include <QEventLoop>
#include <QThreadPool>
#include <QThread>

#include <TalcsCore/TransportAudioSource.h>
#include <TalcsCore/PositionableMixerAudioSource.h>

#include <TalcsDspx/DspxProjectContext.h>
#include <TalcsDspx/DspxTrackContext.h>

namespace talcs {
    DspxProjectAudioExporterSourceWriter::DspxProjectAudioExporterSourceWriter(DspxProjectAudioExporterPrivate *d, DspxTrackContext *trackContext, AudioSource *src, AbstractAudioFormatIO *outFile, int channelCountToMonoize, qint64 length)
        : AudioSourceWriter(src, outFile, channelCountToMonoize, length), d(d) {
    }

    bool DspxProjectAudioExporterSourceWriter::processBlock(qint64 processedSampleCount, qint64 samplesToProcess) {
        if (d->isClippingCheckEnabled) {
            for (int ch = 0; ch < m_buf->channelCount(); ch++) {
                if (m_buf->magnitude(ch) > 1.0) {
                    emit clippingDetected(processedSampleCount);
                    break;
                }
            }
        }
        return AudioSourceWriter::processBlock(processedSampleCount, samplesToProcess);
    }

    void DspxProjectAudioExporterPrivate::saveMixerState() {
        savedMixerPosition = projectContext->transport()->position();
        savedMixerSilentFlags = projectContext->masterControlMixer()->silentFlags();
        auto masterTrack = projectContext->masterTrackMixer();
        for (auto trackContext : projectContext->tracks()) {
            savedMixerSourceDataList.append({
                trackContext,
                trackContext->controlMixer(),
                trackContext->controlMixer()->silentFlags(),
                masterTrack->isSourceSolo(trackContext->controlMixer()),
                masterTrack->isMutedBySoloSetting(trackContext->controlMixer()) ? -1 : trackContext->controlMixer()->silentFlags(),
            });
        }
    }

    void DspxProjectAudioExporterPrivate::restoreMixerState() {
        projectContext->masterTrackMixer()->removeAllSources();
        auto masterTrack = projectContext->masterTrackMixer();
        for (const auto &mixerSourceData : savedMixerSourceDataList) {
            masterTrack->addSource(mixerSourceData.source);
            masterTrack->setSourceSolo(mixerSourceData.source, mixerSourceData.solo);
            mixerSourceData.source->setSilentFlags(mixerSourceData.silentFlags);
        }
        projectContext->masterControlMixer()->setSilentFlags(savedMixerSilentFlags);
        projectContext->transport()->setPosition(savedMixerPosition);

        savedMixerSourceDataList.clear();
        savedMixerSilentFlags = 0;
        savedMixerPosition = 0;

        taskSources.clear();
    }

    void DspxProjectAudioExporterPrivate::makeMixedTaskMixerLayout() {
        if (!isMuteSoloEnabled) {
            projectContext->masterControlMixer()->setSilentFlags(0);
        }
        auto masterTrack = projectContext->masterTrackMixer();
        masterTrack->removeAllSources();
        for (auto trackContext : mixedTask) {
            taskSources.insert(trackContext->controlMixer(), nullptr);
        }
        for (const auto &mixerSourceData : savedMixerSourceDataList) {
            if (!taskSources.contains(mixerSourceData.source))
                continue;
            masterTrack->addSource(mixerSourceData.source);
            if (!isMuteSoloEnabled) {
                mixerSourceData.source->setSilentFlags(0);
            } else {
                mixerSourceData.source->setSilentFlags(mixerSourceData.actualSilentFlags);
            }
        }
        projectContext->transport()->setPosition(projectContext->timeConverter()(startTick));
    }

    QPair<DspxTrackContext *, AbstractAudioFormatIO *> DspxProjectAudioExporterPrivate::makeNextSeparatedThruMasterTaskMixerLayoutAndGetCorrespondingData() {
        if (!isMuteSoloEnabled) {
            projectContext->masterControlMixer()->setSilentFlags(0);
        }
        auto masterTrack = projectContext->masterTrackMixer();
        masterTrack->removeAllSources();
        if (taskSources.isEmpty()) {
            for (const auto &[trackContext, io] : separatedTasks) {
                taskSources.insert(trackContext->controlMixer(), io);
            }
        }
        for (;savedMixerSourceDataIt != savedMixerSourceDataList.cend(); savedMixerSourceDataIt++) {
            auto &mixerSourceData = *savedMixerSourceDataIt;
            if (!taskSources.contains(mixerSourceData.source))
                continue;
            masterTrack->addSource(mixerSourceData.source);
            if (!isMuteSoloEnabled) {
                mixerSourceData.source->setSilentFlags(0);
            } else {
                mixerSourceData.source->setSilentFlags(mixerSourceData.actualSilentFlags);
            }
            savedMixerSourceDataIt++;
            masterTrack->setNextReadPosition(projectContext->timeConverter()(startTick));
            return {mixerSourceData.trackContext, taskSources.value(mixerSourceData.source)};
        }
        return {nullptr, nullptr};
    }

    DspxProjectAudioExporter::DspxProjectAudioExporter(DspxProjectContext *context, QObject *parent) : QObject(parent), d_ptr(new DspxProjectAudioExporterPrivate) {
        Q_D(DspxProjectAudioExporter);
        d->q_ptr = this;
        d->projectContext = context;
    }

    DspxProjectAudioExporter::~DspxProjectAudioExporter() {

    }

    void DspxProjectAudioExporter::setMuteSoloEnabled(bool v) {
        Q_D(DspxProjectAudioExporter);
        d->isMuteSoloEnabled = v;
    }

    bool DspxProjectAudioExporter::isMuteSoloEnabled() const {
        Q_D(const DspxProjectAudioExporter);
        return d->isMuteSoloEnabled;
    }

    void DspxProjectAudioExporter::setRange(int startTick, int lengthTick) {
        Q_D(DspxProjectAudioExporter);
        d->startTick = startTick;
        d->lengthTick = lengthTick;
    }

    int DspxProjectAudioExporter::start() const {
        Q_D(const DspxProjectAudioExporter);
        return d->startTick;
    }

    int DspxProjectAudioExporter::length() const {
        Q_D(const DspxProjectAudioExporter);
        return d->lengthTick;
    }

    void DspxProjectAudioExporter::setThruMaster(bool v) {
        Q_D(DspxProjectAudioExporter);
        d->thruMaster = v;
    }

    bool DspxProjectAudioExporter::thruMaster() const {
        Q_D(const DspxProjectAudioExporter);
        return d->thruMaster;
    }
    void DspxProjectAudioExporter::setMonoChannel(bool v) {
        Q_D(DspxProjectAudioExporter);
        d->isMonoChannel = v;
    }
    bool DspxProjectAudioExporter::isMonoChannel() const {
        Q_D(const DspxProjectAudioExporter);
        return d->isMonoChannel;
    }

    void DspxProjectAudioExporter::addSeparatedTask(DspxTrackContext *track, AbstractAudioFormatIO *outFile) {
        Q_D(DspxProjectAudioExporter);
        d->separatedTasks.append({track, outFile});
    }

    void DspxProjectAudioExporter::setMixedTask(const QList<DspxTrackContext *> &tracks, AbstractAudioFormatIO *outFile) {
        Q_D(DspxProjectAudioExporter);
        d->mixedTask = tracks;
        d->mixedTaskOutFile = outFile;
    }

    void DspxProjectAudioExporter::clearTask() {
        Q_D(DspxProjectAudioExporter);
        d->separatedTasks.clear();
        d->mixedTask.clear();
        d->mixedTaskOutFile = nullptr;
    }

    void DspxProjectAudioExporter::setClippingCheckEnabled(bool v) {
        Q_D(DspxProjectAudioExporter);
        d->isClippingCheckEnabled = v;
    }

    bool DspxProjectAudioExporter::isClippingCheckEnabled() const {
        Q_D(const DspxProjectAudioExporter);
        return d->isClippingCheckEnabled;
    }

    DspxProjectAudioExporter::Result DspxProjectAudioExporterPrivate::executeMixedTask() {
        Q_Q(DspxProjectAudioExporter);

        saveMixerState();

        makeMixedTaskMixerLayout();
        auto ret = executeTaskImpl(projectContext->masterControlMixer(), nullptr, mixedTaskOutFile);

        restoreMixerState();
        return ret;
    }

    DspxProjectAudioExporter::Result DspxProjectAudioExporterPrivate::executeSeparatedTask() {
        Q_Q(DspxProjectAudioExporter);

        saveMixerState();

        DspxProjectAudioExporter::Result ret = DspxProjectAudioExporter::OK;

        savedMixerSourceDataIt = savedMixerSourceDataList.cbegin();
        while (true) {
            auto [trackContext, io] = makeNextSeparatedThruMasterTaskMixerLayoutAndGetCorrespondingData();
            if (!trackContext)
                break;
            ret = executeTaskImpl(thruMaster ? projectContext->masterControlMixer() : trackContext->controlMixer(), trackContext, io);
            if (ret != DspxProjectAudioExporter::OK)
                break;
        }

        restoreMixerState();
        return ret;
    }

    DspxProjectAudioExporter::Result DspxProjectAudioExporterPrivate::executeTaskImpl(PositionableMixerAudioSource *src, DspxTrackContext *trackContext, AbstractAudioFormatIO *io) {
        Q_Q(DspxProjectAudioExporter);

        auto convertTime = projectContext->timeConverter();
        auto length = convertTime(startTick + lengthTick) - convertTime(startTick);

        QEventLoop eventLoop;
        QThread exportThread;

        DspxProjectAudioExporterSourceWriter writer(this, trackContext, src, io, isMonoChannel ? 2 : 0, length);
        auto cleanup = [=](void *) {currentWriter = nullptr;};
        std::unique_ptr<void, decltype(cleanup)> _(nullptr, cleanup);
        currentWriter = &writer;
        writer.moveToThread(&exportThread);
        QObject::connect(&exportThread, &QThread::started, &writer, &AudioSourceProcessorBase::start);

        bool clippingFlag = false;
        QObject::connect(&writer, &DspxProjectAudioExporterSourceWriter::clippingDetected, q, [&] {
            if (!clippingFlag) {
                clippingFlag = true;
                emit q->clippingDetected(trackContext);
            }
        });
        QObject::connect(&writer, &AudioSourceProcessorBase::blockProcessed, q, [&](qint64 processedSampleCount) {
            emit q->progressChanged(1.0 * processedSampleCount / length, trackContext);
        });
        QObject::connect(&writer, &AudioSourceProcessorBase::finished, q, [&] {
            if (writer.status() == AudioSourceProcessorBase::Completed) {
                eventLoop.exit(DspxProjectAudioExporter::OK);
            } else if (writer.status() == AudioSourceProcessorBase::Failed) {
                eventLoop.exit(DspxProjectAudioExporter::Fail);
            } else if (writer.status() == AudioSourceProcessorBase::Interrupted) {
                eventLoop.exit(interruptionFlagIsFail ? DspxProjectAudioExporter::Fail : DspxProjectAudioExporter::Interrupted);
            }
        });
        exportThread.start();
        DspxProjectAudioExporter::Result ret(eventLoop.exec());
        exportThread.quit();
        exportThread.wait();
        return ret;
    }

    DspxProjectAudioExporter::Result DspxProjectAudioExporter::exec() {
        Q_D(DspxProjectAudioExporter);
        if (d->separatedTasks.isEmpty() && d->mixedTask.isEmpty() && !d->mixedTaskOutFile)
            return OK;
        if (!d->separatedTasks.isEmpty() && (!d->mixedTask.isEmpty() || d->mixedTaskOutFile)) {
            Q_UNREACHABLE();
        } else if (!d->separatedTasks.isEmpty()) {
            return d->executeSeparatedTask();
        } else {
            return d->executeMixedTask();
        }
    }

    void DspxProjectAudioExporter::interrupt(bool isFail) {
        Q_D(DspxProjectAudioExporter);
        d->interruptionFlagIsFail = isFail;
        if (d->currentWriter)
            d->currentWriter->interrupt();
    }
}
