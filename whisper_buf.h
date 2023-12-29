#pragma once

#include <iostream>
#include <sstream>
#include <csignal>
#include <thread>

#include "../whisper.cpp/whisper.h"
#include "../common-sdl.h"

// command-line parameters
struct whisper_params {
    int32_t n_threads  = std::min(4, (int32_t) std::thread::hardware_concurrency());
    int32_t step_ms    = 3000;
    int32_t length_ms  = 10000;
    int32_t keep_ms    = 200;
    int32_t capture_id = -1;
    int32_t max_tokens = 32;
    int32_t audio_ctx  = 0;

    float vad_thold    = 0.6f;
    float freq_thold   = 100.0f;

    bool speed_up      = false;
    bool translate     = false;
    bool no_fallback   = false;
    bool print_special = false;
    bool no_context    = true;
    bool no_timestamps = false;
    bool tinydiarize   = false;

    std::string language  = "en";
    std::string model     = "models/ggml-base.en.bin";
    std::string fname_out;
    bool save_audio = false; // save audio to wav file
};

class WhisperBuf : public std::streambuf
{
private:
    std::string line = "";
    char ch{}; // single-byte buffer
    whisper_params params;

protected:
    int underflow() override
    {
        if(line.empty())
        {
            
        }
        ch = line[0];
        line.erase(0, 1);
        setg(&ch, &ch, &ch + 1); // make one read position available
        return ch; 
    }
public:
    WhisperBuf()
    {
        audio_async audio(params.length_ms);
        if (!audio.init(params.capture_id, WHISPER_SAMPLE_RATE))
        {
            fprintf(stderr, "%s: audio.init() failed!\n", __func__);
        }
    }
};