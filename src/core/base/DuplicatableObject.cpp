#include "DuplicatableObject.h"

namespace talcs {

    /**
     * @class DuplicatableObject
     * @brief Base class for objects that can be duplicated
     */

    /**
     * Gets whether the object can be duplicated.
     */
    bool DuplicatableObject::isDuplicatable() const {
        return false;
    }

    /**
     * Duplicate the object.
     *
     * It is the caller's responsibility to delete the new object.
     *
     * - If the new object is stream-like, it is not opened even though the original object is opened and relevant
     * stream-related specifications are also not copied.
     *
     * - If the new object is QObject, the parent of the original object is copied to the new object.
     *
     * - Even if the original object's ownership is taken by another object, the new object's ownership is not taken by
     * that object.
     *
     * - If the original object holds another object, the held object is copied to the new object. If the original
     * object owns the held object, then the new object also owns the copied held object. Therefore, if the original
     * object does not own the object held by it, it should be noted that the lifetime of copied held-object should be
     * managed properly.
     */
    DuplicatableObject *DuplicatableObject::duplicate() const {
        return nullptr;
    }

} // talcs