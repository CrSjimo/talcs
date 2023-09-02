#ifndef IERRORSTRINGPROVIDER_H
#define IERRORSTRINGPROVIDER_H

#include <QString>

#include "global/TalcsGlobal.h"

namespace talcs {
    class TALCS_EXPORT IErrorStringProvider {
    public:
        QString errorString() const;

    protected:
        void setErrorString(const QString &error);
        void clearErrorString();

    private:
        QString m_error;
    };
}



#endif // IERRORSTRINGPROVIDER_H
