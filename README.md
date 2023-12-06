# TALCS

![TALCS Logo](https://raw.githubusercontent.com/CrSjimo/talcs/main/doc/talcs.svg)

<b>T</b>ALCS: <b>A</b>udio <b>L</b>ibrary of <b>C</b>r<b>S</b>jimo

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

- [Doxygen](https://www.doxygen.nl/)

- [qmsetup](https://github.com/stdware/qmsetup)