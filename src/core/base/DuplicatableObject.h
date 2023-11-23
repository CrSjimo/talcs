#ifndef TALCS_DUPLICATABLEOBJECT_H
#define TALCS_DUPLICATABLEOBJECT_H

#include <TalcsCore/TalcsCoreGlobal.h>

namespace talcs {

    class TALCSCORE_EXPORT DuplicatableObject {
    public:
        virtual bool isDuplicatable() const;
        virtual DuplicatableObject *duplicate() const;
    };

} // talcs

#endif //TALCS_DUPLICATABLEOBJECT_H
