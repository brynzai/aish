/*=====================================
		AI Shell by John Boero
		BrynzAI - 2023
=====================================*/
// In this file is support for OpenAI:
//  chatgpt
//  shellgpt
// https://platform.openai.com/docs/api-reference

#include <regex>
#include "../aish.h"

using namespace std;	// Style guide be damned...

// Basic helper to append a message to a thread.
void gptappend(Json::Value &thread, const string &role, const string &message)
{
	Json::Value entry;
	entry["role"] = role;
	entry["content"] = message;
	thread["messages"].append(entry);
}

// Give GPT the option to control your environment.
int shellgpt(const string &cmd)
{
	Json::Value thread, eq, message, response;
	Json::FastWriter fastWriter;
	static string apiorg = getenvsafe("OPENAI_ORG");
    static string apikey = getenvsafe("OPENAI_API_KEY");
	static string gpturl = getenvsafe("OPENAI_ENDPOINT", "https://api.openai.com/v1/chat/completions");
	string sname;

    unsetenv("OPENAI_API_KEY");
	try
	{
		if (thread.empty())
		{
			Json::Value history;
			history["role"] = "system";
			
			thread["model"] = getenvsafe("OPENAI_MODEL", "gpt-3.5-turbo");;
			thread["temperature"] = temperature;
			thread["messages"] = Json::arrayValue;
			thread["messages"].append(history);
		}

		thread["messages"][0]["content"] = "You are a bash script coder providing only bash scripts.\n" + global_thread;
		gptappend(thread, "user", cmd);

		struct curl_slist *headers = curl_slist_append(NULL, ("Authorization: Bearer " + apikey).c_str());
		headers = curl_slist_append(headers, "Content-Type: application/json");

		string payload = fastWriter.write(thread);
		//cout << payload << endl;

		int httpCode = mycurljson(gpturl, response, "POST", headers, payload);
		if (httpCode)
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
		global_thread.append("user: " + cmd + "\nllama: " + scr + '\n');
		// This is ugly but seems to be the best way to extract markdown.
		smatch match;
		regex reg("```\n?(.*)```", regex::extended);
		if (regex_search(scr, match, reg))
			scr = match[1];
		
		ofstream script(sname);
		script << scr;
		script.close();

		// Add -x if we added a runtime -d for debug.
		sname = (string)"/usr/bin/env bash " + (debug ? "-x ":"") + sname;
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
	static string gpturl = getenvsafe("OPENAI_ENDPOINT", "https://api.openai.com/v1/chat/completions");

	// Default endpoint.
	if (gpturl.empty())
		gpturl = "https://api.openai.com/v1/chat/completions";

    unsetenv("OPENAI_API_KEY");
	try
	{
		if (thread.empty())
		{
			Json::Value history;
			history["role"] = "system";

			thread["model"] = getenvsafe("OPENAI_MODEL", "https://api.openai.com/v1/chat/completions");
			thread["temperature"] = temperature;
			thread["messages"] = Json::arrayValue;
			thread["messages"].append(history);
		}
		thread["messages"][0]["content"] = 
			"You are a friendly chat bot in a chat with a human and possibly other chatbots.\n" 
			+ global_thread;
		gptappend(thread, "user", cmd);

		struct curl_slist *headers = curl_slist_append(NULL, (string("Authorization: Bearer ") + apikey).c_str());
		headers = curl_slist_append(headers, "Content-Type: application/json");

		string payload = fastWriter.write(thread);

		int httpCode = mycurljson(gpturl, response, "POST", headers, payload);
		if (httpCode)
			cerr << YELLOW << "WARN: HTTP " << httpCode << ": " << response << RESET << endl;

		Json::Value newmsg = response["choices"][0]["message"];
		gptappend(thread, newmsg["role"].asString(), newmsg["content"].asString());
		*logs << newmsg["content"].asString() << endl;
		global_thread.append("user: " + cmd + "\nchatgpt: " + newmsg["content"].asString() + '\n');
	}
	catch (const exception& e)
	{
		cerr << e.what() << endl;
		return 1;
	}
	return 0;
}

// Register these functions as plugins.
AishPlugin chatGPTPlugin("chatgpt", chatgpt);
AishPlugin shellGPTPlugin("shellgpt", shellgpt);
