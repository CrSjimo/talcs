#include "NameProvider.h"

namespace talcs {
    QString NameProvider::name() const {
        return m_name;
    }

    void NameProvider::setName(const QString &name) {
        this->m_name = name;
    }
}
