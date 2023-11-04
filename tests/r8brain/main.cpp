#include <QCoreApplication>

#include <QFile>
#include <QDebug>
#include <QTime>
#include <TalcsFormat/AudioFormatIO.h>
#include <TalcsFormat/R8BrainResampler.h>
#include <TalcsCore/InterleavedAudioDataWrapper.h>
#include <TalcsCore/AudioBuffer.h>

using namespace talcs;

void testFunc() {
    float *f32audio;
    double *f64audio;
    size_t length;

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

    R8BrainResampler resampler(1, 1024, [=](float *buffer, qint64 size) {
        static int pos = 0;
        std::copy(f32audio + pos, f32audio + pos + size, buffer);
        pos += size;
    });

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
