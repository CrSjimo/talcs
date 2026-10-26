# Tutorial: Using audio devices with TALCS

TALCS includes native `WASAPI` and `WASAPI (Exclusive Mode)` drivers through `IAudioClient3` on Windows and `CoreAudio` and `CoreAudio (Hog Mode)` drivers through the HAL Output Audio Unit on macOS. Linux uses [SDL2 Audio](https://wiki.libsdl.org/). All built-in drivers for the target platform are registered by `AudioDriverManager::createBuiltInDriverManager()`. The ASIO module provides `talcs::ASIOAudioDriver`, which requires registration with `AudioDriverManager::addAudioDriver()`.

The ASIO module requires `TALCS_ASIO=ON` and the `asiosdk` vcpkg package. Link applications to `talcs::Asio` and include `<TalcsAsio/ASIOAudioDriver.h>` to register the driver. The module is licensed under GPL-3.0-only.

Here is an example of how to get these built-in [AudioDriver](@ref talcs::AudioDriver) objects:

```cpp
auto mgr = talcs::AudioDriverManager::createBuiltInDriverManager();
qInfo() << "Built-in drivers:" << mgr->drivers();
auto wasapiDrv = mgr->driver("WASAPI");
```

This Windows example creates an [AudioDriverManager](@ref talcs::AudioDriverManager) with the built-in drivers, lists their names, and retrieves the shared driver as `WASAPI`. The exclusive driver is available as `WASAPI (Exclusive Mode)`. `AudioDriver::displayName()` returns the translated display name. Driver lookup and settings use `AudioDriver::name()`.

Here is an example of how to use the [AudioDriver](@ref talcs::AudioDriver) object:

```cpp
if (!wasapiDrv || !wasapiDrv->initialize()) {
    qCritical() << "Cannot initialize WASAPI driver!";
    return 1;
}
qInfo() << "WASAPI devices:" << wasapiDrv->devices();
qInfo() << "WASAPI default device:" << wasapiDrv->defaultDevice();
const auto defaultDeviceName = wasapiDrv->defaultDevice();
auto dev = defaultDeviceName.isEmpty() ? nullptr : wasapiDrv->createDevice(defaultDeviceName);
```

This example initializes the WASAPI audio driver, lists the names of all devices as well as the default device, and creates the [AudioDevice](@ref talcs::AudioDevice) object of the default device.

`defaultDevice()` returns the name of the current default physical endpoint. `createDevice()` creates a device from that name. The device remains bound to that endpoint if the system default changes. `availableSampleRates()` and `availableBufferSizes()` provide independent capability lists. `open()` returns `false` and sets `errorString()` if the requested combination is unsupported. Exclusive mode can align the requested buffer size to the hardware requirements. The actual buffer size is available through `bufferSize()` after `open()` succeeds.

Here is an example of how to use the [AudioDevice](@ref talcs::AudioDevice) object:

```cpp
if (!dev || !dev->isInitialized()) {
    qCritical() << "Cannot create WASAPI audio device!";
    return 1;
}
qInfo() << "Preferred buffer size:" << dev->preferredBufferSize();
qInfo() << "Preferred sample rate:" << dev->preferredSampleRate();
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
