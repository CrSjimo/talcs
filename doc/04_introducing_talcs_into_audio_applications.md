# Introducing TALCS into Audio Applications

![TALCS in DiffScope](https://raw.githubusercontent.com/CrSjimo/talcs/main/doc/talcs_in_diffscope.svg)

## Dealing with drivers and devices

The static member function [AudioDriverManager::createBuiltInDriverManager](@ref talcs::AudioDriverManager::createBuiltInDriverManager())() can be used to create an [AudioDriverManager](@ref talcs::AudioDriverManager) that supports the built-in SDL drivers. After the creation of the built-in audio manager, list all driver modes for the user to select. Then,  get the specified [AudioDriver](@ref talcs::AudioDriver) object.

Only after the initialization can audio devices be listed and created. Once an [AudioDevice](@ref talcs::AudioDevice) object is created, the information about supported buffer sizes and sample rates can be used to configure the audio application and open the device.

To start the device, you need an [AudioDevicePlayback](@ref AudioDevicePlayback) object to produce audio blocks to feed the device. In practice, [AudioSourcePlayback](@ref talcs::AudioSourcePlayback) is usually used, which feeds the device with the audio block produced by an [AudioSource](@ref AudioSource) on each callback.

## Combining different components within a project scope

The [AudioSource](@ref AudioSource) object that directly connects to the device is usually [MixerAudioSource](@ref MixerAudioSource) in practice, which mixes multiple audio sources from different project scopes. Within a single project scope, a [TransportAudioSource](@ref TransportAudioSource) object is usually directly connected to the [MixerAudioSource](@ref MixerAudioSource). Then, all successor sources are [PositionableAudioSource](@ref talcs::PositionableAudioSource).

First, let's dive deeper into [IMixer](@ref talcs::PositionableMixerAudioSource), the interface that [MixerAudioSource](@ref talcs::MixerAudioSource) and [PositionableMixerAudioSource](@ref talcs::PositionableMixerAudioSource) implements. The [IMixer](@ref talcs::IMixer) object reads from a list of audio sources, "mixes" these audio blocks to one, applies gain and pan to it, produces it out, and meanwhile signals the magnitude of the audio block processed. For the quoted term "mix", it could either be "mix together each channel in all input sources" (when [IMixer::routeChannels](@ref talcs::IMixer::routeChannels)() == false) or "arrange all channels in each source one by one" (when [IMixer::routeChannels](@ref talcs::IMixer::routeChannels)() == true).

![IMixer Diagram](https://raw.githubusercontent.com/CrSjimo/talcs/main/doc/imixer.svg)

[IMixer](@ref talcs::IMixer) can also set an input source to solo (using [IMixer::setSourceSolo](talcs::IMixer::setSourceSolo())()) or mute the output (using [IMixer::setSilentFlags](@ref talcs::IMixer::setSilentFlags())()). By combining [PositionableMixerAudioSource](@ref talcs::PositionableMixerAudioSource) objects within a project scope, you can create the track layout.

## Arranging Clips in a Track

A TALCS audio application is supposed to have two types of audio clips: the audio clip that immediately produces audio, and the audio clip that produces audio after loading. Currently, TALCS does not support the audio clip that streamingly loads audio and produces it.

For the first one, [AudioSourceClipSeries](@ref talcs::AudioSourceClipSeries) is used. If the track is an audio track, add [AudioFormatInputSource](@ref talcs::AudioFormatInputSource) clips to the series. The [AudioFormatInputSource](@ref AudioFormatInputSource) object reads audio from [AudioFormatIO](@ref AudioFormatIO) and produces it. Note that compared with other object connections, one [AudioFormatIO](@ref talcs::AudioFormatIO) can be set to multiple [AudioFormatInputSource](@ref talcs::AudioFormatInputSource) objects.

For the second one, [FutureAudioSourceClipSeries](@ref talcs::FutureAudioSourceClipSeries) is used. A [FutureAudioSourceClipSeries](@ref talcs::FutureAudioSourceClipSeries) series takes multiple [FutureAudioSource](@talcs::FutureAudioSource) clips as input. In practice, these sources are manipulated by the synthesis engine using [QFuture](https://doc.qt.io/qt-5/qfuture.html) APIs.