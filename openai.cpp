#include "aish.h"
#include <regex>

using namespace std;	// Style guide be damned...

const string gpturl = "https://api.openai.com/v1/chat/completions";

// Basic helper to append a message to a thread.
void gptappend(Json::Value &thread, const std::string &role, const std::string &message)
{
	Json::Value entry;
	entry["role"] = role;
	entry["content"] = message;
	thread.append(entry);
}

// Give GPT the option to control your environment.
int shellgpt(const string &cmd)
{
	Json::Value thread, eq, message, response;
	Json::FastWriter fastWriter;
	static string apiorg = getenvsafe("OPENAI_ORG");
	static string apikey = getenvsafe("OPENAI_API_KEY");
	string sname;

	unsetenv("OPENAI_API_KEY");
	try
	{
		if (thread.empty())
		{
			thread["model"] = "gpt-3.5-turbo";
			thread["temperature"] = temperature;

			gptappend(thread["messages"], "system", "You are a bash script coder providing only bash scripts");
		}

		gptappend(thread["messages"], "user", cmd);

		struct curl_slist *headers = curl_slist_append(NULL, ("Authorization: Bearer " + apikey).c_str());
		headers = curl_slist_append(headers, "Content-Type: application/json");

		string payload = fastWriter.write(thread);
		//cout << payload << endl;

		int httpCode = mycurljson(gpturl, response, "POST", headers, payload);
		if (httpCode != 200)
		{
			cerr << YELLOW << "WARN: HTTP " << httpCode << ": " << response << RESET << endl;
			return httpCode;
		}

		{
			using namespace chrono;
			milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
			sname = getenvsafe("HOME") + "/.aish/gpt_" + to_string(ms.count()) + ".sh";
		}
		string scr = response["choices"][0]["message"]["content"].asString();
		
		// This is ugly but seems to be the best way to extract markdown.
		smatch match;
		regex reg("```(.*)```", regex::extended);
		if (regex_search(scr, match, reg))
			scr = match[1];
		
		ofstream script(sname);
		script << scr;
		script.close();

		// Add -x if we added a runtime -d for debug.
		sname = (string)"/bin/bash " + (debug ? "-x ":"") + sname;
		system(sname.c_str());
	}
	catch (const exception& e)
	{
		cerr << e.what() << endl;
		return 1;
	}
	return 0;
}

// Simple Chat GPT.
int chatgpt(const string &cmd)
{
	Json::Value thread, eq, message, response;
	Json::FastWriter fastWriter;
	static string apiorg = getenvsafe("OPENAI_ORG");
	static string apikey = getenvsafe("OPENAI_API_KEY");

	unsetenv("OPENAI_API_KEY");
	try
	{
		if (thread.empty())
		{
			thread["model"] = "gpt-3.5-turbo";
			thread["temperature"] = temperature;
		}

		gptappend(thread["messages"], "user", cmd);

		struct curl_slist *headers = curl_slist_append(NULL, ("Authorization: Bearer " + apikey).c_str());
		headers = curl_slist_append(headers, "Content-Type: application/json");

		string payload = fastWriter.write(thread);
		// cout << payload << endl;

		int httpCode = mycurljson(gpturl, response, "POST", headers, payload);
		if (httpCode)
			cerr << YELLOW << "WARN: HTTP " << httpCode << ": " << response << RESET << endl;

		Json::Value newmsg = response["choices"][0]["message"];
		cout << newmsg["content"].asString() << endl;
		thread["messages"].append(newmsg);
	}
	catch (const exception& e)
	{
		cerr << e.what() << endl;
		return 1;
	}
	return 0;
}
