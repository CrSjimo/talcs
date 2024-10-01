/******************************************************************************
* Copyright (c) 2024 CrSjimo                                                 *
 *                                                                            *
 * This file is part of TALCS.                                                *
 *                                                                            *
 * TALCS is free software: you can redistribute it and/or modify it under the *
 * terms of the GNU Lesser General Public License as published by the Free    *
 * Software Foundation, either version 3 of the License, or (at your option)  *
 * any later version.                                                         *
 *                                                                            *
 * TALCS is distributed in the hope that it will be useful, but WITHOUT ANY   *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS  *
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for    *
 * more details.                                                              *
 *                                                                            *
 * You should have received a copy of the GNU Lesser General Public License   *
 * along with TALCS. If not, see <https://www.gnu.org/licenses/>.             *
 ******************************************************************************/

#ifndef TALCS_TAKEOWNERSHIPPOINTER_H
#define TALCS_TAKEOWNERSHIPPOINTER_H

#include <memory>
#include <tuple>

#include <TalcsCore/TalcsCoreGlobal.h>

namespace talcs {

    template<typename T>
    class TakeOwnershipPointer {
    public:
        TakeOwnershipPointer(T *ptr = nullptr, bool takeOwnership = false) : d(ptr), m_takeOwnership(takeOwnership) {
        }
        ~TakeOwnershipPointer() {
            if (!m_takeOwnership) {
                std::ignore = d.release();
            }
        }

        void reset(T *ptr, bool takeOwnership) {
            std::ignore = d.release();
            d.reset(ptr);
            m_takeOwnership = takeOwnership;
        }

        T *get() const {
            return d.get();
        }

        T *operator->() const {
            return d.get();
        }
        T &operator*() const {
            return *d.get();
        }
        operator T*() const {
            return d.get();
        }

    private:
        std::unique_ptr<T> d;
        bool m_takeOwnership;
    };

} // talcs

#endif //TALCS_TAKEOWNERSHIPPOINTER_H
