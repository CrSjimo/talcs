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

#ifndef TALCS_AUDIOSOURCEWRITER_H
#define TALCS_AUDIOSOURCEWRITER_H

#include <TalcsCore/AudioSourceProcessorBase.h>
#include <TalcsFormat/TalcsFormatGlobal.h>

namespace talcs {

    class AudioSourceWriterPrivate;
    class AudioSource;
    class AbstractAudioFormatIO;

    class TALCSFORMAT_EXPORT AudioSourceWriter : public AudioSourceProcessorBase {
        Q_OBJECT
        Q_DECLARE_PRIVATE(AudioSourceWriter)
    public:
        AudioSourceWriter(AudioSource *src, AbstractAudioFormatIO *outFile, qint64 length, QObject *parent = nullptr);
        ~AudioSourceWriter() override;

    protected:
        IAudioSampleContainer *prepareBuffer() override;
        bool processBlock(qint64 processedSampleCount, qint64 samplesToProcess) override;
        void processWillFinish() override;
    };
    
}



#endif // TALCS_AUDIOSOURCEWRITER_H
