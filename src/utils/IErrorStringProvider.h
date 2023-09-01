//
// Created by Crs_1 on 2023/7/10.
//

#ifndef IERRORSTRINGPROVIDER_H
#define IERRORSTRINGPROVIDER_H

#include <QString>

#include "global/TalcsGlobal.h"

class TALCS_EXPORT IErrorStringProvider {
public:
    QString errorString() const;
protected:
    void setErrorString(const QString &error);
    void clearErrorString();
private:
    QString m_error;
};



#endif // IERRORSTRINGPROVIDER_H
