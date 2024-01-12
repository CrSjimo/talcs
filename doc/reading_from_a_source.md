# Reading from a Source

[AudioSource::read](@ref talcs::AudioSource::read())() is the main feature of [AudioSource](@ref talcs::AudioSource) objects. This method has one parameter typed [AudioSourceReadData](@ref talcs::AudioSourceReadData). An [AudioSourceReadData](@ref talcs::AudioSourceReadData) struct has 4 properties: `buffer`, `startPos`, `length`, and `silentFlags`. The return value of this method indicates the length actually read.

`buffer` is an [IAudioSampleContainer](@ref talcs::IAudioSampleContainer) where the audio data read will be put into. `startPos` and `length` specify the affected range of the `buffer`. If the `buffer` is not empty, the contents outside the specified range will be not modified. Note that Even if the return value is less than `length`, the range between the return value and `length` may also be modified.

It is specified by [IAudioSampleProvider::channelCount](@ref talcs::IAudioSampleProvider::channelCount())() of the `buffer` how many channels are required to read. If the number of channels provided by the source from which the audio is read exceeds the number of channels required to read, then the excess channels provided by the source should be discarded. If it is the opposite situation, then the excess channels of the `buffer` should be set to zero.

`silentFlags` specifies which channels are not needed by the reader bitwisely (i.e. the LSB is the first channel, and the MSB is the 32nd channel). This could be used by the source to skip some procedures. `silentFlags` is not a mandatory requirement. It is optional. The source can skip the channels not needed, as well as read these channels anyway. Note that the skipped procedures should not affect the continuity of the audio.

For [PositionableAudioSource](@ref talcs::PositionableAudioSource) objects, the return value always equals the required `length` unless exceeding the [PositionableAudioSource::length](@ref talcs::PositionableAudioSource::length())().


