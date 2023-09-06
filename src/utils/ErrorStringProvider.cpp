#include "ErrorStringProvider.h"

namespace talcs {
    QString ErrorStringProvider::errorString() const {
        return m_error;
    }
    void ErrorStringProvider::setErrorString(const QString &error) {
        this->m_error = error;
    }
    void ErrorStringProvider::clearErrorString() {
        this->m_error.clear();
    }
}
