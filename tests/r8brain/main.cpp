#include <QCoreApplication>

#include <QFile>
#include <QDebug>
#include <QTime>
#include <TalcsFormat/AudioFormatIO.h>
#include <TalcsFormat/R8BrainResampler.h>
#include <TalcsCore/InterleavedAudioDataWrapper.h>
#include <TalcsCore/AudioBuffer.h>

using namespace talcs;

float *f32audio;
double *f64audio;
size_t length;

class MyResampler : public R8BrainResampler {
public:
    MyResampler(double ratio, qint64 bufferSize) : R8BrainResampler(ratio, bufferSize) {}

private:

    void read(float *inputBlock, qint64 length_) override {
        static int pos = 0;
        std::copy(f32audio + pos, f32audio + pos + length_, inputBlock);
        pos += length_;
    }
};

void testFunc() {
    {
        QFile f("D:\\CloudMusic\\07.恋染色.flac");
        AudioFormatIO io(&f);
        io.open(QFile::ReadOnly);
        length = io.length();
        auto p = new float[io.channelCount() * io.length()];
        io.read(p, io.length());
        InterleavedAudioDataWrapper wrapper(p, io.channelCount(), io.length());
        auto buf = AudioBuffer::from(wrapper);
        delete[] p;

        f32audio = new float[length];
        f64audio = new double[length];
        std::copy(buf.constData(0), buf.constData(0) + length, f32audio);
        std::copy(buf.constData(0), buf.constData(0) + length, f64audio);
    }

    qDebug() << "Length:" << length;

    MyResampler resampler(48000.0 / 44100.0, 1024);

    QFile f("D:/test.pcm");
    f.open(QFile::WriteOnly);

    float p[1024];

    for (int i = 0; i < 1000; i++) {
        resampler.process(p);
        f.write((char*)(p), 1024 * sizeof(float));
    }

    qDebug() << "Finished";

}


int main(int argc, char **argv) {
    QCoreApplication a(argc, argv);
    testFunc();
    return a.exec();
}
