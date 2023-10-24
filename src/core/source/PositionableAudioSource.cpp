#include "PositionableAudioSource.h"
#include "PositionableAudioSource_p.h"

namespace talcs {

    /**
     * @class PositionableAudioSource
     * @brief Base class for sources from which audio data can be streamly read, and the source can be repositioned
     */

    /**
     * Default constructor.
     */
    PositionableAudioSource::PositionableAudioSource() : PositionableAudioSource(*new PositionableAudioSourcePrivate) {
    }

    PositionableAudioSource::PositionableAudioSource(PositionableAudioSourcePrivate &d) : AudioSource(d) {
    }

    PositionableAudioSource::~PositionableAudioSource() = default;

    /**
     * Gets the next read position.
     */
    qint64 PositionableAudioSource::nextReadPosition() const {
        Q_D(const PositionableAudioSource);
        return d->position;
    }

    /**
     * Sets the next read position.
     *
     * The new position might be identical to the current position. Such cases should be handled properly.
     *
     * Note that for derived class that reimplement this function, it should be synchronized with read() function.
     */
    void PositionableAudioSource::setNextReadPosition(qint64 pos) {
        Q_D(PositionableAudioSource);
        d->position = pos;
    }

    PositionableAudioSourceStateSaver::PositionableAudioSourceStateSaver(PositionableAudioSource *src)
        : d(new PositionableAudioSourceStateSaverPrivate{src, src->nextReadPosition()}) {
    }

    PositionableAudioSourceStateSaver::~PositionableAudioSourceStateSaver() {
        d->src->setNextReadPosition(d->position);
    }
    
}
