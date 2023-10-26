# Tutorial: Using audio devices with TALCS

TALCS has built-in support for [SDL2 Audio](https://wiki.libsdl.org/) and  [ASIO](https://forums.steinberg.net/c/developer/asio/) (only Windows). 

Due to license issues, the support for ASIO is optional. If you want to use it, you should enable the `TALCS_ENABLE_ASIO` option and specify the path to [ASIO SDK](https://www.steinberg.net/asiosdk) by setting the CMake variable `TALCS_ASIOSDK_DIR`.

Here is an example of how to get these built-in [AudioDriver](@ref talcs::AudioDriver) objects:

```cpp
auto mgr = talcs::AudioDriverManager::createBuiltInDriverManager();
qInfo() << "Built-in drivers:" << mgr->drivers();
auto wasapiDrv = mgr->driver("wasapi");
```

This example creates an [AudioDriverManager](@ref talcs::AudioDriverManager) with all built-in driver modes, lists the names of all driver modes, and creates a WASAPI audio driver. You can refer to the documentation of SDL2 to find all supported driver modes.

Here is an example of how to use the [AudioDriver](@ref talcs::AudioDriver) object:

```cpp
if (!wasapiDrv->initialize()) {
    qCritical() << "Cannot initialize WASAPI driver!";
    return 1;
}
qInfo() << "WASAPI devices:" << wasapiDrv->devices();
qInfo() << "WASAPI default device:" << wasapiDrv->defaultDevice();
auto dev = wasapiDrv->createDevice(wasapiDrv->defaultDevice());
```

This example initializes the WASAPI audio driver, lists the names of all devices as well as the default device, and creates the [AudioDevice](@ref talcs::AudioDevice) object of the default device.

Here is an example of how to use the [AudioDevice](@ref talcs::AudioDevice) object:

```cpp
if (!dev || !dev->isInitialized()) {
    qCritical() << "Cannot create WASAPI audio device!";
    return 1;
}
qInfo() << "Preferred buffer size:" << dev->preferredBufferSize();
qqInfo) << "Preferred sample rate:" << dev->preferredSampleRate();
if (!dev->open(dev->preferredBufferSize(), dev->preferredSampleRate())) {
    qCritical() << "Cannot open WASAPI audio device!";
    return 1;
}
auto src = new talcs::SineWaveAudioSource(440);
auto srcPlayback = new talcs::AudioSourcePlayback(src);
if (!dev->start(srcPlayback)) {
    qCritical() << "Cannot start playing with WASAPI audio device!";
    return 1;
}
```

Firstly, it is necessary to check whether the audio device is valid. If any error occurs when creating the [AudioDevice](@ref talcs::AudioDevice) object, the pointer will be `nullptr`; if any error occurs when the object is initializing, `AudioDriver::isInitialized()` will return `false`. 

Then, the example outputs the preferred buffer size and sample rate of the audio device and opens the device with them.

After the device is opened, the example creates a [SineWaveAudioSource](@ref talcs::SineWaveAudioSource) to produce a 440 Hz sine wave, and then creates an [AudioSourcePlayback](@ref talcs::AudioSourcePlayback) that uses the source, for the audio device to play it.

After these procedures, the example starts the audio device. It will play a 440 Hz sine wave continuously.

[Source code on GitHub](https://github.com/CrSjimo/talcs/blob/main/tests/AudioDriverTutorial/main.cpp)
