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

namespace talcs {
    DspxProjectAudioExporter::DspxProjectAudioExporter(DspxProjectContext *context, QObject *parent) : QObject(parent), d_ptr(new DspxProjectAudioExporterPrivate) {
        Q_D(DspxProjectAudioExporter);
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

    DspxProjectAudioExporter::Result DspxProjectAudioExporter::exec(int threadCount) {
        Q_D(DspxProjectAudioExporter);
        if (d->separatedTasks.isEmpty() && d->mixedTask.isEmpty() && !d->mixedTaskOutFile)
            return OK;
        if (!d->separatedTasks.isEmpty() && (!d->mixedTask.isEmpty() || d->mixedTaskOutFile)) {
            Q_UNREACHABLE();
        } else if (!d->separatedTasks.isEmpty()) {

        } else {

        }
        return OK;
    }

    void DspxProjectAudioExporter::interrupt() {

    }
}