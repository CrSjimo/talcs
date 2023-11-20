#include "DuplicatableObject.h"

namespace talcs {

    bool DuplicatableObject::isDuplicatable() const {
        return false;
    }

    DuplicatableObject *DuplicatableObject::duplicate() const {
        return nullptr;
    }

} // talcs