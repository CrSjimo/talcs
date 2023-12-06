/******************************************************************************
 * Copyright (c) 2023 CrSjimo                                                 *
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

#ifndef TALCS_MIDIMESSAGE_H
#define TALCS_MIDIMESSAGE_H

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

#include <TalcsMidi/TalcsMidiGlobal.h>
#include <QString>
#include <QVector>

namespace talcs {

    class TALCSMIDI_EXPORT MidiMessage {
    public:
        MidiMessage(int byte1, int byte2, int byte3, double timeStamp = 0) noexcept;

        MidiMessage(int byte1, int byte2, double timeStamp = 0) noexcept;

        MidiMessage(int byte1, double timeStamp = 0) noexcept;

        template<typename... Data>
        MidiMessage(int byte1, int byte2, int byte3, Data... otherBytes)  : m_data(3 + sizeof... (otherBytes)) {
            // this checks that the length matches the data..
            Q_ASSERT(m_data.size() > 3 || byte1 >= 0xf0 || getMessageLengthFromFirstByte((quint8) byte1) == m_data.size());

            const quint8 data[] = {(quint8) byte1, (quint8) byte2, (quint8) byte3,
                                   static_cast<quint8> (otherBytes)...};
            memcpy(m_data.data(), data, m_data.size());
        }

        MidiMessage(const void *data, int numBytes, double timeStamp = 0);

        MidiMessage(const void *data, int maxBytesToUse,
                    int &numBytesUsed, quint8 lastStatusByte,
                    double timeStamp = 0,
                    bool sysexHasEmbeddedLength = true);

        MidiMessage() noexcept;

        MidiMessage(const MidiMessage &, double newTimeStamp);

        ~MidiMessage() noexcept;

        const quint8 *getRawData() const noexcept { return m_data.data(); }

        int getRawDataSize() const noexcept { return m_data.size(); }

        QString getDescription() const;

        double getTimeStamp() const noexcept { return m_timeStamp; }

        void setTimeStamp(double newTimestamp) noexcept { m_timeStamp = newTimestamp; }

        void addToTimeStamp(double delta) noexcept { m_timeStamp += delta; }

        MidiMessage withTimeStamp(double newTimestamp) const;

        int getChannel() const noexcept;

        bool isForChannel(int channelNumber) const noexcept;

        void setChannel(int newChannelNumber) noexcept;

        bool isSysEx() const noexcept;

        const quint8 *getSysExData() const noexcept;

        int getSysExDataSize() const noexcept;

        bool isNoteOn(bool returnTrueForVelocity0 = false) const noexcept;

        static MidiMessage noteOn(int channel, int noteNumber, float velocity) noexcept;

        static MidiMessage noteOn(int channel, int noteNumber, quint8 velocity) noexcept;

        bool isNoteOff(bool returnTrueForNoteOnVelocity0 = true) const noexcept;

        static MidiMessage noteOff(int channel, int noteNumber, float velocity) noexcept;

        static MidiMessage noteOff(int channel, int noteNumber, quint8 velocity) noexcept;

        static MidiMessage noteOff(int channel, int noteNumber) noexcept;

        bool isNoteOnOrOff() const noexcept;

        int getNoteNumber() const noexcept;

        void setNoteNumber(int newNoteNumber) noexcept;

        quint8 getVelocity() const noexcept;

        float getFloatVelocity() const noexcept;

        void setVelocity(float newVelocity) noexcept;

        void multiplyVelocity(float scaleFactor) noexcept;


        bool isSustainPedalOn() const noexcept;

        bool isSustainPedalOff() const noexcept;

        bool isSostenutoPedalOn() const noexcept;

        bool isSostenutoPedalOff() const noexcept;

        bool isSoftPedalOn() const noexcept;

        bool isSoftPedalOff() const noexcept;

        bool isProgramChange() const noexcept;

        int getProgramChangeNumber() const noexcept;

        static MidiMessage programChange(int channel, int programNumber) noexcept;

        bool isPitchWheel() const noexcept;

        int getPitchWheelValue() const noexcept;

        static MidiMessage pitchWheel(int channel, int position) noexcept;

        bool isAftertouch() const noexcept;

        int getAfterTouchValue() const noexcept;

        static MidiMessage aftertouchChange(int channel,
                                            int noteNumber,
                                            int aftertouchAmount) noexcept;

        bool isChannelPressure() const noexcept;

        int getChannelPressureValue() const noexcept;

        static MidiMessage channelPressureChange(int channel, int pressure) noexcept;

        bool isController() const noexcept;

        int getControllerNumber() const noexcept;

        int getControllerValue() const noexcept;

        bool isControllerOfType(int controllerType) const noexcept;

        static MidiMessage controllerEvent(int channel,
                                           int controllerType,
                                           int value) noexcept;

        bool isAllNotesOff() const noexcept;

        bool isAllSoundOff() const noexcept;

        bool isResetAllControllers() const noexcept;

        static MidiMessage allNotesOff(int channel) noexcept;

        static MidiMessage allSoundOff(int channel) noexcept;

        static MidiMessage allControllersOff(int channel) noexcept;

        bool isMetaEvent() const noexcept;

        int getMetaEventType() const noexcept;

        const quint8 *getMetaEventData() const noexcept;

        int getMetaEventLength() const noexcept;


        bool isTrackMetaEvent() const noexcept;

        bool isEndOfTrackMetaEvent() const noexcept;

        static MidiMessage endOfTrack() noexcept;

        bool isTrackNameEvent() const noexcept;

        bool isTextMetaEvent() const noexcept;

        QString getTextFromTextMetaEvent() const;

        static MidiMessage textMetaEvent(int type, QStringRef text);

        bool isTempoMetaEvent() const noexcept;

        double getTempoMetaEventTickLength(short timeFormat) const noexcept;

        double getTempoSecondsPerQuarterNote() const noexcept;

        static MidiMessage tempoMetaEvent(int microsecondsPerQuarterNote) noexcept;

        bool isTimeSignatureMetaEvent() const noexcept;

        void getTimeSignatureInfo(int &numerator, int &denominator) const noexcept;

        static MidiMessage timeSignatureMetaEvent(int numerator, int denominator);

        bool isKeySignatureMetaEvent() const noexcept;

        int getKeySignatureNumberOfSharpsOrFlats() const noexcept;

        bool isKeySignatureMajorKey() const noexcept;

        static MidiMessage keySignatureMetaEvent(int numberOfSharpsOrFlats, bool isMinorKey);

        bool isMidiChannelMetaEvent() const noexcept;

        int getMidiChannelMetaEventChannel() const noexcept;

        static MidiMessage midiChannelMetaEvent(int channel) noexcept;


        bool isActiveSense() const noexcept;

        bool isMidiStart() const noexcept;

        static MidiMessage midiStart() noexcept;

        bool isMidiContinue() const noexcept;

        static MidiMessage midiContinue() noexcept;

        bool isMidiStop() const noexcept;

        static MidiMessage midiStop() noexcept;

        bool isMidiClock() const noexcept;

        static MidiMessage midiClock() noexcept;

        bool isSongPositionPointer() const noexcept;

        int getSongPositionPointerMidiBeat() const noexcept;

        static MidiMessage songPositionPointer(int positionInMidiBeats) noexcept;

        bool isQuarterFrame() const noexcept;

        int getQuarterFrameSequenceNumber() const noexcept;

        int getQuarterFrameValue() const noexcept;

        static MidiMessage quarterFrame(int sequenceNumber, int value) noexcept;

        enum SmpteTimecodeType {
            fps24 = 0,
            fps25 = 1,
            fps30drop = 2,
            fps30 = 3
        };

        bool isFullFrame() const noexcept;

        void getFullFrameParameters(int &hours,
                                    int &minutes,
                                    int &seconds,
                                    int &frames,
                                    SmpteTimecodeType &timecodeType) const noexcept;

        static MidiMessage fullFrame(int hours,
                                     int minutes,
                                     int seconds,
                                     int frames,
                                     SmpteTimecodeType timecodeType);

        enum MidiMachineControlCommand {
            mmc_stop = 1,
            mmc_play = 2,
            mmc_deferredplay = 3,
            mmc_fastforward = 4,
            mmc_rewind = 5,
            mmc_recordStart = 6,
            mmc_recordStop = 7,
            mmc_pause = 9
        };

        bool isMidiMachineControlMessage() const noexcept;

        MidiMachineControlCommand getMidiMachineControlCommand() const noexcept;

        static MidiMessage midiMachineControlCommand(MidiMachineControlCommand command);

        bool isMidiMachineControlGoto(int &hours,
                                      int &minutes,
                                      int &seconds,
                                      int &frames) const noexcept;

        static MidiMessage midiMachineControlGoto(int hours,
                                                  int minutes,
                                                  int seconds,
                                                  int frames);

        static MidiMessage masterVolume(float volume);

        static MidiMessage createSysExMessage(const void *sysexData,
                                              int dataSize);

#ifndef DOXYGEN

        [[deprecated ("This signature has been deprecated in favour of the safer readVariableLengthValue.")]]
        static int readVariableLengthVal(const quint8 *data, int &numBytesUsed) noexcept;

#endif

        struct VariableLengthValue {
            VariableLengthValue() =
            default;

            VariableLengthValue(int
                                valueIn, int
                                bytesUsedIn)
                    : value(valueIn), bytesUsed(bytesUsedIn) {}

            bool isValid() const noexcept { return bytesUsed > 0; }

            int value = 0;
            int bytesUsed = 0;
        };

        static VariableLengthValue readVariableLengthValue(const quint8 *data,
                                                           int maxBytesToUse) noexcept;

        static int getMessageLengthFromFirstByte(quint8 firstByte) noexcept;

        static QString getMidiNoteName(int noteNumber,
                                       bool useSharps,
                                       bool includeOctaveNumber,
                                       int octaveNumForMiddleC);

        static double getMidiNoteInHertz(int noteNumber, double frequencyOfA = 440.0) noexcept;

        static bool isMidiNoteBlack(int noteNumber) noexcept;

        static const char *getGMInstrumentName(int midiInstrumentNumber);

        static const char *getGMInstrumentBankName(int midiBankNumber);

        static const char *getRhythmInstrumentName(int midiNoteNumber);

        static const char *getControllerName(int controllerNumber);

        static quint8 floatValueToMidiByte(float valueBetween0and1) noexcept;

        static quint16 pitchbendToPitchwheelPos(float pitchbendInSemitones,
                                                float pitchbendRangeInSemitones) noexcept;

    private:
        QVector<quint8> m_data;
        double m_timeStamp = 0;
    };

} // namespace talcs

#endif //TALCS_MIDIMESSAGE_H
