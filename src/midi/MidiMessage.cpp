/******************************************************************************
 * Copyright (c) 2022-2023 CrSjimo                                            *
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

/*
  ==============================================================================

   This file is migrated from the JUCE library.
   Copyright (c) 2022 - Raw Material Software Limited

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

#include "MidiMessage.h"

#include <cmath>

#include <QVector>

namespace talcs {

    template<typename T>
    static inline T jlimit(T lowerLimit, T upperLimit, T valueToConstrain) {
        return qBound(lowerLimit, valueToConstrain, upperLimit);
    }

    template<typename T>
    static inline T jmap(T sourceValue, T sourceRangeMin, T sourceRangeMax, T targetRangeMin, T targetRangeMax) {
        Q_ASSERT (!qFuzzyCompare(sourceRangeMax, sourceRangeMin));
        return targetRangeMin +
               ((targetRangeMax - targetRangeMin) * (sourceValue - sourceRangeMin)) / (sourceRangeMax - sourceRangeMin);
    }

    template<typename Type1, typename Type2>
    static inline bool isPositiveAndBelow(Type1 valueToTest, Type2 upperLimit) {
        Q_ASSERT (Type1() <= static_cast<Type1> (upperLimit));
        return Type1() <= valueToTest && valueToTest < static_cast<Type1> (upperLimit);
    }

    template<typename T>
    static inline int roundToInt(T value) {
        return qRound(value);
    }

    template<typename Type, size_t N>
    constexpr static inline int numElementsInArray(Type (&)[N]) {
        return N;
    }

    namespace MidiHelpers {
        inline quint8 initialByte(const int type, const int channel) noexcept {
            return (quint8) (type | jlimit(0, 15, channel - 1));
        }

        inline quint8 validVelocity(const int v) noexcept {
            return (quint8) jlimit(0, 127, v);
        }
    }

    /**
     * @class MidiMessage
     * @brief Encapsulates a MIDI message
     *
     * This class is migrated from the JUCE library.
     *
     * @see @link URL https://docs.juce.com/master/classMidiMessage.html @endlink
     */

//==============================================================================
    quint8 MidiMessage::floatValueToMidiByte(const float v) noexcept {
        Q_ASSERT(v >= 0 && v <= 1.0f);  // if your value is > 1, maybe you're passing an
        // integer value to a float method by mistake?

        return MidiHelpers::validVelocity(roundToInt(v * 127.0f));
    }

    quint16 MidiMessage::pitchbendToPitchwheelPos(const float pitchbend,
                                                  const float pitchbendRange) noexcept {
        // can't translate a pitchbend value that is outside of the given range!
        Q_ASSERT(std::abs(pitchbend) <= pitchbendRange);

        return static_cast<quint16> (pitchbend > 0.0f
                                     ? jmap(pitchbend, 0.0f, pitchbendRange, 8192.0f, 16383.0f)
                                     : jmap(pitchbend, -pitchbendRange, 0.0f, 0.0f, 8192.0f));
    }

//==============================================================================
    MidiMessage::VariableLengthValue
    MidiMessage::readVariableLengthValue(const quint8 *data, int maxBytesToUse) noexcept {
        quint32 v = 0;

        // The largest allowable variable-length value is 0x0f'ff'ff'ff which is
        // represented by the 4-byte stream 0xff 0xff 0xff 0x7f.
        // Longer bytestreams risk overflowing a 32-bit signed int.
        const auto limit = qMin(maxBytesToUse, 4);

        for (int numBytesUsed = 0; numBytesUsed < limit; ++numBytesUsed) {
            const auto i = data[numBytesUsed];
            v = (v << 7) + (i & 0x7f);

            if (!(i & 0x80))
                return {(int) v, numBytesUsed + 1};
        }

        // If this is hit, the input was malformed. Either there were not enough
        // bytes of input to construct a full value, or no terminating byte was
        // found. This implementation only supports variable-length values of up
        // to four bytes.
        return {};
    }

    int MidiMessage::readVariableLengthVal(const quint8 *data, int &numBytesUsed) noexcept {
        numBytesUsed = 0;
        int v = 0, i;

        do {
            i = (int) *data++;

            if (++numBytesUsed > 6)
                break;

            v = (v << 7) + (i & 0x7f);

        } while (i & 0x80);

        return v;
    }

    int MidiMessage::getMessageLengthFromFirstByte(const quint8 firstByte) noexcept {
        // this method only works for valid starting bytes of a short midi message
        Q_ASSERT(firstByte >= 0x80 && firstByte != 0xf0 && firstByte != 0xf7);

        static const char messageLengths[] =
                {
                        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,
                        3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3, 3,
                        1, 2, 3, 2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
                };

        return messageLengths[firstByte & 0x7f];
    }

