# TALCS

![TALCS Logo](https://raw.githubusercontent.com/CrSjimo/talcs/main/doc/talcs.svg)

<b>T</b>ALCS: <b>A</b>udio <b>L</b>ibrary of <b>C</b>r<b>S</b>jimo

[GitHub homepage](https://github.com/CrSjimo/talcs)

[![Run Unit Tests](https://github.com/CrSjimo/talcs/actions/workflows/run-unit-tests.yaml/badge.svg)](https://github.com/CrSjimo/talcs/actions/workflows/run-unit-tests.yaml)
[![Deploy documentations to GitHub Pages](https://github.com/CrSjimo/talcs/actions/workflows/deploy-docs.yaml/badge.svg)](https://github.com/CrSjimo/talcs/actions/workflows/deploy-docs.yaml)
[![Coverity](https://scan.coverity.com/projects/29491/badge.svg)](https://scan.coverity.com/projects/crsjimo-talcs)

## Modules

- Core

  The core module includes basic buffers, sources and utilities for audio processing.

- Device

  The device module includes functionalities of outputting audio to the device.

- Format

  The format module includes functionalities of accessing formatted audio files.

- Remote

  The remote module provides interfaces to communicate with the remote audio bridge. See [talcs-remote](https://github.com/CrSjimo/talcs-remote).

- Synthesis

  The synthesis module provides interfaces for asynchronous audio synthesizers.

## Documentation

[Documentation homepage](https://talcs.sjimo.dev/)

## Dependencies

- [Qt 5.15.2](https://qt-project.org/)

  A cross-platform application development framework

- [5cript/interval-tree](https://github.com/5cript/interval-tree)

  A C++ header only interval tree implementation.

- [libsdl-org/SDL](https://github.com/libsdl-org/SDL)

  Simple Directmedia Layer

- [libsndfile/libsndfile](https://github.com/libsndfile/libsndfile)

  A C library for reading and writing sound files containing sampled audio data.

- [ASIO SDK](https://www.steinberg.net/developers/) <i>(optional)</i>

  Audio Stream Input/Output (ASIO) is a protocol allowing communication between a software application and a computer’s sound card.

- [avaneev/r8brain-free-src](https://github.com/avaneev/r8brain-free-src)

  High-quality pro audio resampler / sample rate converter C++ library. Very fast, for both audio resampling and time-series interpolation.

- [rpclib/rpclib](https://github.com/rpclib/rpclib)

  rpclib is a modern C++ msgpack-RPC server and client library

## Build Dependencies

- [Doxygen](https://www.doxygen.nl/) <i>(optional)</i>

- [qmsetup](https://github.com/stdware/qmsetup)

## License of TALCS

TALCS is free software: you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

TALCS is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with TALCS. If not, see <https://www.gnu.org/licenses/>.

## License of the TALCS Documentation

Permission is granted to copy, distribute and/or modify this document under the terms of the GNU Free Documentation License, Version 1.3 or any later version published by the Free Software Foundation; with no Invariant Sections, no Front-Cover Texts, and no Back-Cover Texts. A copy of the license is included in the section entitled "GNU Free Documentation License".