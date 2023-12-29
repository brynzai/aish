#pragma once

#include <festival/festival.h>
#include <sstream>
#include <iostream>

// Note include path must add speech_tools for festival dependency EST.h

// Text to Speech stringbuf
// Encapsulate this with an ostream to allow a simple ostream* alternative to cout, or fstream.
// John Boero
class TTSBuf : public std::stringbuf
{
  public:
    TTSBuf(std::string voice = "cmu_us_awb_cg")
    {
        std::string cmd = "(voice_";
        festival_initialize(1, 10000000);
        festival_eval_command((cmd + voice + ')').c_str());
    }

    ~TTSBuf()
    {
        festival_tidy_up();
    }

    virtual int sync() override
    {
        int success = festival_say_text(str().c_str());
        std::cout << str() << std::flush;
        str("");
        return success;
    }
};