//==============================================================================
    MidiMessage::MidiMessage() noexcept
            : m_data(2) {
        m_data[0] = 0xf0;
        m_data[1] = 0xf7;
    }

    MidiMessage::MidiMessage(const void *const d, const int dataSize, const double t)
            : m_timeStamp(t), m_data(dataSize) {
        Q_ASSERT(dataSize > 0);
        // this checks that the length matches the data..
        Q_ASSERT(dataSize > 3 || *(quint8 *) d >= 0xf0 || getMessageLengthFromFirstByte(*(quint8 *) d) == m_data.size());

        m_data.resize(dataSize);
        memcpy(m_data.data(), d, (size_t) dataSize);
    }

    MidiMessage::MidiMessage(const int byte1, const double t) noexcept
            : m_timeStamp(t), m_data(1) {
        m_data[0] = (quint8) byte1;

        // check that the length matches the data..
        Q_ASSERT(byte1 >= 0xf0 || getMessageLengthFromFirstByte((quint8) byte1) == 1);
    }

    MidiMessage::MidiMessage(const int byte1, const int byte2, const double t) noexcept
            : m_timeStamp(t), m_data(2) {
        m_data[0] = (quint8) byte1;
        m_data[1] = (quint8) byte2;

        // check that the length matches the data..
        Q_ASSERT(byte1 >= 0xf0 || getMessageLengthFromFirstByte((quint8) byte1) == 2);
    }

    MidiMessage::MidiMessage(const int byte1, const int byte2, const int byte3, const double t) noexcept
            : m_timeStamp(t), m_data(3) {
        m_data[0] = (quint8) byte1;
        m_data[1] = (quint8) byte2;
        m_data[2] = (quint8) byte3;

        // check that the length matches the data..
        Q_ASSERT(byte1 >= 0xf0 || getMessageLengthFromFirstByte((quint8) byte1) == 3);
    }

    MidiMessage::MidiMessage(const MidiMessage &other, const double newTimeStamp)
            : MidiMessage(other) {
        m_timeStamp = newTimeStamp;
    }

    MidiMessage::MidiMessage(const void *srcData, int sz, int &numBytesUsed, const quint8 lastStatusByte,
                             double t, bool sysexHasEmbeddedLength)
            : m_timeStamp(t) {
        auto src = static_cast<const quint8 *> (srcData);
        auto byte = (unsigned int) *src;

        if (byte < 0x80) {
            byte = (unsigned int) lastStatusByte;
            numBytesUsed = -1;
        } else {
            numBytesUsed = 0;
            --sz;
            ++src;
        }

        int size;

        if (byte >= 0x80) {
            if (byte == 0xf0) {
                auto d = src;
                bool haveReadAllLengthBytes = !sysexHasEmbeddedLength;
                int numVariableLengthSysexBytes = 0;

                while (d < src + sz) {
                    if (*d >= 0x80) {
                        if (*d == 0xf7) {
                            ++d;  // include the trailing 0xf7 when we hit it
                            break;
                        }

                        if (haveReadAllLengthBytes) // if we see a 0x80 bit set after the initial data length
                            break;                  // bytes, assume it's the end of the sysex

                        ++numVariableLengthSysexBytes;
                    } else if (!haveReadAllLengthBytes) {
                        haveReadAllLengthBytes = true;
                        ++numVariableLengthSysexBytes;
                    }

                    ++d;
                }

                src += numVariableLengthSysexBytes;
                size = 1 + (int) (d - src);

                m_data.resize(size);
                auto dest = m_data.data();
                *dest = (quint8) byte;
                memcpy(dest + 1, src, (size_t) (size - 1));

                numBytesUsed += (numVariableLengthSysexBytes + size);  // (these aren't counted in the size)
            } else if (byte == 0xff) {
                const auto bytesLeft = readVariableLengthValue(src + 1, sz - 1);
                size = qMin(sz + 1, bytesLeft.bytesUsed + 2 + bytesLeft.value);

                m_data.resize(size);
                auto dest = m_data.data();
                *dest = (quint8) byte;
                memcpy(dest + 1, src, (size_t) size - 1);

                numBytesUsed += size;
            } else {
                size = getMessageLengthFromFirstByte((quint8) byte);
                m_data[0] = (quint8) byte;

                if (size > 1) {
                    m_data[1] = (sz > 0 ? src[0] : 0);

                    if (size > 2)
                        m_data[2] = (sz > 1 ? src[1] : 0);
                }

                numBytesUsed += qMin(size, sz + 1);
            }
        } else {

        }
    }

    MidiMessage::~MidiMessage() noexcept = default;

    QString MidiMessage::getDescription() const {
        if (isNoteOn())
            return "Note on " + MidiMessage::getMidiNoteName(getNoteNumber(), true, true, 3) + " Velocity " +
                   QString::number(getVelocity()) + " Channel " + QString::number(getChannel());
        if (isNoteOff())
            return "Note off " + MidiMessage::getMidiNoteName(getNoteNumber(), true, true, 3) + " Velocity " +
                   QString::number(getVelocity()) + " Channel " + QString::number(getChannel());
        if (isProgramChange())
            return "Program change " + QString::number(getProgramChangeNumber()) + " Channel " + QString::number(getChannel());
        if (isPitchWheel()) return "Pitch wheel " + QString::number(getPitchWheelValue()) + " Channel " + QString::number(getChannel());
        if (isAftertouch())
            return "Aftertouch " + MidiMessage::getMidiNoteName(getNoteNumber(), true, true, 3) + ": " +
                   QString::number(getAfterTouchValue()) + " Channel " + QString::number(getChannel());
        if (isChannelPressure())
            return "Channel pressure " + QString::number(getChannelPressureValue()) + " Channel " + QString::number(getChannel());
        if (isAllNotesOff()) return "All notes off Channel " + QString::number(getChannel());
        if (isAllSoundOff()) return "All sound off Channel " + QString::number(getChannel());
        if (isMetaEvent()) return "Meta event";

        if (isController()) {
            QString name(MidiMessage::getControllerName(getControllerNumber()));

            if (name.isEmpty())
                name = QString::number(getControllerNumber());

            return "Controller " + name + ": " + QString::number(getControllerValue()) + " Channel " + QString::number(getChannel());
        }

        return QByteArray(reinterpret_cast<const char *>(getRawData()), getRawDataSize()).toHex();
    }

    MidiMessage MidiMessage::withTimeStamp(double newTimestamp) const {
        return {*this, newTimestamp};
    }

    int MidiMessage::getChannel() const noexcept {
        auto data = getRawData();

        if ((data[0] & 0xf0) != 0xf0)
            return (data[0] & 0xf) + 1;

        return 0;
    }

    bool MidiMessage::isForChannel(const int channel) const noexcept {
        Q_ASSERT(channel > 0 && channel <= 16); // valid channels are numbered 1 to 16

        auto data = getRawData();

        return ((data[0] & 0xf) == channel - 1)
               && ((data[0] & 0xf0) != 0xf0);
    }

    void MidiMessage::setChannel(const int channel) noexcept {
        Q_ASSERT(channel > 0 && channel <= 16); // valid channels are numbered 1 to 16

        auto data = m_data.data();

        if ((data[0] & 0xf0) != (quint8) 0xf0)
            data[0] = (quint8) ((data[0] & (quint8) 0xf0)
                                | (quint8) (channel - 1));
    }

    bool MidiMessage::isNoteOn(const bool returnTrueForVelocity0) const noexcept {
        auto data = getRawData();

        return ((data[0] & 0xf0) == 0x90)
               && (returnTrueForVelocity0 || data[2] != 0);
    }

    bool MidiMessage::isNoteOff(const bool returnTrueForNoteOnVelocity0) const noexcept {
        auto data = getRawData();

        return ((data[0] & 0xf0) == 0x80)
               || (returnTrueForNoteOnVelocity0 && (data[2] == 0) && ((data[0] & 0xf0) == 0x90));
    }

    bool MidiMessage::isNoteOnOrOff() const noexcept {
        auto d = getRawData()[0] & 0xf0;
        return (d == 0x90) || (d == 0x80);
    }

    int MidiMessage::getNoteNumber() const noexcept {
        return getRawData()[1];
    }

    void MidiMessage::setNoteNumber(const int newNoteNumber) noexcept {
        if (isNoteOnOrOff() || isAftertouch())
            m_data.data()[1] = (quint8) (newNoteNumber & 127);
    }

    quint8 MidiMessage::getVelocity() const noexcept {
        if (isNoteOnOrOff())
            return getRawData()[2];

        return 0;
    }

    float MidiMessage::getFloatVelocity() const noexcept {
        return getVelocity() * (1.0f / 127.0f);
    }

    void MidiMessage::setVelocity(const float newVelocity) noexcept {
        if (isNoteOnOrOff())
            m_data.data()[2] = floatValueToMidiByte(newVelocity);
    }

    void MidiMessage::multiplyVelocity(const float scaleFactor) noexcept {
        if (isNoteOnOrOff()) {
            auto data = m_data.data();
            data[2] = MidiHelpers::validVelocity(roundToInt(scaleFactor * data[2]));
        }
    }

    bool MidiMessage::isAftertouch() const noexcept {
        return (getRawData()[0] & 0xf0) == 0xa0;
    }

    int MidiMessage::getAfterTouchValue() const noexcept {
        Q_ASSERT(isAftertouch());
        return getRawData()[2];
    }

    MidiMessage MidiMessage::aftertouchChange(const int channel,
                                              const int noteNum,
                                              const int aftertouchValue) noexcept {
        Q_ASSERT(channel > 0 && channel <= 16); // valid channels are numbered 1 to 16
        Q_ASSERT(isPositiveAndBelow(noteNum, 128));
        Q_ASSERT(isPositiveAndBelow(aftertouchValue, 128));

        return MidiMessage(MidiHelpers::initialByte(0xa0, channel),
                           noteNum & 0x7f,
                           aftertouchValue & 0x7f);
    }

    bool MidiMessage::isChannelPressure() const noexcept {
        return (getRawData()[0] & 0xf0) == 0xd0;
    }

    int MidiMessage::getChannelPressureValue() const noexcept {
        Q_ASSERT(isChannelPressure());
        return getRawData()[1];
    }

    MidiMessage MidiMessage::channelPressureChange(const int channel, const int pressure) noexcept {
        Q_ASSERT(channel > 0 && channel <= 16); // valid channels are numbered 1 to 16
        Q_ASSERT(isPositiveAndBelow(pressure, 128));

        return MidiMessage(MidiHelpers::initialByte(0xd0, channel), pressure & 0x7f);
    }

    bool MidiMessage::isSustainPedalOn() const noexcept { return isControllerOfType(0x40) && getRawData()[2] >= 64; }

    bool MidiMessage::isSustainPedalOff() const noexcept { return isControllerOfType(0x40) && getRawData()[2] < 64; }

    bool MidiMessage::isSostenutoPedalOn() const noexcept { return isControllerOfType(0x42) && getRawData()[2] >= 64; }

    bool MidiMessage::isSostenutoPedalOff() const noexcept { return isControllerOfType(0x42) && getRawData()[2] < 64; }

    bool MidiMessage::isSoftPedalOn() const noexcept { return isControllerOfType(0x43) && getRawData()[2] >= 64; }

    bool MidiMessage::isSoftPedalOff() const noexcept { return isControllerOfType(0x43) && getRawData()[2] < 64; }


    bool MidiMessage::isProgramChange() const noexcept {
        return (getRawData()[0] & 0xf0) == 0xc0;
    }

    int MidiMessage::getProgramChangeNumber() const noexcept {
        Q_ASSERT(isProgramChange());
        return getRawData()[1];
    }

    MidiMessage MidiMessage::programChange(const int channel, const int programNumber) noexcept {
        Q_ASSERT(channel > 0 && channel <= 16); // valid channels are numbered 1 to 16

        return MidiMessage(MidiHelpers::initialByte(0xc0, channel), programNumber & 0x7f);
    }

    bool MidiMessage::isPitchWheel() const noexcept {
        return (getRawData()[0] & 0xf0) == 0xe0;
    }

    int MidiMessage::getPitchWheelValue() const noexcept {
        Q_ASSERT(isPitchWheel());
        auto data = getRawData();
        return data[1] | (data[2] << 7);
    }

    MidiMessage MidiMessage::pitchWheel(const int channel, const int position) noexcept {
        Q_ASSERT(channel > 0 && channel <= 16); // valid channels are numbered 1 to 16
        Q_ASSERT(isPositiveAndBelow(position, 0x4000));

        return MidiMessage(MidiHelpers::initialByte(0xe0, channel),
                           position & 127, (position >> 7) & 127);
    }

    bool MidiMessage::isController() const noexcept {
        return (getRawData()[0] & 0xf0) == 0xb0;
    }

    bool MidiMessage::isControllerOfType(const int controllerType) const noexcept {
        auto data = getRawData();
        return (data[0] & 0xf0) == 0xb0 && data[1] == controllerType;
    }

    int MidiMessage::getControllerNumber() const noexcept {
        Q_ASSERT(isController());
        return getRawData()[1];
    }

    int MidiMessage::getControllerValue() const noexcept {
        Q_ASSERT(isController());
        return getRawData()[2];
    }

    MidiMessage MidiMessage::controllerEvent(const int channel, const int controllerType, const int value) noexcept {
        // the channel must be between 1 and 16 inclusive
        Q_ASSERT(channel > 0 && channel <= 16);

        return MidiMessage(MidiHelpers::initialByte(0xb0, channel),
                           controllerType & 127, value & 127);
    }

    MidiMessage MidiMessage::noteOn(const int channel, const int noteNumber, const quint8 velocity) noexcept {
        Q_ASSERT(channel > 0 && channel <= 16);
        Q_ASSERT(isPositiveAndBelow(noteNumber, 128));

        return MidiMessage(MidiHelpers::initialByte(0x90, channel),
                           noteNumber & 127, MidiHelpers::validVelocity(velocity));
    }

    MidiMessage MidiMessage::noteOn(const int channel, const int noteNumber, const float velocity) noexcept {
        return noteOn(channel, noteNumber, floatValueToMidiByte(velocity));
    }

    MidiMessage MidiMessage::noteOff(const int channel, const int noteNumber, quint8 velocity) noexcept {
        Q_ASSERT(channel > 0 && channel <= 16);
        Q_ASSERT(isPositiveAndBelow(noteNumber, 128));

        return MidiMessage(MidiHelpers::initialByte(0x80, channel),
                           noteNumber & 127, MidiHelpers::validVelocity(velocity));
    }

    MidiMessage MidiMessage::noteOff(const int channel, const int noteNumber, float velocity) noexcept {
        return noteOff(channel, noteNumber, floatValueToMidiByte(velocity));
    }

    MidiMessage MidiMessage::noteOff(const int channel, const int noteNumber) noexcept {
        Q_ASSERT(channel > 0 && channel <= 16);
        Q_ASSERT(isPositiveAndBelow(noteNumber, 128));

        return MidiMessage(MidiHelpers::initialByte(0x80, channel), noteNumber & 127, 0);
    }

    MidiMessage MidiMessage::allNotesOff(const int channel) noexcept {
        return controllerEvent(channel, 123, 0);
    }

    bool MidiMessage::isAllNotesOff() const noexcept {
        auto data = getRawData();
        return (data[0] & 0xf0) == 0xb0 && data[1] == 123;
    }

    MidiMessage MidiMessage::allSoundOff(const int channel) noexcept {
        return controllerEvent(channel, 120, 0);
    }

    bool MidiMessage::isAllSoundOff() const noexcept {
        auto data = getRawData();
        return data[1] == 120 && (data[0] & 0xf0) == 0xb0;
    }

    bool MidiMessage::isResetAllControllers() const noexcept {
        auto data = getRawData();
        return (data[0] & 0xf0) == 0xb0 && data[1] == 121;
    }

    MidiMessage MidiMessage::allControllersOff(const int channel) noexcept {
        return controllerEvent(channel, 121, 0);
    }

    MidiMessage MidiMessage::masterVolume(const float volume) {
        auto vol = jlimit(0, 0x3fff, roundToInt(volume * 0x4000));

        return {0xf0, 0x7f, 0x7f, 0x04, 0x01, vol & 0x7f, vol >> 7, 0xf7};
    }

