#include "ErrorStringProvider.h"

namespace talcs {

    /**
     * @class ErrorStringProvider
     * @brief Base class that supports setting and getting the error string.
     */

    /**
     * Gets the error string.
     */
    QString ErrorStringProvider::errorString() const {
        return m_error;
    }

    /**
     * Sets the error string.
     */
    void ErrorStringProvider::setErrorString(const QString &error) {
        this->m_error = error;
    }

    /**
     * Clears the error string.
     */
    void ErrorStringProvider::clearErrorString() {
        this->m_error.clear();
    }
    
}
