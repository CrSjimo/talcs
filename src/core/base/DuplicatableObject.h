#ifndef TALCS_DUPLICATABLEOBJECT_H
#define TALCS_DUPLICATABLEOBJECT_H

namespace talcs {

    class DuplicatableObject {
    public:
        virtual bool isDuplicatable() const;
        virtual DuplicatableObject *duplicate() const;
    };

} // talcs

#endif //TALCS_DUPLICATABLEOBJECT_H