//==============================================================================
    bool MidiMessage::isSysEx() const noexcept {
        return *getRawData() == 0xf0;
    }

    MidiMessage MidiMessage::createSysExMessage(const void *sysexData, const int dataSize) {
        QVector<char> m(dataSize + 2);

        m[0] = 0xf0;
        memcpy(m.data() + 1, sysexData, (size_t) dataSize);
        m[dataSize + 1] = 0xf7;

        return MidiMessage(m.data(), dataSize + 2);
    }

    const quint8 *MidiMessage::getSysExData() const noexcept {
        return isSysEx() ? getRawData() + 1 : nullptr;
    }

    int MidiMessage::getSysExDataSize() const noexcept {
        return isSysEx() ? m_data.size() - 2 : 0;
    }

//==============================================================================
    bool MidiMessage::isMetaEvent() const noexcept { return *getRawData() == 0xff; }

    bool MidiMessage::isActiveSense() const noexcept { return *getRawData() == 0xfe; }

    int MidiMessage::getMetaEventType() const noexcept {
        auto data = getRawData();
        return (m_data.size() < 2 || *data != 0xff) ? -1 : data[1];
    }

    int MidiMessage::getMetaEventLength() const noexcept {
        auto data = getRawData();

        if (*data == 0xff) {
            const auto var = readVariableLengthValue(data + 2, m_data.size() - 2);
            return qMax(0, qMin(m_data.size() - 2 - var.bytesUsed, var.value));
        }

        return 0;
    }

    const quint8 *MidiMessage::getMetaEventData() const noexcept {
        Q_ASSERT(isMetaEvent());

        auto d = getRawData() + 2;
        const auto var = readVariableLengthValue(d, m_data.size() - 2);
        return d + var.bytesUsed;
    }

    bool MidiMessage::isTrackMetaEvent() const noexcept { return getMetaEventType() == 0; }

    bool MidiMessage::isEndOfTrackMetaEvent() const noexcept { return getMetaEventType() == 47; }

    bool MidiMessage::isTextMetaEvent() const noexcept {
        auto t = getMetaEventType();
        return t > 0 && t < 16;
    }

    QString MidiMessage::getTextFromTextMetaEvent() const {
        auto textData = reinterpret_cast<const char *> (getMetaEventData());

        return QByteArray(textData, getMetaEventLength());
    }

    MidiMessage MidiMessage::textMetaEvent(int type, const QString &text) {
        Q_ASSERT(type > 0 && type < 16);

        MidiMessage result;

        const size_t textSize = text.toUtf8().size() - 1;

        quint8 header[8];
        size_t n = sizeof(header);

        header[--n] = (quint8) (textSize & 0x7f);

        for (size_t i = textSize; (i >>= 7) != 0;)
            header[--n] = (quint8) ((i & 0x7f) | 0x80);

        header[--n] = (quint8) type;
        header[--n] = 0xff;

        const size_t headerLen = sizeof(header) - n;
        const int totalSize = (int) (headerLen + textSize);

        result.m_data.resize(totalSize);
        auto dest = result.m_data.data();

        memcpy(dest, header + n, headerLen);
        memcpy(dest + headerLen, text.toUtf8().data(), textSize);

        return result;
    }

    bool MidiMessage::isTrackNameEvent() const noexcept {
        auto data = getRawData();
        return (data[1] == 3) && (*data == 0xff);
    }

    bool MidiMessage::isTempoMetaEvent() const noexcept {
        auto data = getRawData();
        return (data[1] == 81) && (*data == 0xff);
    }

    bool MidiMessage::isMidiChannelMetaEvent() const noexcept {
        auto data = getRawData();
        return (data[1] == 0x20) && (*data == 0xff) && (data[2] == 1);
    }

    int MidiMessage::getMidiChannelMetaEventChannel() const noexcept {
        Q_ASSERT(isMidiChannelMetaEvent());
        return getRawData()[3] + 1;
    }

    double MidiMessage::getTempoSecondsPerQuarterNote() const noexcept {
        if (!isTempoMetaEvent())
            return 0.0;

        auto d = getMetaEventData();

        return (((unsigned int) d[0] << 16)
                | ((unsigned int) d[1] << 8)
                | d[2])
               / 1000000.0;
    }

    double MidiMessage::getTempoMetaEventTickLength(const short timeFormat) const noexcept {
        if (timeFormat > 0) {
            if (!isTempoMetaEvent())
                return 0.5 / timeFormat;

            return getTempoSecondsPerQuarterNote() / timeFormat;
        }

        const int frameCode = (-timeFormat) >> 8;
        double framesPerSecond;

        switch (frameCode) {
            case 24:
                framesPerSecond = 24.0;
                break;
            case 25:
                framesPerSecond = 25.0;
                break;
            case 29:
                framesPerSecond = 30.0 * 1000.0 / 1001.0;
                break;
            case 30:
                framesPerSecond = 30.0;
                break;
            default:
                framesPerSecond = 30.0;
                break;
        }

        return (1.0 / framesPerSecond) / (timeFormat & 0xff);
    }

    MidiMessage MidiMessage::tempoMetaEvent(int microsecondsPerQuarterNote) noexcept {
        return {0xff, 81, 3,
                (quint8) (microsecondsPerQuarterNote >> 16),
                (quint8) (microsecondsPerQuarterNote >> 8),
                (quint8) microsecondsPerQuarterNote};
    }

    bool MidiMessage::isTimeSignatureMetaEvent() const noexcept {
        auto data = getRawData();
        return (data[1] == 0x58) && (*data == (quint8) 0xff);
    }

    void MidiMessage::getTimeSignatureInfo(int &numerator, int &denominator) const noexcept {
        if (isTimeSignatureMetaEvent()) {
            auto d = getMetaEventData();
            numerator = d[0];
            denominator = 1 << d[1];
        } else {
            numerator = 4;
            denominator = 4;
        }
    }

    MidiMessage MidiMessage::timeSignatureMetaEvent(const int numerator, const int denominator) {
        int n = 1;
        int powerOfTwo = 0;

        while (n < denominator) {
            n <<= 1;
            ++powerOfTwo;
        }

        return {0xff, 0x58, 0x04, numerator, powerOfTwo, 1, 96};
    }

    MidiMessage MidiMessage::midiChannelMetaEvent(const int channel) noexcept {
        return {0xff, 0x20, 0x01, jlimit(0, 0xff, channel - 1)};
    }

    bool MidiMessage::isKeySignatureMetaEvent() const noexcept {
        return getMetaEventType() == 0x59;
    }

    int MidiMessage::getKeySignatureNumberOfSharpsOrFlats() const noexcept {
        return (int) (qint8) getMetaEventData()[0];
    }

    bool MidiMessage::isKeySignatureMajorKey() const noexcept {
        return getMetaEventData()[1] == 0;
    }

    MidiMessage MidiMessage::keySignatureMetaEvent(int numberOfSharpsOrFlats, bool isMinorKey) {
        Q_ASSERT(numberOfSharpsOrFlats >= -7 && numberOfSharpsOrFlats <= 7);

        return {0xff, 0x59, 0x02, numberOfSharpsOrFlats, isMinorKey ? 1 : 0};
    }

    MidiMessage MidiMessage::endOfTrack() noexcept {
        return {0xff, 0x2f, 0x00};
    }

