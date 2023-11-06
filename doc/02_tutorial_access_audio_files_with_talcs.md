# Tutorial: Access audio files with TALCS

TALCS  uses [libsndfile](https://libsndfile.github.io/libsndfile/) to read and write audio files in various codecs, and uses [r8brain](https://github.com/avaneev/r8brain-free-src) to convert between different sample rates.

[AudioFormatIO](@ref talcs::AudioFormatIO) can be used to read audio from and write audio to a [QIODevice](https://doc.qt.io/qt-5/qiodevice.html) object. Here is an example of how to read audio from a file:

```cpp
QFile audioFile1("path/to/audio/file.wav");
auto audioFormatIO1 = new talcs::AudioFormatIO(&audioFile1);
audioFormatIO1->open(QFile::ReadOnly);
auto data = new float[1024 * audioFormatIO1->channelCount()];
audioFormatIO1->read(data, 1024);
```

This example reads a part of audio data with a length of 1024 samples from a WAV file.

Similarly, Here is an example of how to write audio to a file:

```cpp
QFile audioFile2("path/to/another/audio/file.wav");
auto audioFormatIO2 = new talcs::AudioFormatIO(&audioFile2);
audioFormatIO2->open(
    QFile::WriteOnly,
    talcs::AudioFormatIO::WAV | talcs::AudioFormatIO::PCM_24,
    audioFormatIO1->channelCount(),
    44100);
audioFormatIO2->write(data, 1024);
```

[AudioFormatInputSource](@ref talcs::AudioFormatInputSource) is a type of [PositionableAudioSource](@ref talcs::PositionableAudioSource) that reads audio from an [AudioFormatIO](@ref talcs::AudioFormatIO) object. Here is an example of how to use it:

```cpp
QFile audioFile("path/to/audio/file.wav");
auto audioFormatIO = new talcs::AudioFormatIO(&audioFile);
auto audioFormatInputSource = new talcs::AudioFormatInputSource(audioFormatIO);
```

[AudioSourceWriter](@ref talcs::AudioSourceWriter) can be used to write the audio data produced by any [AudioSource](@ref talcs::AudioSource) objects to an [AudioFormatIO](@ref talcs::AudioFormatIO) object. Here is an example of how to use it:

```cpp
QFile audioFile("path/to/audio/file.wav");
auto audioFormatIO = new talcs::AudioFormatIO(&audioFile);
audioFormatIO->open(
    QFile::WriteOnly,
    talcs::AudioFormatIO::WAV | talcs::AudioFormatIO::PCM_24,
    2,
    44100);
auto src = new talcs::SineWaveAudioSource(440);
src->open(4096, 44100);
auto srcWriter = new talcs::AudioSourceWriter(
    src,
    audioFormatIO,
    44100 * 4);
srcWriter->start();
```

See the source code of the complete example program for a deeper understanding of how to access audio files with TALCS.

[Source code on GitHub](https://github.com/CrSjimo/talcs/blob/main/tests/AccessAudioFileTutorial/main.cpp)
