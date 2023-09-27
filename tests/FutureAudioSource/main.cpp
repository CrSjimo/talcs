#include <list>

#include <QApplication>
#include <QFuture>
#include <QThread>
#include <QFutureWatcher>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QTreeWidget>
#include <QVBoxLayout>

#include <SVSBasic/LongTime.h>

#include <QDialog>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <device/AudioDevice.h>
#include <device/AudioDriver.h>
#include <device/AudioDriverManager.h>
#include <device/AudioSourcePlayback.h>
#include <source/SineWaveAudioSource.h>
#include <source/TransportAudioSource.h>
#include <synthesis/FutureAudioSource.h>
#include <synthesis/FutureAudioSourceClipSeries.h>

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
    FutureAudioSourceClip clip;
    QString id;
    SineWaveAudioSource *source;
    double rate;
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
        positionEdit->setText(SVS::LongTime(sampleToMsec(in.clip.position())).toString());
        lengthEdit->setText(SVS::LongTime(sampleToMsec(in.clip.length())).toString());
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
    QObject::connect(submitButton, &QPushButton::clicked, &dlg, [=, &dlg, &spec](){
        if (idEdit->text().isEmpty())
            return;
        if (SVS::LongTime::fromString(lengthEdit->text()).totalMsec() == 0)
            return;
        if (freqEdit->value() == 0.0)
            return;
        if (rateEdit->value() == 0.0)
            return;
        spec.id = idEdit->text();
        spec.rate = rateEdit->value();
        spec.source = new SineWaveAudioSource(freqEdit->value());
        QFutureInterface<PositionableAudioSource *> futureInterface;
        futureInterface.setProgressRange(0, msecToSample(SVS::LongTime::fromString(lengthEdit->text()).totalMsec()));
        futureInterface.reportStarted();
        spec.clip = {
            msecToSample(SVS::LongTime::fromString(positionEdit->text()).totalMsec()),
            new FutureAudioSource(futureInterface.future()),
            0,
            msecToSample(SVS::LongTime::fromString(lengthEdit->text()).totalMsec()),
        };
        dlg.accept();
    });
    if(dlg.exec() == QDialog::Accepted) {
        return spec;
    } else {
        return {};
    }
}

std::set<ClipSpec> futureList;

QMutex progressMutex;
[[noreturn]] void runProgress() {
    for(;;) {
        QMutexLocker locker(&progressMutex);
        QList<std::set<ClipSpec>::iterator> itToErase;
        for(auto it = futureList.begin(); it != futureList.end(); it++) {
            auto futureInterface = it->clip.content()->future().d;
            auto increment = msecToSample(1.0 * it->rate * 100.0);
            futureInterface.setProgressValue(std::min<int>(futureInterface.progressValue() + increment, futureInterface.progressMaximum()));
            if(futureInterface.progressValue() >= futureInterface.progressMaximum()) {
                futureInterface.reportFinished(reinterpret_cast<PositionableAudioSource *const *>(&it->source));
                itToErase.append(it);
            }
        }
        for(const auto &it: itToErase) {
            futureList.erase(it);
        }
        QThread::msleep(100);
    }
}

bool addClipToSeries(const ClipSpec &clipSpec) {
    QMutexLocker locker(&progressMutex);
    if(!series->addClip(clipSpec.clip))
        return false;
    futureList.insert(clipSpec);
    return true;
}

void terminateClip(const ClipSpec &clipSpec) {
    clipSpec.clip.content()->future().d.reportCanceled();
    delete clipSpec.clip.content();
    delete clipSpec.source;
}

void removeClipFromSeries(const ClipSpec &clipSpec) {
    QMutexLocker locker(&progressMutex);
    futureList.erase(clipSpec);
    series->removeClipAt(clipSpec.clip.position());
    terminateClip(clipSpec);
}

