# Tutorial: Combining Audio Clips and Mixing Multi-track Audio with TALCS

TALCS can be used to combine several audio clips and mix multi-track audio.

[AudioSourceClipSeries](@ref talcs::AudioSourceClipSeries) is a type of [PositionableAudioSource](@ref talcs::PositionableAudioSource) that takes several AudioSourceClip (a. k. a [AudioClipBase](@ref talcs::AudioClipBase)< [PositionableAudioSource](@ref talcs::PositionableAudioSource) >), each of which takes a [PositionableAudioSource](@ref talcs::PositionableAudioSource). These clips are sequentially combined together and produced by the [AudioSourceClipSeries](@ref talcs::AudioSourceClipSeries).

Here is an example of how to use [AudioSourceClipSeries](@ref talcs::AudioSourceClipSeries):

```cpp
auto audioSourceClipSeries = new talcs::AudioSourceClipSeries;
auto src1 = new talcs::SineWaveAudioSource(440);
audioSourceClipSeries->addClip({
    .position = 0,
    .content = src1,
    .startPos = 0,
    .length = 44100 * 2,
});
auto src2 = new talcs::SineWaveAudioSource(880);
audioSourceClipSeries->addClip({
    .position = 44100 * 4,
    .content = src2,
    .startPos = 0,
    .length = 44100 * 2,
});
```

This example creates two audio clips. The first one is a 440 Hz sine wave and the second one is an 880 Hz sine wave. The start position of the first clip is 0 and the start position of the second clip is 44100 * 4. The length of each clip is 44100 * 2.

Please note that overlapping clips are not allowed to be added to the series. In this case, [AudioSourceClipSeries::addClip()](@ref talcs::AudioSourceClipSeries::addClip()) returns `false`.

[IMixer](@ref talcs::IMixer) is an interface that mixes several tracks together. There are 2 classes that implement it: [MixerAudioSource](@ref talcs::MixerAudioSource) (which takes [AudioSource](@ref talcs::AudioSource)) and [PositionableMixerAudioSource](@ref talcs::PositionableAudioSource) (which takes [PositionableAudioSource](@ref talcs::PositionableAudioSource)). 

Here is an example of how to use [PositionableMixerAudioSource](@ref talcs::PositionableAudioSource):

```cpp
auto mixer = new talcs::PositionableMixerAudioSource;
auto src3 = new talcs::SineWaveAudioSource(1320);
mixer->addSource(audioSourceClipSeries);
mixer->addSource(src3);
mixer->setGain(0.5);
mixer->setPan(0.5);
```

The example creates a mixer and mixes the [AudioSourceClipSeries](@ref talcs::AudioSourceClipSeries) object created above and a 1320 Hz sine wave source together. Then the example sets the output gain to 0.5 (approx. -3 dB) and the output pan to 0.5.

[IMixer](@ref talcs::IMixer) can also be used to route the input sources to output channels. To use this feature, call [IMixer::setRouteChannels()](@ref talcs::IMixer::setRouteChannels()) and set it to `true` For example, if 4 channels are read from this object, and this object owns 3 input sources, the first and the second channels are read from the first input source, the third and the fourth channels are read from the second source, and the third source is not read. The order in which input sources are processed depends on the order in which they were added.

Now, let's dive deeper and have a look at a more complicated example. First, we define a schema of JSON that contains the synthesis information. `audio` property represents the index of audio file in `audioFiles`, and `pos`, `start`, `length` are all measured in seconds.

```json
{
    "audioFiles": [
        "path/to/the/first/audio/file.flac",
        "path/to/the/second/audio/file.mp3"
    ],
    "tracks": [
        {
            "gain": 0.5,
            "pan": 0.5,
            "clips": [
                {
                    "audio": 1,
                    "pos": 0,
                    "start": 1.14,
                    "length": 5.14
                },
                {
                    "audio": 0,
                    "pos": 19.19,
                    "start": 8.10,
                    "length": 19.26
                },
                {
                    "audio": 1,
                    "pos": 11.45,
                    "start": 14,
                    "length": 7
                }
            ]
        },
        {
            "gain": 0.5,
            "pan": 0.0,
            "clips": [
                {
                    "audio": 0,
                    "pos": 0,
                    "start": 0,
                    "length": 100
                }
            ]
        }
    ]
}
```

Then, parse the JSON file and create the layout of clip series and tracks:

```cpp
using namespace talcs;
QList<QFile *> srcFileList;
QList<AudioFormatIO *> srcIoList;
QList<AudioFormatInputSource *> srcList;
QList<PositionableMixerAudioSource *> trackSrcList;
PositionableMixerAudioSource mixer;
qint64 effectiveLength = 0;
QFile f("path/to/synth/file.json");
f.open(QFile::ReadOnly);
auto doc = QJsonDocument::fromJson(f.readAll());
for(const auto &audioFileNameJsonVal: doc.object().value("audioFiles").toArray()) {
    auto audioFile = new QFile(audioFileNameJsonVal.toString());
    qDebug() << audioFileNameJsonVal.toString();
    srcFileList.append(audioFile);
    srcIoList.append(new AudioFormatIO(audioFile));
}
for(const auto &trackSpec: doc.object().value("tracks").toArray()) {
    auto clipSeries = new AudioSourceClipSeries;
    auto trackSrc = new PositionableMixerAudioSource;
    trackSrc->addSource(clipSeries, true);
    trackSrcList.append(trackSrc);
    trackSrc->setGain(trackSpec.toObject().value("gain").toDouble(1.0));
    trackSrc->setPan(trackSpec.toObject().value("pan").toDouble(0.0));
    for(const auto &clipSpec: trackSpec.toObject().value("clips").toArray()) {
        auto audioIndex = clipSpec.toObject().value("audio").toInt();
        auto positionSec = clipSpec.toObject().value("pos").toDouble();
        auto startPosSec = clipSpec.toObject().value("start").toDouble();
        auto lengthSec = clipSpec.toObject().value("length").toDouble();
        qint64 position = positionSec * device->sampleRate();
        qint64 startPos = startPosSec * device->sampleRate();
        qint64 length = lengthSec * device->sampleRate();
        auto src = new AudioFormatInputSource(srcIoList[audioIndex]);
        srcList.append(src);
        if(!clipSeries->addClip({position, src, startPos, length})) {
            QMessageBox::critical(&mainWindow, "Mixer", "Cannot add clip.");
        }
    }
    effectiveLength = std::max(effectiveLength, clipSeries->effectiveLength());
    mixer.addSource(trackSrc);
}
```

Finally, play the audio to the device:

```cpp
AudioDevice *device;
/* create and initialize the device */
TransportAudioSource tpSrc;
tpSrc.setSource(&mixer);
AudioSourcePlayback playback(&tpSrc);
tpSrc.play();
device->start(&playback);
```