//==============================================================================
    bool MidiMessage::isSongPositionPointer() const noexcept { return *getRawData() == 0xf2; }

    int MidiMessage::getSongPositionPointerMidiBeat() const noexcept {
        auto data = getRawData();
        return data[1] | (data[2] << 7);
    }

    MidiMessage MidiMessage::songPositionPointer(const int positionInMidiBeats) noexcept {
        return {0xf2,
                positionInMidiBeats & 127,
                (positionInMidiBeats >> 7) & 127};
    }

    bool MidiMessage::isMidiStart() const noexcept { return *getRawData() == 0xfa; }

    MidiMessage MidiMessage::midiStart() noexcept { return MidiMessage(0xfa); }

    bool MidiMessage::isMidiContinue() const noexcept { return *getRawData() == 0xfb; }

    MidiMessage MidiMessage::midiContinue() noexcept { return MidiMessage(0xfb); }

    bool MidiMessage::isMidiStop() const noexcept { return *getRawData() == 0xfc; }

    MidiMessage MidiMessage::midiStop() noexcept { return MidiMessage(0xfc); }

    bool MidiMessage::isMidiClock() const noexcept { return *getRawData() == 0xf8; }

    MidiMessage MidiMessage::midiClock() noexcept { return MidiMessage(0xf8); }

    bool MidiMessage::isQuarterFrame() const noexcept { return *getRawData() == 0xf1; }

    int MidiMessage::getQuarterFrameSequenceNumber() const noexcept { return ((int) getRawData()[1]) >> 4; }

    int MidiMessage::getQuarterFrameValue() const noexcept { return ((int) getRawData()[1]) & 0x0f; }

    MidiMessage MidiMessage::quarterFrame(const int sequenceNumber, const int value) noexcept {
        return MidiMessage(0xf1, (sequenceNumber << 4) | value);
    }

    bool MidiMessage::isFullFrame() const noexcept {
        auto data = getRawData();

        return data[0] == 0xf0
               && data[1] == 0x7f
               && m_data.size() >= 10
               && data[3] == 0x01
               && data[4] == 0x01;
    }

    void MidiMessage::getFullFrameParameters(int &hours, int &minutes, int &seconds, int &frames,
                                             MidiMessage::SmpteTimecodeType &timecodeType) const noexcept {
        Q_ASSERT(isFullFrame());

        auto data = getRawData();
        timecodeType = (SmpteTimecodeType) (data[5] >> 5);
        hours = data[5] & 0x1f;
        minutes = data[6];
        seconds = data[7];
        frames = data[8];
    }

    MidiMessage MidiMessage::fullFrame(int hours, int minutes, int seconds, int frames,
                                       MidiMessage::SmpteTimecodeType timecodeType) {
        return {0xf0, 0x7f, 0x7f, 0x01, 0x01,
                (hours & 0x01f) | (timecodeType << 5),
                minutes, seconds, frames,
                0xf7};
    }

    bool MidiMessage::isMidiMachineControlMessage() const noexcept {
        auto data = getRawData();

        return data[0] == 0xf0
               && data[1] == 0x7f
               && data[3] == 0x06
               && m_data.size() > 5;
    }

    MidiMessage::MidiMachineControlCommand MidiMessage::getMidiMachineControlCommand() const noexcept {
        Q_ASSERT(isMidiMachineControlMessage());

        return (MidiMachineControlCommand) getRawData()[4];
    }

    MidiMessage MidiMessage::midiMachineControlCommand(MidiMessage::MidiMachineControlCommand command) {
        return {0xf0, 0x7f, 0, 6, command, 0xf7};
    }

