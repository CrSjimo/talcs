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

#ifndef TALCS_IMIXER_H
#define TALCS_IMIXER_H

#include <QList>

namespace talcs {

    template <class T>
    struct IMixer {
        virtual bool addSource(T *src, bool takeOwnership) = 0;
        virtual bool removeSource(T *src) = 0;
        virtual void removeAllSources() = 0;
        virtual QList<T *> sources() const = 0;
        virtual void setSourceSolo(T *src, bool isSolo) = 0;
        virtual bool isSourceSolo(T *src) const = 0;

        virtual void setGain(float gain) = 0;
        virtual float gain() const = 0;

        virtual void setPan(float pan) = 0;
        virtual float pan() const = 0;

        virtual void setRouteChannels(bool routeChannels) = 0;
        virtual bool routeChannels() const = 0;

        virtual void setSilentFlags(int silentFlags) = 0;
        virtual int silentFlags() const = 0;

        virtual void setLevelMeterChannelCount(int count) = 0;
        virtual int levelMeterChannelCount() = 0;
    };
    
}

#endif // TALCS_IMIXER_H
