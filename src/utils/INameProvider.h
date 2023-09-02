#ifndef INAMEPROVIDER_H
#define INAMEPROVIDER_H

#include <QString>

#include "global/TalcsGlobal.h"

namespace talcs {
    class TALCS_EXPORT INameProvider {
    public:
        QString name() const;

    protected:
        void setName(const QString &name);

    private:
        QString m_name;
    };
}



#endif // INAMEPROVIDER_H
