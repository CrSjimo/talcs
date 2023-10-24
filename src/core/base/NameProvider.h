#ifndef TALCS_NAMEPROVIDER_H
#define TALCS_NAMEPROVIDER_H

#include <QString>

#include "global/TalcsGlobal.h"

namespace talcs {
    class TALCS_EXPORT NameProvider {
    public:
        QString name() const;

    protected:
        void setName(const QString &name);

    private:
        QString m_name;
    };
}



#endif // TALCS_NAMEPROVIDER_H
