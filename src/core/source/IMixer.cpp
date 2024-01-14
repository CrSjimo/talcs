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

#include "IMixer.h"

namespace talcs {

    /**
     * @interface IMixer
     * @brief Interface for objects that have a list of input sources and produces audio from them
     */

    /**
     * @fn bool IMixer::addSource(T *src, bool takeOwnership)
     * Adds an input source.
     * @param src the input source
     * @param takeOwnership If set to @c true, the object will be deleted on destruction.
     * @return @c true if success
     */

    /**
     * @fn bool IMixer::removeSource(T *src)
     * Removes an input source. The ownership of the removed object is no longer taken.
     * @return true if success
     */

    /**
     * @fn void IMixer::removeAllSources()
     * Removes all input sources. The ownerships of the removed objects are no longer taken.
     */

    /**
     * @fn QList<T *> IMixer::sources() const
     * Gets all input sources.
     */

    /**
     * @fn void IMixer::setSourceSolo(T *src, bool isSolo)
     * Sets an input source to be solo or not-solo.
     */

    /**
     * @fn bool IMixer::isSourceSolo(T *src) const
     * Gets whether an input source is solo or not.
     */

    /**
     * @fn void IMixer::setGain(float gain)
     * Sets the output gain.
     */

    /**
     * @fn float IMixer::gain() const
     * Gets the output gain.
     */

    /**
     * @fn void IMixer::setPan(float pan)
     * Sets the output pan.
     *
     * The value should be between -1 and 1. (unless you do want to use this feature to invert the phase of one of the channels)
     *
     * Specifically, the gain of the left channel will be multiplied by <tt>max(1, 1 - pan)</tt>, and the gain of the right channel will
     * be multiplied by <tt>max(1, 1 + pan)</tt>.
     */

    /**
     * @fn float IMixer::pan() const
     * Gets the output pan.
     */

    /**
     * @fn void IMixer::setRouteChannels(bool routeChannels)
     * Sets whether to route the input sources to output channels.
     *
     * If set to @c true, 2 channels will be read from each input source, and each 2 channels will be produced by combination.
     *
     * For example, if 4 channels are read from this object, and this object owns 3 input sources, the first and the second
     * channels are read from the first input source, the third and the fourth channels are read from the second source,
     * and the third source is not read.
     *
     * The order in which input sources are processed depends on the order in which they were added.
     */

    /**
     * @fn bool IMixer::routeChannels() const
     * Gets whether to route the input sources to output channels.
     */

    /**
     * @fn void IMixer::setSilentFlags(int silentFlags)
     * Sets the silent flags of the source.
     *
     * @see AudioSourceReadData::silentFlags
     */

    /**
     * @fn int IMixer::silentFlags() const
     * Gets the silent flags of the source.
     *
     * @see AudioSourceReadData::silentFlags
     */

    /**
     * @fn void IMixer::setLevelMeterChannelCount(int count)
     * Sets the number of channels to calculate the magnitude of each block. If the number exceeds the number of channels
     * in the produced audio block, excess part will be set to zero.
     *
     * @see MixerAudioSource::levelMetered(), PositionableMixerAudioSource::levelMetered()
     */

    /**
     * @fn int IMixer::levelMeterChannelCount() const
     * Gets the number of channels to calculate the magnitude of each block.
     *
     * @see MixerAudioSource::levelMetered(), PositionableMixerAudioSource::levelMetered()
     */
     
}