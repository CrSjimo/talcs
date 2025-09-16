/******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                 *
 *                                                                            *
 * This file is part of TALCS.                                                *
 *                                                                            *
 * TALCS is free software: you can redistribute it and/or modify it under the *
 * terms of the GNU Lesser General Public License as published by the Free    *
 * Software Foundation, either version 3 of the License, or (at your option)  *
 * any later version.                                                         *
 *                                                                            *
 * TALCS is distributed in the hope that it will be useful, but WITHOUT ANY   *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS  *
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for    *
 * more details.                                                              *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with TALCS. If not, see <https://www.gnu.org/licenses/>.             *
 ******************************************************************************/

#include <set>

#include <QApplication>
#include <QComboBox>
#define QFUTURE_TEST
#include <QFuture>
#include <QFutureWatcher>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QThread>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <QDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QProgressBar>

#include <TalcsDevice//AudioDevice.h>
#include <TalcsDevice//AudioDriver.h>
#include <TalcsDevice//AudioDriverManager.h>
#include <TalcsDevice//AudioSourcePlayback.h>
#include <TalcsCore/SineWaveAudioSource.h>
#include <TalcsCore/TransportAudioSource.h>
#include <TalcsCore/FutureAudioSource.h>
#include <TalcsCore/FutureAudioSourceClipSeries.h>

using namespace talcs;

static AudioDevice *dev = nullptr;
static TransportAudioSource *src = nullptr;
static FutureAudioSourceClipSeries *series = nullptr;
static QTreeWidget *clipsList = nullptr;

inline int sampleToMsec(qint64 sample) {
    return 1.0 * sample / dev->sampleRate() * 1000.0;
}

inline qint64 msecToSample(double msec) {
    return msec * dev->sampleRate() / 1000;
}

void openDevice() {
    auto mgr = AudioDriverManager::createBuiltInDriverManager();
    auto drv = mgr->driver(mgr->drivers()[0]);
    drv->initialize();
    dev = drv->createDevice(drv->defaultDevice());
    dev->open(dev->preferredBufferSize(), dev->preferredSampleRate());
    src = new TransportAudioSource(series = new FutureAudioSourceClipSeries, false);
    series->setBufferingTarget(src);
    auto playback = new AudioSourcePlayback(src);
    dev->start(playback);
}

struct ClipSpec {
    FutureAudioSourceClipSeries::ClipView clip;
    QString id;
    SineWaveAudioSource *source;
    double rate;
    FutureAudioSource *futureSource;
    qint64 position;
    qint64 length;
    inline bool operator<(const ClipSpec &other) const {
        return clip.position() < other.clip.position();
    }
};
Q_DECLARE_METATYPE(ClipSpec)

ClipSpec showClipEditDialog(const ClipSpec &in = {}) {
    QDialog dlg;
    auto layout = new QFormLayout;
    dlg.setLayout(layout);

    auto idEdit = new QLineEdit;
    auto positionEdit = new QLineEdit;
    auto lengthEdit = new QLineEdit;
    auto freqEdit = new QDoubleSpinBox;
    freqEdit->setRange(0, 1000);
    auto rateEdit = new QDoubleSpinBox;

    if (!in.id.isEmpty()) {
        idEdit->setText(in.id);
        positionEdit->setText(QString::number(double(sampleToMsec(in.clip.position())) * .001, 'f', 3));
        lengthEdit->setText(QString::number(double(sampleToMsec(in.clip.length())) * .001, 'f', 3));
        freqEdit->setValue(in.source->frequency()(1));
        rateEdit->setValue(in.rate);
    }

    layout->addRow("ID", idEdit);
    layout->addRow("Position", positionEdit);
    layout->addRow("Length", lengthEdit);
    layout->addRow("Frequency", freqEdit);
    layout->addRow("Rate", rateEdit);

    auto submitButton = new QPushButton("Submit");
    layout->addRow(submitButton);

    ClipSpec spec;
    QObject::connect(submitButton, &QPushButton::clicked, &dlg, [=, &dlg, &spec]() {
        if (idEdit->text().isEmpty())
            return;
        if (lengthEdit->text().toDouble() * 1000.0 == 0)
            return;
        if (freqEdit->value() == 0.0)
            return;
        if (rateEdit->value() == 0.0)
            return;
        spec.id = idEdit->text();
        spec.rate = rateEdit->value();
        spec.source = new SineWaveAudioSource(freqEdit->value());
        QFutureInterface<PositionableAudioSource *> futureInterface;
        futureInterface.setProgressRange(0, msecToSample(lengthEdit->text().toDouble() * 1000.0));
        futureInterface.reportStarted();
        spec.position = msecToSample(positionEdit->text().toDouble() * 1000.0);
        spec.futureSource = new FutureAudioSource(futureInterface.future());
        spec.length = msecToSample(lengthEdit->text().toDouble() * 1000.0);
        dlg.accept();
    });
    if (dlg.exec() == QDialog::Accepted) {
        return spec;
    } else {
        return {};
    }
}

