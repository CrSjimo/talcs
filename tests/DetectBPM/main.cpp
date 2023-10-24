#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QMessageBox>
#include <QFileDialog>
#include <QThread>

#include "format/AudioFormatIO.h"
#include "format/AudioFormatInputSource.h"
#include "tools/AudioSourceBPMDetector.h"

using namespace talcs;

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    auto win = new QMainWindow;
    auto mainWidget = new QWidget;
    auto mainLayout = new QVBoxLayout;
    auto filenameLabel = new QLabel;
    auto browseButton = new QPushButton("Browse");
    auto detectButton = new QPushButton("Detect");
    mainLayout->addWidget(filenameLabel);
    mainLayout->addWidget(browseButton);
    mainLayout->addWidget(detectButton);
    mainWidget->setLayout(mainLayout);
    win->setCentralWidget(mainWidget);
    win->show();

    QFile f;
    AudioFormatIO afio(&f);
    AudioFormatInputSource src(&afio);

    QObject::connect(browseButton, &QPushButton::clicked, win, [&](){
        auto filename = QFileDialog::getOpenFileName(win);
        if(filename.isEmpty()) return;
        f.setFileName(filename);
        filenameLabel->setText(filename);
    });

    QObject::connect(detectButton, &QPushButton::clicked, win, [&](){
        QDialog dlg;
        dlg.setWindowTitle("Detecting...");
        auto dlgLayout = new QVBoxLayout;
        auto exportProgressBar = new QProgressBar;
        dlgLayout->addWidget(exportProgressBar);
        dlg.setLayout(dlgLayout);

        afio.open(QIODevice::ReadOnly);
        auto sampleRate = afio.sampleRate();
        src.open(4096, sampleRate);
        QThread thread;
        AudioSourceBPMDetector detector(&src, src.length());
        detector.moveToThread(&thread);
        QObject::connect(&thread, &QThread::started, &detector, &AudioSourceBPMDetector::start);
        QObject::connect(&detector, &AudioSourceBPMDetector::blockProcessed, exportProgressBar, [&](qint64 sampleCountProcessed) {
            exportProgressBar->setValue(sampleCountProcessed * 100 / src.length());
        });
        QObject::connect(&dlg, &QDialog::rejected, &thread, [&]() { detector.interrupt(); });
        QObject::connect(&detector, &AudioSourceBPMDetector::finished, win,[&] {
            if(detector.status() == talcs::AudioSourceProcessorBase::Completed)
                dlg.accept();
            else
                QMessageBox::warning(win, "Export", "Exporting is interrupted.");
        });
        thread.start();
        dlg.exec();
        QMessageBox::information(win, "BPM", QString::number(detector.bpm()));
        src.close();
        src.setNextReadPosition(0);
        thread.quit();
        thread.wait();

    });

    return a.exec();
}
