/*=====================================
		AI Shell by John Boero
		BrynzAI - 2023
=====================================*/
//#define CURL_STATICLIB

#include "aish.h"
#include "ttsstream.h"

#include <sstream>
#include <fstream>
#include <regex>
#include <map>
#include <sys/stat.h>
#include <signal.h>
#include "aish.h"
//#include <ncurses.h> // FUTURE

using namespace std; // Sod the style guide.

// Globals... so sue me?
ostream *logs = &std::cout;

// Paragraph mode parsing?
// This implies IFS is a blank line.
bool paragraph, debug;
float temperature = 0.2f;
string global_thread = "This is a conversation between user and one or more chatbots. Thread history:\n",
	ps1 = getenvsafe("PS1") + "aish: ", 
	mode = "shellbard";

// Ignore signals for now.
void signals(int sig_num) {}

int main (int argc, char **argv)
{
	int res;
	string cmd;
	string stemp = getenvsafe("AISH_TEMP");
	stringstream buffer;
	istream *input = &cin;
	ifstream scriptFile;
	ofstream history;

	// Ignore ^C SIGINT for now.
	//signal(SIGINT, signals);

	if (stemp != "")
		temperature = stof(stemp);

    while(true)
    {
        // note the colon (:) to indicate that 'b' has a parameter and is not a switch
        switch(getopt(argc, argv, "m:dhpvxac?"))
        {
            case 'x':
			debug = true;
            #if DEBUG
			cerr << GREY << "Debug/trace output on." << RESET << endl;
            #endif
			continue;

            case 'm':
            mode = optarg;
			mode = regex_replace(mode, regex("(^[ ]+)|([ ]+$)"),"");
			if (!AishPlugin::GetPlugin(mode))
			{
				cerr << "ERROR: Mode " << mode << " is not supported by this version of aish. Available mode plugins: ";
				AishPlugin::PrintAll();
				exit(1);
			}
			continue;

			case 'v':
			cout << "aish v0.2.0" << endl;
			return 0;
			
			case 'p':
			paragraph = true;
			continue;

			case 'c':
			// TODO immediate mode. cin << argv
			continue;

			case 'a':
			#ifdef AUDIO_MODE
			// Set logs to a Festival text to speech ostream.
			logs = new ostream(new TTSBuf);
			#else
			cerr << "WARN: this build does not support Festival text to speech." << endl;
			#endif
			continue;

            case '?':
            case 'h':
            default :
            *logs << R"USAGE(Usage: aish [-xp] [-m MODE]
  -m mode is one of: chatbard|chatgpt|shellbard|shellgpt and defaults to shellbard.
Environment variables required for Bard: CLOUDSDK_CORE_PROJECT, GOOGLE_APPLICATION_CREDENTIALS
Environment variables required for GPT: OPENAI_ORG, OPENAI_API_KEY
  -v shows version and quits.
  -a Audio mode - voice support.
  -p paragraph mode: Use a blank line to submit.
  -x trace mode similar to bash.
WARNING use AI for shell carefully and at you own risk.
List of plugins supported by this build:)USAGE" << endl;
			AishPlugin::PrintAll();
            return 0;
			break;

            case -1:
            break;
        }

        break;
    }

	// Make sure to warn people about unexpected AI shell behaviour.
	// Test the file ~/.aish/accept exists.
	if (regex_match(mode, regex(".*shell.*")))
	{
		ifstream accepted(getenvsafe("HOME") + "/.aish/accept");
		if(!accepted) {
			cerr << YELLOW << "WARNING use AI for shell carefully and at you own risk."
				<< endl << "touch ~/.aish/accept to hide this in the future." << RESET << endl;
		}
	}

	
	// Did we specify a file after args?
	if (optind < argc)
	{
		try {
			string fname = argv[optind];
			scriptFile = ifstream(fname);
			if (!scriptFile)
			{
				cerr << RED << "ERROR opening script: " << fname << RESET << endl;
				return 1;
			}
			input = &scriptFile;
			// Discard shebang line. PS1 blank.
			getline(*input, fname);
			ps1 = "";
		}
		catch (exception const &e){
			cerr << RED << "ERROR exeption opening input file: " << e.what() << RESET << endl;
			return 1;
		}
	}

	// Create or ignore error if dir already exists.
	// Handlers must check ability to create temp content.
	mkdir((getenvsafe("HOME") + "/.aish").c_str(), 0700);
	history.open(getenvsafe("HOME") + "/.aish/aish_history");
	
	if (ps1.length())
		ps1 = YELLOW+mode+"🙂"+RESET+"> ";
	cout << ps1 << flush;

	while (getline(*input, cmd))
	{
		// Trim white space and comment. Escape quotes.
		cmd = regex_replace(cmd, (regex)"^\\s+|\\s+$|\\s*#.*$", "");
		cmd = regex_replace(cmd, regex("\""), "\\\"");

		// Live mode switch feature (group chat)
		// Take the first word of cmd, to_lower and strip punctuation.
		// If we're trying to speak to another mode, switch mode...
		try
		{
			stringstream cmdstream(cmd);
			string firstword;
			cmdstream >> firstword;
			transform(firstword.begin(), firstword.end(), firstword.begin(),
				[](unsigned char c){ return tolower(c); });
			firstword = regex_replace(firstword, regex("[[:punct:]]"), "");

			if (AishPlugin::GetPlugin(firstword))
			{
				mode = firstword;
				*logs << YELLOW << mode << RESET << "🫡: ";
				if (ps1.length())
					ps1 = YELLOW+mode+"🙂"+RESET+"> ";
			}
		}
		catch(const std::exception& e)
		{
			cerr <<YELLOW<< "WARN Unable to detect or switch mode: " << e.what() <<RESET<< endl;
		}
		
		// Paragraph mode parsing isn't straightforward.
		if (paragraph)
		{
			if (cmd != "")
			{
				buffer << cmd << "\\n";
				continue;
			}
			else
			{
				if (!buffer.tellp())
					continue;
				else
					getline(buffer, cmd);
			}
		}

		if (cmd.length())
		{
			// Specialty commands, exit, cd, etc.
			if (cmd == "quit" || cmd == "exit")
				break;
			
			if (regex_match(cmd, (regex)"^cd\\s*.*?"))
			{
				cmd = regex_replace(cmd, (regex)"^cd\\s*", "");
				if (chdir(cmd.c_str()))
					cerr << RED << "Failed to change directory: ENO " << errno << RESET << endl;
				*logs << ps1;
				continue;
			}

			// Simple replace params with the rest of argc.
			// Note extra params beyond argc and variables will currently not be replaced!
			for (int param = 1; ++optind < argc; ++param)
			{
				string paramstr = "\\$" + to_string(param);
				cmd = regex_replace(cmd, (regex)paramstr, argv[optind]);
			}

			AishPlugin::plugFunc handler = AishPlugin::GetPlugin(mode);
			if (handler)
				res = handler(cmd);
			else
			{
				cerr << "ERROR: Plugin not supported or not found: " << mode << endl;
				continue;
			}

			// Set the ? env variable just in case.
			setenv("?", to_string(res).c_str(), 1);
			if (res)
			{
				*logs << regex_replace(ps1, (regex)"🙂", "😕");
				continue;
			}
		}
		#if DEBUG
			cerr << "global_thread: " << global_thread << endl;
		#endif
		cout << ps1;
	}

	if (logs && logs != &std::cout)
	{
		delete logs;
		logs = NULL;
	}
    return 0;
}
