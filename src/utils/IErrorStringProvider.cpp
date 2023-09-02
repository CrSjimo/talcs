#include "IErrorStringProvider.h"

namespace talcs {
    QString IErrorStringProvider::errorString() const {
        return m_error;
    }
    void IErrorStringProvider::setErrorString(const QString &error) {
        this->m_error = error;
    }
    void IErrorStringProvider::clearErrorString() {
        this->m_error.clear();
    }
}
