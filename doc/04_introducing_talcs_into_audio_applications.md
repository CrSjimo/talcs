# Introducing TALCS into Audio Applications

![TALCS in DiffScope](https://raw.githubusercontent.com/CrSjimo/talcs/main/doc/talcs_in_diffscope.svg)

## Dealing with drivers and devices

The static member function [AudioDriverManager::createBuiltInDriverManager](@ref talcs::AudioDriverManager::createBuiltInDriverManager())() can be used to create an [AudioDriverManager](@ref talcs::AudioDriverManager) that supports the built-in SDL drivers. After the creation of the built-in audio manager, list all driver modes for the user to select. Then,  get the specified [AudioDriver](@ref talcs::AudioDriver) object.

Only after the initialization can audio devices be listed and created. Once an [AudioDevice](@ref talcs::AudioDevice) object is created, the information about supported buffer sizes and sample rates can be used to configure the audio application and open the device.

To start the device, you need an [AudioDevicePlayback](@ref AudioDevicePlayback) object to produce audio blocks to feed the device. In practice, [AudioSourcePlayback](@ref talcs::AudioSourcePlayback) is usually used, which feed the device with the audio block produced by an [AudioSource](@ref AudioSource) on each callback.

## Combining different components within a project scope

The [AudioSource](@ref AudioSource) object that directly connects to the device is usually [MixerAudioSource](@ref MixerAudioSource) in practice, because it mixes multiple audio sources from different project scopes. And within a single project scope, a [TransportAudioSource](@ref TransportAudioSource) object is usually directly connects to the [MixerAudioSource](@ref MixerAudioSource).