void addClip() {
    for(;;) {
        auto clipSpec = showClipEditDialog();
        if (clipSpec.id.isEmpty())
            break;
        if (addClipToSeries(clipSpec)) {
            auto item = new QTreeWidgetItem({
                clipSpec.id,
                SVS::LongTime(sampleToMsec(clipSpec.clip.position())).toString(),
                SVS::LongTime(sampleToMsec(clipSpec.clip.length())).toString(),
                QString::number(clipSpec.source->frequency()(1)),
                QString::number(clipSpec.rate),
            });
            item->setData(0, Qt::UserRole, QVariant::fromValue(clipSpec));
            QObject::connect(clipSpec.clip.content(), &FutureAudioSource::progressChanged, clipsList, [=](int progress){
                item->setText(5, QString::number(100.0 * progress / clipSpec.clip.length()) + "%");
            });
            QObject::connect(clipSpec.clip.content(), &FutureAudioSource::statusChanged, clipsList, [=](FutureAudioSource::Status status){
                if(status == talcs::FutureAudioSource::Ready) {
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
    if(!oldItem)
        return;
    auto oldClipSpec = oldItem->data(0, Qt::UserRole).value<ClipSpec>();
    for(;;) {
        auto clipSpec = showClipEditDialog(oldClipSpec);
        if(clipSpec.id.isEmpty())
            break;
        clipsList->takeTopLevelItem(clipsList->indexOfTopLevelItem(oldItem));
        removeClipFromSeries(oldClipSpec);
        delete oldItem;
        if (addClipToSeries(clipSpec)) {
            auto item = new QTreeWidgetItem({
                clipSpec.id,
                SVS::LongTime(sampleToMsec(clipSpec.clip.position())).toString(),
                SVS::LongTime(sampleToMsec(clipSpec.clip.length())).toString(),
                QString::number(clipSpec.source->frequency()(1)),
                QString::number(clipSpec.rate),
            });
            item->setData(0, Qt::UserRole, QVariant::fromValue(clipSpec));
            QObject::connect(clipSpec.clip.content(), &FutureAudioSource::progressChanged, clipsList, [=](int progress){
                item->setText(5, QString::number(100.0 * progress / clipSpec.clip.length()) + "%");
            });
            QObject::connect(clipSpec.clip.content(), &FutureAudioSource::statusChanged, clipsList, [=](FutureAudioSource::Status status){
                if(status == talcs::FutureAudioSource::Ready) {
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
    auto clipSpec = item->data(0, Qt::UserRole).value<ClipSpec>();
    removeClipFromSeries(clipSpec);
    delete item;
}

void reloadClip(QTreeWidgetItem *item) {
    if(!item)
        return;
    auto clipSpec = item->data(0, Qt::UserRole).value<ClipSpec>();
    auto freq = clipSpec.source->frequency()(1);
    removeClipFromSeries(clipSpec);
    clipSpec.source = new SineWaveAudioSource(freq);
    QFutureInterface<PositionableAudioSource *> futureInterface;
    futureInterface.setProgressRange(0, clipSpec.clip.length());
    futureInterface.reportStarted();
    clipSpec.clip = {
        clipSpec.clip.position(),
        new FutureAudioSource(futureInterface.future()),
        0,
        clipSpec.clip.length(),
    };
    addClipToSeries(clipSpec);
    item->setData(0, Qt::UserRole, QVariant::fromValue(clipSpec));
    QObject::connect(clipSpec.clip.content(), &FutureAudioSource::progressChanged, clipsList, [=](int progress){
        item->setText(5, QString::number(100.0 * progress / clipSpec.clip.length()) + "%");
    });
    QObject::connect(clipSpec.clip.content(), &FutureAudioSource::statusChanged, clipsList, [=](FutureAudioSource::Status status){
        if(status == talcs::FutureAudioSource::Ready) {
            item->setText(5, "Ready");
        }
    });
}

int main(int argc, char **argv) {
    QApplication a(argc, argv);
    qRegisterMetaType<ClipSpec>();
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
    clipsList->setHeaderLabels({"Id", "Position", "Length", "Frequency", "Rate", "Progress"});
    clipsList->setSortingEnabled(true);
    clipsList->sortItems(1, Qt::AscendingOrder);
    auto clipsActionsLayout = new QHBoxLayout;
    auto addButton = new QPushButton("Add");
    QObject::connect(addButton, &QPushButton::clicked, clipsList, &addClip);
    auto modifyButton = new QPushButton("Modify");
    QObject::connect(modifyButton, &QPushButton::clicked, clipsList, [=](){
        modifyClip(clipsList->currentItem());
    });
    auto deleteButton = new QPushButton("Delete");
    QObject::connect(deleteButton, &QPushButton::clicked, clipsList, [=](){
        deleteClip(clipsList->currentItem());
    });
    auto reloadButton = new QPushButton("Reload");
    QObject::connect(reloadButton, &QPushButton::clicked, clipsList, [=](){
        reloadClip(clipsList->currentItem());
    });
    clipsActionsLayout->addWidget(addButton);
    clipsActionsLayout->addWidget(modifyButton);
    clipsActionsLayout->addWidget(deleteButton);
    clipsActionsLayout->addWidget(reloadButton);
    clipsGroupBoxLayout->addWidget(clipsList);
    clipsGroupBoxLayout->addLayout(clipsActionsLayout);
    clipsGroupBox->setLayout(clipsGroupBoxLayout);

    auto transportGroupBox = new QGroupBox("Transport");
    auto transportGroupBoxLayout = new QVBoxLayout;
    auto timeLayout = new QHBoxLayout;
    auto timeLabel = new QLabel("00:00.000");
    QObject::connect(src, &TransportAudioSource::positionAboutToChange, timeLabel, [=](qint64 pos){
        timeLabel->setText(SVS::LongTime(sampleToMsec(pos)).toString());
    });
    auto loadingLabel = new QLabel;
    QObject::connect(src, &TransportAudioSource::bufferingCounterChanged, loadingLabel, [=](int counter){
        if(counter)
            loadingLabel->setText("Loading...");
        else
            loadingLabel->setText("");
    });
    timeLayout->addWidget(timeLabel);
    timeLayout->addWidget(loadingLabel);
    auto setTimeLayout = new QHBoxLayout;
    auto setTimeEdit = new QLineEdit;
    setTimeEdit->setPlaceholderText("Position");
    auto setTimeButton = new QPushButton("Set");
    QObject::connect(setTimeButton, &QPushButton::clicked, src, [=](){
        src->setPosition(msecToSample(SVS::LongTime::fromString(setTimeEdit->text()).totalMsec()));
        setTimeEdit->clear();
    });
    setTimeLayout->addWidget(setTimeEdit);
    setTimeLayout->addWidget(setTimeButton);
    auto playPauseButton = new QPushButton("Play");
    QObject::connect(playPauseButton, &QPushButton::clicked, src, [=](){
        if(src->isPlaying()) {
            src->pause();
            playPauseButton->setText("Play");
        } else {
            src->play();
            playPauseButton->setText("Pause");
        }
    });
    transportGroupBoxLayout->addLayout(timeLayout);
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