std::set<ClipSpec> futureList;

QMutex progressMutex;
[[noreturn]] void runProgress() {
    for (;;) {
        QMutexLocker locker(&progressMutex);
        QList<std::set<ClipSpec>::iterator> itToErase;
        for (auto it = futureList.begin(); it != futureList.end(); it++) {
            auto futureInterface = it->futureSource->future().d;
            auto increment = msecToSample(1.0 * it->rate * 100.0);
            futureInterface.setProgressValue(
                qMin<int>(futureInterface.progressValue() + increment, futureInterface.progressMaximum()));
            if (futureInterface.progressValue() >= futureInterface.progressMaximum()) {
                futureInterface.reportFinished(reinterpret_cast<PositionableAudioSource *const *>(&it->source));
                itToErase.append(it);
            }
        }
        for (const auto &it : itToErase) {
            futureList.erase(it);
        }
        QThread::msleep(100);
    }
}

bool addClipToSeries(ClipSpec &clipSpec) {
    QMutexLocker locker(&progressMutex);
    clipSpec.clip = series->insertClip(clipSpec.futureSource, clipSpec.position, 0, clipSpec.length);
    if (!clipSpec.clip.isValid())
        return false;
    futureList.insert(clipSpec);

    return true;
}

void terminateClip(const ClipSpec &clipSpec) {
    clipSpec.futureSource->future().d.reportCanceled();
    delete clipSpec.futureSource;
    delete clipSpec.source;
}

void removeClipFromSeries(const ClipSpec &clipSpec) {
    QMutexLocker locker(&progressMutex);
    futureList.erase(clipSpec);
    series->removeClip(clipSpec.clip);
    terminateClip(clipSpec);
}

void addClip() {
    for (;;) {
        auto clipSpec = showClipEditDialog();
        if (clipSpec.id.isEmpty())
            break;
        if (addClipToSeries(clipSpec)) {
            auto item = new QTreeWidgetItem({
                clipSpec.id,
                QString::number(double(sampleToMsec(clipSpec.position)) * .001, 'f', 3),
                QString::number(double(sampleToMsec(clipSpec.length)) * .001, 'f', 3),
                QString::number(clipSpec.source->frequency()(1)),
                QString::number(clipSpec.rate),
            });
            item->setData(0, Qt::UserRole, QVariant::fromValue(QByteArray(reinterpret_cast<char *>(&clipSpec), sizeof(ClipSpec))));
            QObject::connect(clipSpec.futureSource, &FutureAudioSource::progressChanged, clipsList,
                             [=](int progress) {
                                 item->setText(5, QString::number(100.0 * progress / clipSpec.length) + "%");
                             });
            QObject::connect(clipSpec.futureSource, &FutureAudioSource::statusChanged, clipsList,
                             [=](FutureAudioSource::Status status) {
                                 if (status == talcs::FutureAudioSource::Ready) {
                                     item->setText(5, "Ready");
                                 }
                             });
            clipsList->addTopLevelItem(item);
            break;
        } else {
            QMessageBox::critical(clipsList, "Error", "Clip overlaps. Please try again");
            terminateClip(clipSpec);
        }
    }
}

