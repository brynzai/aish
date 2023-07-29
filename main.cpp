/*=====================================
		AI Shell by John Boero
		BrynzAI - 2023
=====================================*/
//#define CURL_STATICLIB

#include "aish.h"

#include <sstream>
#include <fstream>
#include <regex>
#include <sys/stat.h>
#include <signal.h>
#include "aish.h"
//#include <ncurses.h> // FUTURE

using namespace std; // Sod the style guide.

// Lazy headers.. also bad style
int chatgpt(const string &cmd);
int shellgpt(const string &cmd);
int chatbard(const string &cmd);
int shellbard(const string &cmd);

// Globals... so sue me?
ostream *logs = &std::cout;

// Paragraph mode parsing?
// This implies IFS is a blank line.
bool paragraph, debug;
float temperature = 0.2f;
string ps1;
aishmode mode = SHELLBARD;

// Ignore signals for now.
void signals(int sig_num) {}

int main (int argc, char **argv)
{
	int res;
	string cmd, smode;
	string stemp = getenvsafe("AISH_TEMP");
	stringstream buffer;
	istream *input = &cin;
	ifstream scriptFile;
	ofstream history;

	// Ignore ^C SIGINT for now.
	signal(SIGINT, signals);

	if (stemp != "")
		temperature = stof(stemp);

    while(true)
    {
        // note the colon (:) to indicate that 'b' has a parameter and is not a switch
        switch(getopt(argc, argv, "m:xhpd?"))
        {
            case 'x':
			debug = true;
            #if DEBUG
			cerr << GREY << "Debug/trace output on." << RESET << endl;
            #endif
			continue;

            case 'm':
            smode = optarg;
			if (smode == "shellbard")
				mode = SHELLBARD;
			else if (smode == "chatbard")
				mode = CHATBARD;
			else if (smode == "shellgpt")
				mode = SHELLGPT;
			else if (smode == "chatgpt")
				mode = CHATGPT;
			else
				mode = UNSUPPORTED;
            continue;

			case 'p':
			paragraph = true;
			continue;

            case '?':
            case 'h':
            default :
            cout << R"USAGE(Usage: aish [-xp] [-m MODE]
Where MODE is one of: chatbard|chatgpt|shellbard|shellgpt and defaults to shellbard.
Environment variabes required for Bard: 
] Ask the shell to do anything for you in plain language. Use a blank line to submit.
			)USAGE" << endl;
            return 0;
			break;

            case -1:
            break;
        }

        break;
    }

	ps1 = "\e[0;33m"+smode+"🙂\033[0m> ";
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
			// Discard shebang line
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
	
	cout << ps1;

	while (getline(*input, cmd))
	{
		// Trim white space and comment. Suck it, Python.
		cmd = regex_replace(cmd, (regex)"^\\s+|\\s+$|\\s*#.*$", "");

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

		if (cmd != "")
		{
			// Specialty commands, exit, cd, etc.
			if (cmd == "quit" || cmd == "exit")
				break;
			
			if (cmd == "cd" || regex_match(cmd, (regex)"^cd\\s*.*"))
			{
				cmd = regex_replace(cmd, (regex)"^cd\\s*", "");
				if (chdir(cmd.c_str()))
					cerr << RED << "Failed to change directory: ENO " << errno << RESET << endl;
				cout << ps1;
				continue;
			}

			// Simple replace params with the rest of argc.
			// Note extra params beyond argc and variables will currently not be replaced!
			for (int param = 1; ++optind < argc; ++param)
			{
				string paramstr = "\\$" + to_string(param);
				cmd = regex_replace(cmd, (regex)paramstr, argv[optind]);
			}

			if (debug) cerr << GREY << smode << ": " << cmd << RESET << endl;

			// Make sure to warn people about unexpected AI shell behaviour.
			// Future maybe we can offer a bypass for this.
			if (regex_match(smode, (regex)".*shell.*"))
				cerr << YELLOW << "WARNING use AI for shell carefully and at you own risk." << RESET << endl;

			// Ugly but simple.
			switch (mode)
			{
				case SHELLGPT: res = shellgpt(cmd); break;
				case CHATGPT: res = chatgpt(cmd); break;
				case SHELLBARD: res = shellbard(cmd); break;
				case CHATBARD: res = chatbard(cmd); break;
				default:
					cerr << "Mode " << smode << " is not yet supported by aish." << endl;
					return 1;
			}

			// Set the ? env variable just in case.
			setenv("?", to_string(res).c_str(), 1);
			if (res)
			{
				cout << regex_replace(ps1, (regex)"🙂", "😕");
				continue;
			}
		}
		cout << ps1;
	}

    return 0;
}
