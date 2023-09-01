//
// Created by Crs_1 on 2023/7/10.
//

#ifndef INAMEPROVIDER_H
#define INAMEPROVIDER_H

#include <QString>

#include "global/TalcsGlobal.h"

class TALCS_EXPORT INameProvider {
public:
    QString name() const;
protected:
    void setName(const QString &name);
private:
    QString m_name;
};



#endif // INAMEPROVIDER_H
