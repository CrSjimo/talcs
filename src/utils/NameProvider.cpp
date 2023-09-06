#include "NameProvider.h"

namespace talcs {
    /**
     * @class NameProvider
     * @brief Base class that supports setting and getting the name
     */

    /**
     * Gets the name.
     */
    QString NameProvider::name() const {
        return m_name;
    }

    /**
     * Sets the name.
     */
    void NameProvider::setName(const QString &name) {
        this->m_name = name;
    }
}
