#ifndef TALCS_IMIXER_H
#define TALCS_IMIXER_H

#include <QList>

namespace talcs {
    template <class T>
    struct IMixer {
        virtual bool addSource(T *src, bool takeOwnership) = 0;
        virtual bool removeSource(T *src) = 0;
        virtual void removeAllSources() = 0;
        virtual QList<T *> sources() const = 0;

        virtual void setGain(float gain) = 0;
        virtual float gain() const = 0;

        virtual void setPan(float pan) = 0;
        virtual float pan() const = 0;

        virtual void setRouteChannels(bool routeChannels) = 0;
        virtual bool routeChannels() const = 0;
    };
}

#endif // TALCS_IMIXER_H