void modifyClip(QTreeWidgetItem *oldItem) {
    if (!oldItem)
        return;
    auto oldClipSpecData = oldItem->data(0, Qt::UserRole).value<QByteArray>();
    auto oldClipSpec = *reinterpret_cast<ClipSpec *>(oldClipSpecData.data());
    for (;;) {
        auto clipSpec = showClipEditDialog(oldClipSpec);
        if (clipSpec.id.isEmpty())
            break;
        AudioDeviceLocker devLocker(dev);
        clipsList->takeTopLevelItem(clipsList->indexOfTopLevelItem(oldItem));
        removeClipFromSeries(oldClipSpec);
        delete oldItem;
        if (addClipToSeries(clipSpec)) {
            auto item = new QTreeWidgetItem({
                clipSpec.id,
                QString::number(double(sampleToMsec(clipSpec.position)) * .001, 'f', 3),
                QString::number(double(sampleToMsec(clipSpec.length)) * .001, 'f', 3),
                QString::number(clipSpec.source->frequency()(1)),
                QString::number(clipSpec.rate),
            });
            item->setData(0, Qt::UserRole, QVariant::fromValue(QByteArray(reinterpret_cast<char *>(&clipSpec), sizeof(ClipSpec))));
            QObject::connect(clipSpec.futureSource, &FutureAudioSource::progressChanged, clipsList,
                             [=](int progress) {
                                 item->setText(5, QString::number(100.0 * progress / clipSpec.length) + "%");
                             });
            QObject::connect(clipSpec.futureSource, &FutureAudioSource::statusChanged, clipsList,
                             [=](FutureAudioSource::Status status) {
                                 if (status == talcs::FutureAudioSource::Ready) {
                                     item->setText(5, "Ready");
                                 }
                             });
            clipsList->addTopLevelItem(item);
            break;
        } else {
            QMessageBox::critical(clipsList, "Error", "Clip overlaps. Please try again");
            terminateClip(clipSpec);
        }
    }
}

void deleteClip(QTreeWidgetItem *item) {
    if (!item)
        return;
    clipsList->takeTopLevelItem(clipsList->indexOfTopLevelItem(item));
    auto clipSpecData = item->data(0, Qt::UserRole).value<QByteArray>();
    auto clipSpec = *reinterpret_cast<ClipSpec *>(clipSpecData.data());
    removeClipFromSeries(clipSpec);
    delete item;
}

