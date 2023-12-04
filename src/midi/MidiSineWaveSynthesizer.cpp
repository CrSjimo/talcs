#include "MidiSineWaveSynthesizer.h"
#include "MidiSineWaveSynthesizer_p.h"

#include <cmath>

namespace talcs {
    static quint16 bundlePlayData(qint8 note, qint8 vel) {
        qint8 a[2] = {note, vel};
        return *reinterpret_cast<quint16 *>(a);
    }

    static void depackPlayData(quint16 playData, qint8 &note, qint8 &vel) {
        auto a = reinterpret_cast<qint8 *>(&playData);
        note = a[0];
        vel = a[1];
    }

    static inline double freqOfKey(int key) {
        return 440.0 * std::pow(2, 1.0 * (key - 69) / 12.0);
    }

    MidiSineWaveSynthesizer::MidiSineWaveSynthesizer() : AudioSource(), d(new MidiSineWaveSynthesizerPrivate) {
    }

    bool MidiSineWaveSynthesizer::open(qint64 bufferSize, double sampleRate) {
        return AudioStreamBase::open(bufferSize, sampleRate);
    }

    void MidiSineWaveSynthesizer::close() {
        QMutexLocker locker(&d->mutex);
        d->note = -1;
        d->phase = 0;
        AudioStreamBase::close();
    }

    MidiSineWaveSynthesizer::~MidiSineWaveSynthesizer() {

    }

    qint64 MidiSineWaveSynthesizer::read(const AudioSourceReadData &readData) {
        QMutexLocker locker(&d->mutex);
        static const double PI = 3.14159265358979323846;
        if (d->note == -1) {
            for (int ch = 0; ch < readData.buffer->channelCount(); ch++) {
                readData.buffer->clear(ch, readData.startPos, readData.length);
            }
        } else {
            double sr = sampleRate();
            double delta = 2 * PI * MidiMessage::getMidiNoteInHertz(d->note) / sr;
            for (qint64 i = 0; i < readData.length; i++) {
                if (d->fadeIn != 0.0) {
                    readData.buffer->sampleAt(0, readData.startPos + i) = std::sin(d->phase) * (1.0 * d->velocity / 127.0) * d->fadeIn;
                    d->fadeIn /= 0.99;
                    if (d->fadeIn >= 1)
                        d->fadeIn = 1;
                } else if (d->fadeOut != 0.0) {
                    readData.buffer->sampleAt(0, readData.startPos + i) = std::sin(d->phase) * (1.0 * d->velocity / 127.0) * d->fadeOut;
                    d->fadeOut *= 0.99;
                }
                d->phase += delta;
            }
            for (int ch = 1; ch < readData.buffer->channelCount(); ch++) {
                readData.buffer->setSampleRange(ch, readData.startPos, readData.length, *readData.buffer, 0, readData.startPos);
            }
        }
        if (d->fadeIn == 0.0 && d->fadeOut <= 0.005) {
            d->fadeOut = 0.0;
            d->note = -1;
            d->phase = 0;
        }
        return readData.length;

    }

    void MidiSineWaveSynthesizer::deviceWillStartCallback(MidiInputDevice *device) {

    }

    void MidiSineWaveSynthesizer::deviceStoppedCallback() {
        QMutexLocker locker(&d->mutex);
        d->note = -1;
        d->phase = 0;
    }

    void MidiSineWaveSynthesizer::workCallback(const MidiMessage &message) {
        QMutexLocker locker(&d->mutex);
        if (message.isNoteOn()) {
            d->note = message.getNoteNumber();
            if (d->fadeIn != 0.0) {
                d->fadeIn = qMin(1.0, d->fadeIn * d->velocity / message.getVelocity());
            } else if (d->fadeOut != 0.0) {
                d->fadeIn = qMin(1.0, d->fadeOut * d->velocity / message.getVelocity());
            }
            d->velocity = message.getVelocity();
            d->fadeOut = 0.0;
            if (d->fadeIn <= 0.005)
                d->fadeIn = 0.005;
        } else if (message.isNoteOff() && message.getNoteNumber() == d->note) {
            d->fadeOut = 1.0;
            d->fadeIn = 0.0;
        } else if (message.isAllNotesOff()) {
            d->fadeOut = 1.0;
            d->fadeIn = 0.0;
        }
    }

    void MidiSineWaveSynthesizer::errorCallback(const QString &errorString) {
        QMutexLocker locker(&d->mutex);
        d->note = -1;
        d->phase = 0;
    }
} // talcs