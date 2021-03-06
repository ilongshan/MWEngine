/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2017-2019 Igor Zinken - https://www.igorski.nl
 *
 * AAudio driver implementation adapted from the Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
#ifndef AAUDIO_PLAYAUDIOENGINE_H
#define AAUDIO_PLAYAUDIOENGINE_H

#include <aaudio/AAudio.h>
#include <thread>
#include <mutex>

namespace MWEngine {

#define BUFFER_SIZE_AUTOMATIC 0
// Time constants
#define NANOS_PER_SECOND 1000000000L
#define NANOS_PER_MILLISECOND 1000000L

void PrintAudioStreamInfo(const AAudioStream * stream);

int64_t timestamp_to_nanoseconds(timespec ts);
int64_t get_time_nanoseconds(clockid_t clockid);

class AAudio_IO
{
    public:
        AAudio_IO( int amountOfInputChannels, int amountOfOutputChannels );
        ~AAudio_IO();
        void setDeviceId          ( int32_t deviceId );
        void setRecordingDeviceId ( int32_t recordingDeviceId );
        void setBufferSizeInBursts( int32_t numBursts );
        aaudio_data_callback_result_t dataCallback( AAudioStream *stream,
                                                    void *audioData,
                                                    int32_t numFrames );
        void errorCallback( AAudioStream *stream, aaudio_result_t  __unused error );
        double getCurrentOutputLatencyMillis();
        int getEnqueuedInputBuffer( float* destinationBuffer, int amountOfSamples );
        void enqueueOutputBuffer  ( float* sourceBuffer,      int amountOfSamples );
        bool render;

    private:

        // By not specifying an audio device id we are telling AAudio that
       // we want the stream to be created using the default playback audio device.
        int32_t _outputDeviceId  = AAUDIO_UNSPECIFIED;
        int32_t _inputDeviceId   = AAUDIO_UNSPECIFIED;
        
        int32_t _sampleRate;
        int16_t _inputChannelCount;
        int16_t _outputChannelCount;
        aaudio_format_t _sampleFormat;
        float*   _enqueuedOutputBuffer = nullptr;
        float*   _recordBuffer         = nullptr;
        int16_t* _recordBufferI        = nullptr;
        bool _flushInputOnCallback     = true;

        AAudioStream* _inputStream  = nullptr;
        AAudioStream* _outputStream = nullptr;

        int32_t _underrunCountOutputStream;
        int32_t _bufferSizeInFrames;
        int32_t _framesPerBurst;
        double currentOutputLatencyMillis_ = 0;
        int32_t _bufferSizeSelection       = BUFFER_SIZE_AUTOMATIC;

        std::thread* _streamRestartThread;
        std::mutex   _restartingLock;

        void createInputStream();
        void createOutputStream();
        void createAllStreams();
        void startStream( AAudioStream* stream );
        void stopStream ( AAudioStream* stream );
        void closeStream( AAudioStream* stream );
        void closeAllStreams();
        void flushInputStream( void *audioData, int32_t numFrames );
        void restartStreams();

        AAudioStreamBuilder* createStreamBuilder();
        void setupOutputStream( AAudioStreamBuilder* builder );
        void setupInputStream ( AAudioStreamBuilder* builder );
        void updateBufferSizeInFrames( int bufferSize );

        aaudio_result_t calculateCurrentOutputLatencyMillis(AAudioStream *stream, double *latencyMillis);
    };

} // E.O namespace MWEngine

#endif
