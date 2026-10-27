/******************************************************************************
 * Copyright (c) 2023-2024 CrSjimo                                            *
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

#include "MidiInputEvent.h"

#include <atomic>
#include <chrono>

namespace talcs {

    double MidiInputEvent::currentTime() noexcept {
        static const auto epoch = std::chrono::steady_clock::now();
        return 1.0 + std::chrono::duration<double>(std::chrono::steady_clock::now() - epoch).count();
    }

    quint64 MidiInputEvent::createSourceId() noexcept {
        static std::atomic<quint64> next{1};
        return next.fetch_add(1, std::memory_order_relaxed);
    }

}
