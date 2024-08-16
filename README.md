# TALCS

![TALCS Logo](https://raw.githubusercontent.com/CrSjimo/talcs/main/doc/talcs.svg)

<b>T</b>ALCS: <b>A</b>udio <b>L</b>ibrary of <b>C</b>r<b>S</b>jimo

[GitHub homepage](https://github.com/CrSjimo/talcs)

[![Run Unit Tests](https://github.com/CrSjimo/talcs/actions/workflows/run-unit-tests.yaml/badge.svg)](https://github.com/CrSjimo/talcs/actions/workflows/run-unit-tests.yaml)
[![Deploy documentations to GitHub Pages](https://github.com/CrSjimo/talcs/actions/workflows/deploy-docs.yaml/badge.svg)](https://github.com/CrSjimo/talcs/actions/workflows/deploy-docs.yaml)
[![Coverity](https://scan.coverity.com/projects/29491/badge.svg)](https://scan.coverity.com/projects/crsjimo-talcs)

## Modules

- Core
  
  The core module includes basic buffers, sources, and utilities for audio processing. The core module is required by all other modules.

- Essential Modules
  
  - Device
    
    The device module includes functionalities for outputting audio to the device.
  
  - Format
    
    The format module includes functionalities for accessing formatted audio files.
  
  - MIDI
    
    The MIDI module can process MIDI messages, read inputs from MIDI devices, and do MIDI-related audio processing.

- Additional Modules
  
  - Remote (requires: Device, MIDI)
    
    The remote module provides interfaces to communicate with the remote audio bridge. See [talcs-remote](https://github.com/CrSjimo/talcs-remote).
  
  - JUCE Adapter
    
    The JUCE adapter module provides adapters for some classes in [JUCE](https://github.com/juce-framework/JUCE).
  
  - DSPX (requires: Format)
    
    The DSPX module includes classes for building up the audio-processing context in [DSPX](https://github.com/diffscope/opendspx/) editor applications.

Essential modules will be built by default, and additional modules will not be built by default. To enable or disable the build of a module, set the corresponding CMake option on or off.

The device module provides two internal implementations: SDL and ASIO. SDL is enabled and ASIO is disabled by default. To enable or disable, set the CMake option `TALCS_DEVICE_ENABLE_SDL` and `TALCS_DEVICE_ENABLE_ASIO` on or off.

## API Documentation

[Documentation homepage](https://talcs.sjimo.dev/)

## Dependencies

- [Qt](https://qt-project.org/) (tested on 5.15.2 and 6.5.3)

- [5cript/interval-tree](https://github.com/5cript/interval-tree)

- [libsdl-org/SDL](https://github.com/libsdl-org/SDL) (used by: Device)

- [libsndfile/libsndfile](https://github.com/libsndfile/libsndfile) (used by: Format)

- [ASIO SDK](https://www.steinberg.net/developers/) <i>(optional)</i> (used by: Device)

- [avaneev/r8brain-free-src](https://github.com/avaneev/r8brain-free-src) (used by: Format)

- [thestk/rtmidi](https://github.com/thestk/rtmidi) (used by: MIDI)

- [rpclib/rpclib](https://github.com/rpclib/rpclib) (used by: Remote)

- [Boost](https://www.boost.org/) (used by: Remote)

- [JUCE](https://github.com/juce-framework/JUCE) (used by: JUCE Adapter)

- Build Dependencies
  
  - [qmsetup](https://github.com/stdware/qmsetup)
  
  - [Doxygen](https://www.doxygen.nl/) <i>(optional)</i>

TALCS uses vcpkg to manage dependency libraries, except Qt, ASIO SDK, r8brain, and JUCE:

- The port files of interval-tree and qmsetup are in [stdware/vcpkg-overlay](https://github.com/stdware/vcpkg-overlay). The documentation about how to install libraries is also in this repository. This repository is included as a submodule in `scripts/vcpkg`.

- ASIO SDK should be downloaded manually from [Steinberg's official website](https://www.steinberg.net/asiosdk) and configured in CMake option `TALCS_ASIOSDK_DIR`. 

- r8brain is included as a submodule in the TALCS repository.

- JUCE should be manually specified in the CMake option `JUCE_DIR` to make CMake find it.

## License of TALCS

TALCS is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

TALCS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with TALCS. If not, see <https://www.gnu.org/licenses/>.

## License of the TALCS Documentation

Permission is granted to copy, distribute and/or modify this document under the terms of the GNU Free Documentation License, Version 1.3 or any later version published by the Free Software Foundation; with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts. A copy of the license is included in the section entitled "GNU Free Documentation License".