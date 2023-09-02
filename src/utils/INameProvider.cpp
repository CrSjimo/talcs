#include "INameProvider.h"

namespace talcs {
    QString INameProvider::name() const {
        return m_name;
    }

    void INameProvider::setName(const QString &name) {
        this->m_name = name;
    }
}
