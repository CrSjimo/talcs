#include "AudioSource.h"
#include "AudioSource_p.h"

namespace talcs {

    /**
     * @struct AudioSourceReadData
     * @brief The object that contains the target for AudioSource to fill data in
     *
     * Note that the "read" in the class name is a past participle, which means the data is "read" from an AudioSource
     * object.
     * @see AudioSource, IAudioSampleContainer
     * @var AudioSourceReadData::buffer
     * The IAudioSampleContainer object used
     *
     * @var AudioSourceReadData::startPos
     * The start position of the specified range
     *
     * @var AudioSourceReadData::length
     * The length of the specified range
     *
     * @var AudioSourceReadData::silentFlags
     * Bitwise flags of whether a specified channel is silent
     */

    /**
     * Implicit constructor. The data is filled from the start of the IAudioSampleContainer object and the length read
     * is to be the number of samples of it.
     */
    AudioSourceReadData::AudioSourceReadData(IAudioSampleContainer *buffer)
        : AudioSourceReadData(buffer, 0, buffer->sampleCount()) {
    }

    /**
     * The data is filled into a specified range in the IAudioSampleContainer object.
     */
    AudioSourceReadData::AudioSourceReadData(IAudioSampleContainer *buffer, qint64 startPos, qint64 length, int silentFlags)
        : buffer(buffer), startPos(startPos), length(length), silentFlags(silentFlags) {
    }

    /**
     * @class AudioSource
     * @brief Base class for sources from which audio data can be streamly read
     */

    /**
     * Default constructor.
     */
    AudioSource::AudioSource() : AudioSource(*new AudioSourcePrivate) {
    }

    AudioSource::AudioSource(AudioSourcePrivate &d) : d_ptr(&d) {
        d.q_ptr = this;
    }
    AudioSource::~AudioSource() = default;

    /**
     * @fn qint64 AudioSource::read(const AudioSourceReadData &readData)
     * Reads audio data from the source.
     * @param readData see docs in class AudioSourceReadData
     * @returns the actual length of audio read measured in samples
     */
     
}
