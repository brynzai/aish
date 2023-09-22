#ifdef AUDIO_MODE

#include <iostream>
#include <festival/festival.h>

// Note include path must add speech_tools for festival dependency EST.h

using namespace std;

// Text to Speech stringbuf
// Encapsulate this with an ostream to allow a simple ostream* alternative to cout, or fstream.
// John Boero
class TTSBuf : public stringbuf
{
  public:
    TTSBuf(string voice = "cmu_us_awb_cg")
    {
        string cmd = "(voice_";
        festival_initialize(1, 10000000);
        festival_eval_command((cmd + voice + ')').c_str());
    }

    ~TTSBuf()
    {
        festival_tidy_up();
    }

    virtual int sync()
    {
        int success;
        cout << str() << flush;
        success = festival_say_text(str().c_str());
        str("");
        return success;
    }
};

#endif