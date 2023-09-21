#ifndef TALCS_AUDIOCLIPBASE_H
#define TALCS_AUDIOCLIPBASE_H

#include <set>

#include <QPair>
#include <QScopedPointer>

#include "interval-tree/interval_tree.hpp"

#include "buffer/IAudioSampleProvider.h"

namespace talcs {

    using AudioClipBaseInterval_ = lib_interval_tree::interval<qint64>;

    template <class T>
    struct AudioClipBase : public AudioClipBaseInterval_ {
        explicit AudioClipBase(qint64 position = -1, T *content = nullptr, qint64 startPos = 0, qint64 length = 0)
            : AudioClipBaseInterval_(position, position + length - 1), m_content(content),
              m_startPos(startPos) {
        }
        T *m_content;
        qint64 m_startPos;

        qint64 position() const {
            return low_;
        }

        T *content() const {
            return m_content;
        }

        qint64 startPos() const {
            return m_startPos;
        }

        qint64 endPos() const {
            return high_ + 1;
        }

        qint64 length() const {
            return high_ - low_ + 1;
        }
    };

    template <class T>
    class AudioClipSeriesBase {
        using AudioClipBaseIntervalTree_ = lib_interval_tree::interval_tree<AudioClipBase<T>>;
    public:
        virtual bool addClip(const AudioClipBase<T> &clip) {
            if(qAsConst(m_clips).overlap_find(clip) != m_clips.end())
                return false;
            m_clips.insert(clip);
            m_endSet.insert(clip.endPos());
            return true;
        }
        AudioClipBase<T> findClipAt(qint64 pos) const {
            auto it = findClipIt(pos);
            if (it == m_clips.end())
                return {};
            return *it;
        }
        QList<AudioClipBase<T>> clips() const {
            return QList<AudioClipBase<T>>(m_clips.begin(), m_clips.end());
        }
        virtual bool removeClipAt(qint64 pos) {
            auto it = findClipIt(pos);
            if (it == m_clips.end())
                return false;
            m_clips.erase(it);
            m_endSet.erase(it->interval().endPos());
            return true;
        }
        virtual void clearClips() {
            m_clips.clear();
        }
        qint64 effectiveLength() const {
            if(m_endSet.empty())
                return 0;
            return *m_endSet.rbegin();
        }

    protected:
        typename AudioClipBaseIntervalTree_::const_iterator findClipIt(qint64 pos) const {
            return m_clips.overlap_find({pos, nullptr, 0, pos + 1});
        }
        AudioClipBaseIntervalTree_ m_clips;
        std::set<qint64> m_endSet;
    };
}

#endif // TALCS_AUDIOCLIPBASE_H
