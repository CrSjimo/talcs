#ifndef TALCS_ERRORSTRINGPROVIDER_H
#define TALCS_ERRORSTRINGPROVIDER_H

#include <QString>

#include "global/TalcsGlobal.h"

namespace talcs {
    class TALCS_EXPORT ErrorStringProvider {
    public:
        QString errorString() const;

    protected:
        void setErrorString(const QString &error);
        void clearErrorString();

    private:
        QString m_error;
    };
}



#endif // TALCS_ERRORSTRINGPROVIDER_H
