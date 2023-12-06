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

#ifndef TALCS_POSITIONABLEAUDIOSOURCE_H
#define TALCS_POSITIONABLEAUDIOSOURCE_H

#include <TalcsCore/AudioSource.h>

namespace talcs {

    class PositionableAudioSourcePrivate;

    class TALCSCORE_EXPORT PositionableAudioSource : public AudioSource {
        Q_DECLARE_PRIVATE(PositionableAudioSource)
    public:
        PositionableAudioSource();
        ~PositionableAudioSource() override;
        virtual qint64 length() const = 0;
        virtual qint64 nextReadPosition() const;
        virtual void setNextReadPosition(qint64 pos);

    protected:
        explicit PositionableAudioSource(PositionableAudioSourcePrivate &d);
    };

    class PositionableAudioSourceStateSaverPrivate;

    class TALCSCORE_EXPORT PositionableAudioSourceStateSaver {
    public:
        explicit PositionableAudioSourceStateSaver(PositionableAudioSource *src);
        ~PositionableAudioSourceStateSaver();

    private:
        QScopedPointer<PositionableAudioSourceStateSaverPrivate> d;
    };
    
}

#endif // TALCS_POSITIONABLEAUDIOSOURCE_H