//==============================================================================
    bool MidiMessage::isMidiMachineControlGoto(int &hours, int &minutes, int &seconds, int &frames) const noexcept {
        auto data = getRawData();

        if (m_data.size() >= 12
            && data[0] == 0xf0
            && data[1] == 0x7f
            && data[3] == 0x06
            && data[4] == 0x44
            && data[5] == 0x06
            && data[6] == 0x01) {
            hours = data[7] % 24;   // (that some machines send out hours > 24)
            minutes = data[8];
            seconds = data[9];
            frames = data[10];

            return true;
        }

        return false;
    }

    MidiMessage MidiMessage::midiMachineControlGoto(int hours, int minutes, int seconds, int frames) {
        return {0xf0, 0x7f, 0, 6, 0x44, 6, 1, hours, minutes, seconds, frames, 0xf7};
    }

//==============================================================================
    QString MidiMessage::getMidiNoteName(int note, bool useSharps, bool includeOctaveNumber, int octaveNumForMiddleC) {
        static const char *const sharpNoteNames[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
        static const char *const flatNoteNames[] = {"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"};

        if (isPositiveAndBelow(note, 128)) {
            QString s(useSharps ? sharpNoteNames[note % 12]
                                : flatNoteNames[note % 12]);

            if (includeOctaveNumber)
                s += QString::number(note / 12 + (octaveNumForMiddleC - 5));

            return s;
        }

        return {};
    }

    double MidiMessage::getMidiNoteInHertz(const int noteNumber, const double frequencyOfA) noexcept {
        return frequencyOfA * std::pow(2.0, (noteNumber - 69) / 12.0);
    }

    bool MidiMessage::isMidiNoteBlack(int noteNumber) noexcept {
        return ((1 << (noteNumber % 12)) & 0x054a) != 0;
    }

    const char *MidiMessage::getGMInstrumentName(const int n) {
        static const char *names[] =
                {
                        ("Acoustic Grand Piano"), ("Bright Acoustic Piano"), ("Electric Grand Piano"),
                        ("Honky-tonk Piano"),
                        ("Electric Piano 1"), ("Electric Piano 2"), ("Harpsichord"), ("Clavinet"),
                        ("Celesta"), ("Glockenspiel"), ("Music Box"), ("Vibraphone"),
                        ("Marimba"), ("Xylophone"), ("Tubular Bells"), ("Dulcimer"),
                        ("Drawbar Organ"), ("Percussive Organ"), ("Rock Organ"), ("Church Organ"),
                        ("Reed Organ"), ("Accordion"), ("Harmonica"), ("Tango Accordion"),
                        ("Acoustic Guitar (nylon)"), ("Acoustic Guitar (steel)"), ("Electric Guitar (jazz)"),
                        ("Electric Guitar (clean)"),
                        ("Electric Guitar (mute)"), ("Overdriven Guitar"), ("Distortion Guitar"), ("Guitar Harmonics"),
                        ("Acoustic Bass"), ("Electric Bass (finger)"), ("Electric Bass (pick)"), ("Fretless Bass"),
                        ("Slap Bass 1"), ("Slap Bass 2"), ("Synth Bass 1"), ("Synth Bass 2"),
                        ("Violin"), ("Viola"), ("Cello"), ("Contrabass"),
                        ("Tremolo QStrings"), ("Pizzicato QStrings"), ("Orchestral Harp"), ("Timpani"),
                        ("QString Ensemble 1"), ("QString Ensemble 2"), ("SynthQStrings 1"), ("SynthQStrings 2"),
                        ("Choir Aahs"), ("Voice Oohs"), ("Synth Voice"), ("Orchestra Hit"),
                        ("Trumpet"), ("Trombone"), ("Tuba"), ("Muted Trumpet"),
                        ("French Horn"), ("Brass Section"), ("SynthBrass 1"), ("SynthBrass 2"),
                        ("Soprano Sax"), ("Alto Sax"), ("Tenor Sax"), ("Baritone Sax"),
                        ("Oboe"), ("English Horn"), ("Bassoon"), ("Clarinet"),
                        ("Piccolo"), ("Flute"), ("Recorder"), ("Pan Flute"),
                        ("Blown Bottle"), ("Shakuhachi"), ("Whistle"), ("Ocarina"),
                        ("Lead 1 (square)"), ("Lead 2 (sawtooth)"), ("Lead 3 (calliope)"), ("Lead 4 (chiff)"),
                        ("Lead 5 (charang)"), ("Lead 6 (voice)"), ("Lead 7 (fifths)"), ("Lead 8 (bass+lead)"),
                        ("Pad 1 (new age)"), ("Pad 2 (warm)"), ("Pad 3 (polysynth)"), ("Pad 4 (choir)"),
                        ("Pad 5 (bowed)"), ("Pad 6 (metallic)"), ("Pad 7 (halo)"), ("Pad 8 (sweep)"),
                        ("FX 1 (rain)"), ("FX 2 (soundtrack)"), ("FX 3 (crystal)"), ("FX 4 (atmosphere)"),
                        ("FX 5 (brightness)"), ("FX 6 (goblins)"), ("FX 7 (echoes)"), ("FX 8 (sci-fi)"),
                        ("Sitar"), ("Banjo"), ("Shamisen"), ("Koto"),
                        ("Kalimba"), ("Bag pipe"), ("Fiddle"), ("Shanai"),
                        ("Tinkle Bell"), ("Agogo"), ("Steel Drums"), ("Woodblock"),
                        ("Taiko Drum"), ("Melodic Tom"), ("Synth Drum"), ("Reverse Cymbal"),
                        ("Guitar Fret Noise"), ("Breath Noise"), ("Seashore"), ("Bird Tweet"),
                        ("Telephone Ring"), ("Helicopter"), ("Applause"), ("Gunshot")
                };

        return isPositiveAndBelow(n, numElementsInArray(names)) ? names[n] : nullptr;
    }

    const char *MidiMessage::getGMInstrumentBankName(const int n) {
        static const char *names[] =
                {
                        ("Piano"), ("Chromatic Percussion"), ("Organ"), ("Guitar"),
                        ("Bass"), ("QStrings"), ("Ensemble"), ("Brass"),
                        ("Reed"), ("Pipe"), ("Synth Lead"), ("Synth Pad"),
                        ("Synth Effects"), ("Ethnic"), ("Percussive"), ("Sound Effects")
                };

        return isPositiveAndBelow(n, numElementsInArray(names)) ? names[n] : nullptr;
    }

    const char *MidiMessage::getRhythmInstrumentName(const int n) {
        static const char *names[] =
                {
                        ("Acoustic Bass Drum"), ("Bass Drum 1"), ("Side Stick"), ("Acoustic Snare"),
                        ("Hand Clap"), ("Electric Snare"), ("Low Floor Tom"), ("Closed Hi-Hat"),
                        ("High Floor Tom"), ("Pedal Hi-Hat"), ("Low Tom"), ("Open Hi-Hat"),
                        ("Low-Mid Tom"), ("Hi-Mid Tom"), ("Crash Cymbal 1"), ("High Tom"),
                        ("Ride Cymbal 1"), ("Chinese Cymbal"), ("Ride Bell"), ("Tambourine"),
                        ("Splash Cymbal"), ("Cowbell"), ("Crash Cymbal 2"), ("Vibraslap"),
                        ("Ride Cymbal 2"), ("Hi Bongo"), ("Low Bongo"), ("Mute Hi Conga"),
                        ("Open Hi Conga"), ("Low Conga"), ("High Timbale"), ("Low Timbale"),
                        ("High Agogo"), ("Low Agogo"), ("Cabasa"), ("Maracas"),
                        ("Short Whistle"), ("Long Whistle"), ("Short Guiro"), ("Long Guiro"),
                        ("Claves"), ("Hi Wood Block"), ("Low Wood Block"), ("Mute Cuica"),
                        ("Open Cuica"), ("Mute Triangle"), ("Open Triangle")
                };

        return (n >= 35 && n <= 81) ? names[n - 35] : nullptr;
    }

    const char *MidiMessage::getControllerName(const int n) {
        static const char *names[] =
                {
                        ("Bank Select"), ("Modulation Wheel (coarse)"), ("Breath controller (coarse)"),
                        nullptr,
                        ("Foot Pedal (coarse)"), ("Portamento Time (coarse)"), ("Data Entry (coarse)"),
                        ("Volume (coarse)"), ("Balance (coarse)"),
                        nullptr,
                        ("Pan position (coarse)"), ("Expression (coarse)"), ("Effect Control 1 (coarse)"),
                        ("Effect Control 2 (coarse)"),
                        nullptr, nullptr,
                        ("General Purpose Slider 1"), ("General Purpose Slider 2"),
                        ("General Purpose Slider 3"), ("General Purpose Slider 4"),
                        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                        nullptr, nullptr,
                        ("Bank Select (fine)"), ("Modulation Wheel (fine)"), ("Breath controller (fine)"),
                        nullptr,
                        ("Foot Pedal (fine)"), ("Portamento Time (fine)"), ("Data Entry (fine)"), ("Volume (fine)"),
                        ("Balance (fine)"), nullptr, ("Pan position (fine)"), ("Expression (fine)"),
                        ("Effect Control 1 (fine)"), ("Effect Control 2 (fine)"),
                        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                        ("Hold Pedal (on/off)"), ("Portamento (on/off)"), ("Sustenuto Pedal (on/off)"),
                        ("Soft Pedal (on/off)"),
                        ("Legato Pedal (on/off)"), ("Hold 2 Pedal (on/off)"), ("Sound Variation"), ("Sound Timbre"),
                        ("Sound Release Time"), ("Sound Attack Time"), ("Sound Brightness"), ("Sound Control 6"),
                        ("Sound Control 7"), ("Sound Control 8"), ("Sound Control 9"), ("Sound Control 10"),
                        ("General Purpose Button 1 (on/off)"), ("General Purpose Button 2 (on/off)"),
                        ("General Purpose Button 3 (on/off)"), ("General Purpose Button 4 (on/off)"),
                        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                        ("Reverb Level"), ("Tremolo Level"), ("Chorus Level"), ("Celeste Level"),
                        ("Phaser Level"), ("Data Button increment"), ("Data Button decrement"),
                        ("Non-registered Parameter (fine)"),
                        ("Non-registered Parameter (coarse)"), ("Registered Parameter (fine)"),
                        ("Registered Parameter (coarse)"),
                        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                        nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
                        ("All Sound Off"), ("All Controllers Off"), ("Local Keyboard (on/off)"), ("All Notes Off"),
                        ("Omni Mode Off"), ("Omni Mode On"), ("Mono Operation"), ("Poly Operation")
                };

        return isPositiveAndBelow(n, numElementsInArray(names)) ? names[n] : nullptr;
    }

} // namespace talcs