void reloadClip(QTreeWidgetItem *item) {
    if (!item)
        return;
    auto clipSpecData = item->data(0, Qt::UserRole).value<QByteArray>();
    auto clipSpec = *reinterpret_cast<ClipSpec *>(clipSpecData.data());
    auto freq = clipSpec.source->frequency()(1);
    AudioDeviceLocker devLocker(dev);
    removeClipFromSeries(clipSpec);
    clipSpec.source = new SineWaveAudioSource(freq);
    QFutureInterface<PositionableAudioSource *> futureInterface;
    futureInterface.setProgressRange(0, clipSpec.length);
    futureInterface.reportStarted();
    clipSpec.futureSource = new FutureAudioSource(futureInterface.future());
    addClipToSeries(clipSpec);
    item->setData(0, Qt::UserRole, QVariant::fromValue(QByteArray(reinterpret_cast<char *>(&clipSpec), sizeof(ClipSpec))));
    QObject::connect(clipSpec.futureSource, &FutureAudioSource::progressChanged, clipsList, [=](int progress) {
        item->setText(5, QString::number(100.0 * progress / clipSpec.length) + "%");
    });
    QObject::connect(clipSpec.futureSource, &FutureAudioSource::statusChanged, clipsList,
                     [=](FutureAudioSource::Status status) {
                         if (status == talcs::FutureAudioSource::Ready) {
                             item->setText(5, "Ready");
                         }
                     });
}

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    openDevice();
    auto t = QThread::create(&runProgress);
    t->start();
    auto win = new QMainWindow;
    auto winWidget = new QWidget;
    auto winLayout = new QVBoxLayout;
    auto devLabel = new QLabel("Device: " + dev->name());

    auto clipsGroupBox = new QGroupBox("Clips");
    auto clipsGroupBoxLayout = new QVBoxLayout;
    clipsList = new QTreeWidget;
    clipsList->setColumnCount(6);
    clipsList->setHeaderLabels({"ID", "Position", "Length", "Frequency", "Rate", "Progress"});
    clipsList->setSortingEnabled(true);
    clipsList->sortItems(1, Qt::AscendingOrder);
    auto clipsActionsLayout = new QHBoxLayout;
    auto addButton = new QPushButton("Add");
    QObject::connect(addButton, &QPushButton::clicked, clipsList, &addClip);
    auto modifyButton = new QPushButton("Modify");
    QObject::connect(modifyButton, &QPushButton::clicked, clipsList, [=]() { modifyClip(clipsList->currentItem()); });
    auto deleteButton = new QPushButton("Delete");
    QObject::connect(deleteButton, &QPushButton::clicked, clipsList, [=]() { deleteClip(clipsList->currentItem()); });
    auto reloadButton = new QPushButton("Reload");
    QObject::connect(reloadButton, &QPushButton::clicked, clipsList, [=]() { reloadClip(clipsList->currentItem()); });
    auto loadingProgressBar = new QProgressBar;
    auto availableProgressBar = new QProgressBar;
    QObject::connect(series, &FutureAudioSourceClipSeries::progressChanged, win,
                     [=](int lengthAvailable, int lengthLoaded, int lengthOfAllClips) {
                         loadingProgressBar->setMaximum(lengthOfAllClips);
                         availableProgressBar->setMaximum(lengthOfAllClips);
                         loadingProgressBar->setValue(lengthLoaded);
                         availableProgressBar->setValue(lengthAvailable);
                     });
    auto progressBarLayout = new QFormLayout;
    progressBarLayout->addRow("Loading", loadingProgressBar);
    progressBarLayout->addRow("Available", availableProgressBar);
    clipsActionsLayout->addWidget(addButton);
    clipsActionsLayout->addWidget(modifyButton);
    clipsActionsLayout->addWidget(deleteButton);
    clipsActionsLayout->addWidget(reloadButton);
    clipsGroupBoxLayout->addWidget(clipsList);
    clipsGroupBoxLayout->addLayout(clipsActionsLayout);
    clipsGroupBoxLayout->addLayout(progressBarLayout);
    clipsGroupBox->setLayout(clipsGroupBoxLayout);

    auto transportGroupBox = new QGroupBox("Transport");
    auto transportGroupBoxLayout = new QVBoxLayout;
    auto timeLayout = new QHBoxLayout;
    auto timeLabel = new QLabel("00:00.000");
    QObject::connect(src, &TransportAudioSource::positionAboutToChange, timeLabel,
                     [=](qint64 pos) { timeLabel->setText(QString::number(double(sampleToMsec(pos)) * .001, 'f', 3)); });
    auto loadingLabel = new QLabel;
    QObject::connect(src, &TransportAudioSource::bufferingCounterChanged, loadingLabel, [=](int counter) {
        if (counter)
            loadingLabel->setText("Loading...");
        else
            loadingLabel->setText("");
    });
    timeLayout->addWidget(timeLabel);
    timeLayout->addWidget(loadingLabel);
    auto setModeComboBox = new QComboBox;
    setModeComboBox->addItems({"Notify", "Skip", "Block"});
    QObject::connect(setModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), series, [=](int index){
        series->setReadMode((FutureAudioSourceClipSeries::ReadMode)index);
    });
    auto setTimeLayout = new QHBoxLayout;
    auto setTimeEdit = new QLineEdit;
    setTimeEdit->setPlaceholderText("Position");
    auto setTimeButton = new QPushButton("Set");
    QObject::connect(setTimeButton, &QPushButton::clicked, src, [=]() {
        src->setPosition(msecToSample(setTimeEdit->text().toDouble() * 1000.0));
        setTimeEdit->clear();
    });
    setTimeLayout->addWidget(setTimeEdit);
    setTimeLayout->addWidget(setTimeButton);
    auto playPauseButton = new QPushButton("Play");
    QObject::connect(playPauseButton, &QPushButton::clicked, src, [=]() {
        if (src->isPlaying()) {
            src->pause();
            playPauseButton->setText("Play");
        } else {
            src->play();
            playPauseButton->setText("Pause");
        }
    });
    transportGroupBoxLayout->addLayout(timeLayout);
    transportGroupBoxLayout->addWidget(setModeComboBox);
    transportGroupBoxLayout->addLayout(setTimeLayout);
    transportGroupBoxLayout->addWidget(playPauseButton);
    transportGroupBox->setLayout(transportGroupBoxLayout);

    winLayout->addWidget(devLabel);
    winLayout->addWidget(clipsGroupBox);
    winLayout->addWidget(transportGroupBox);
    winWidget->setLayout(winLayout);
    win->setCentralWidget(winWidget);
    win->show();
    return a.exec();
}
