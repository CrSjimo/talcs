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

#include "AudioVisualizer.h"

#include <QPlainTextEdit>

#include <TalcsCore/PositionableAudioSource.h>

AudioVisualizer::AudioVisualizer(talcs::PositionableAudioSource *src, QPlainTextEdit *edit, QObject *parent) : talcs::AudioSourceProcessorBase(src, src->length(), parent), m_edit(edit) {

}

talcs::IAudioSampleContainer *AudioVisualizer::prepareBuffer() {
    m_buf.resize(1, source()->bufferSize());
    return &m_buf;
}

bool AudioVisualizer::processBlock(qint64 processedSampleCount, qint64 samplesToProcess) {
    auto minMax = m_buf.findMinMax(0);
    // 处理峰值数据
    // 在这个样例中，会把峰值数据输出到文本框里，实际应用时这个地方可以和可视化widget对接
    // 然后这个函数运行在另一个线程上，需要用queued connection
    QMetaObject::invokeMethod(m_edit, "appendPlainText", Qt::QueuedConnection, Q_ARG(QString, QString("%1 %2").arg(minMax.first).arg(minMax.second)));
    return true;
}

void AudioVisualizer::processWillFinish() {

}
