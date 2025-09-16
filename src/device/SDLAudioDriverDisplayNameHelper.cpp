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

#include <QString>
#include <QHash>

#include "SDLAudioDriverDisplayNameHelper.h"

namespace talcs {

    /**
     * @class SDLAudioDriverDisplayNameHelper
     * @brief Helpful function for getting the display name of an SDL audio driver.
     * @deprecated
     */

    /**
     * Gets the display name of an SDL audio driver.
     * @param driverName raw driver name
     * @deprecated
     */
    QString SDLAudioDriverDisplayNameHelper::getDisplayName(const QString &driverName) {
        static QHash<QString, QString> map = {
                {"pipewire",    "Pipewire"                   },
                {"pulseaudio",  "PulseAudio"                 },
                {"alsa",        "ALSA PCM audio"             },
                {"jack",        "JACK Audio Connection Kit"  },
                {"sndio",       "OpenBSD sndio"              },
                {"netbsd",      "NetBSD audio"               },
                {"dsp",         "OSS /dev/dsp standard audio"},
                {"qsa",         "QNX QSA Audio"              },
                {"audio",       "UNIX /dev/audio interface"  },
                {"arts",        "Analog RealTime Synthesizer"},
                {"esd",         "Enlightened Sound Daemon"   },
                {"nacl",        "SDL NaCl Audio Driver"      },
                {"nas",         "Network Audio System"       },
                {"wasapi",      "WASAPI"                     },
                {"directsound", "DirectSound"                },
                {"winmm",       "Windows Waveform Audio"     },
                {"paud",        "AIX Paudio"                 },
                {"haiku",       "Haiku BSoundPlayer"         },
                {"coreaudio",   "CoreAudio"                  },
                {"disk",        "direct-to-disk audio"       },
                {"dummy",       "SDL dummy audio driver"     },
                {"fusionsound", "FusionSound"                },
                {"AAudio",      "AAudio audio driver"        },
                {"openslES",    "opensl ES audio driver"     },
                {"android",     "SDL Android audio driver"   },
                {"ps2",         "PS2 audio driver"           },
                {"psp",         "PSP audio driver"           },
                {"vita",        "VITA audio driver"          },
                {"n3ds",        "SDL N3DS audio driver"      },
                {"emscripten",  "SDL emscripten audio driver"},
                {"DART",        "OS/2 DART"                  },
        };
        return map.value(driverName, driverName);
    }
}
