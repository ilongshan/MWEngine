/**
 * The MIT License (MIT)
 *
 * Copyright (c) 2017-2019 Igor Zinken - http://www.igorski.nl
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
#include "adapter.h"
#include "../audioengine.h"
#include <utilities/debug.h>

namespace MWEngine {
namespace DriverAdapter {

#if DRIVER == 0
    OPENSL_STREAM* driver_openSL = nullptr; // OpenSL
#elif DRIVER == 1
    AAudio_IO* driver_aAudio = nullptr;     // AAudio
#endif

    bool create() {

#if DRIVER == 0

        Debug::log( "DriverAdapter::initializing OpenSL driver");

        // OpenSL
        driver_openSL = android_OpenAudioDevice(
            AudioEngineProps::SAMPLE_RATE, AudioEngineProps::INPUT_CHANNELS,
            AudioEngineProps::OUTPUT_CHANNELS, AudioEngineProps::BUFFER_SIZE
        );
        return ( driver_openSL != NULL );

#elif DRIVER == 1

        Debug::log( "DriverAdapter::initializing AAudio driver");

        // AAudio
        driver_aAudio = new AAudio_IO(
            AudioEngineProps::INPUT_CHANNELS, AudioEngineProps::OUTPUT_CHANNELS
        );

        if ( driver_aAudio == nullptr ) {
            return false;
        }
        // TODO: allow specifying these from the outside?
        // driver_aAudio->setDeviceId();
        // driver_aAudio->setRecordingDeviceId();
        driver_aAudio->setBufferSizeInBursts( 1 ); // Google provides {0, 1, 2, 4, 8} as values

        return true;
#endif

    }

    void destroy() {

#if DRIVER == 0
        // OpenSL
        if ( driver_openSL != nullptr ) {
            android_CloseAudioDevice( driver_openSL );
        }
        driver_openSL = nullptr;
#elif DRIVER == 1
        // AAudio
        delete driver_aAudio;
        driver_aAudio = nullptr;
#endif

    }

    void render() {

#if DRIVER == 0
        // OpenSL maintains its own locking mechanism, we can invoke
        // the render cycle directly from the audio engine thread loop
        AudioEngine::render( AudioEngineProps::BUFFER_SIZE );
#elif DRIVER == 1
        // AAudio triggers its callback internally when ready
        // AAudio driver will request render() on its own
        driver_aAudio->render = true;
#endif
    }

    void writeOutput( float* outputBuffer, int amountOfSamples ) {

#if DRIVER == 0
        // OpenSL
        android_AudioOut( driver_openSL, outputBuffer, amountOfSamples );
#elif DRIVER == 1
        // AAudio
        driver_aAudio->enqueueOutputBuffer( outputBuffer, amountOfSamples );
#endif
    }

    int getInput( float* recordBuffer, int amountOfSamples ) {

#if DRIVER == 0
        // OpenSL
        return android_AudioIn( driver_openSL, recordBuffer, AudioEngineProps::BUFFER_SIZE );
#elif DRIVER == 1
        // AAudio
        return driver_aAudio->getEnqueuedInputBuffer( recordBuffer, amountOfSamples );
#endif
        return 0;
    }
}

} // E.O namespace MWEngine
