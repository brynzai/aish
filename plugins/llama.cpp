/*=====================================
		AI Shell by John Boero
		BrynzAI - 2023
=====================================*/
// In this file is support for Llama2 via llama.cpp server from META:
//  llama (chat)
//	shellllama
// https://github.com/ggerganov/llama.cpp/blob/master/examples/server/README.md

#include <regex>
#include "../aish.h"

using namespace std;	// Sod the style guide.

// Custom callback for streaming Llama curl usage.
size_t streaming_callback(const char* in, size_t size, size_t num, char* out)
{
	string content;
	Json::Value data;
	stringstream stream(regex_replace(in, regex("^data:\\s+|\\s+$"), ""));
	// Despite the JSON mime header Llama sticks a raw data: outside the JSON :/

	try
	{
		stream >> data;
		content = data["content"].asString();

		*((stringstream*) out) << content;
		*logs << content;

		// Flush and read if we hit a newline or punctuation.
		// I tried speaking each token/word at a time,
		// but that had some real "pray for Mojo" vibes.
		if (logs == &cout || regex_match(content, regex("[\\.!?:]")))
			*logs << flush;
		return size * num;
	}
	catch(const std::exception& e)
	{
		cerr << e.what() << endl;
		return 0;
	}
}

// Shell via Llama
int shellllama(const string &cmd)
{
	Json::Value message, response;
	Json::FastWriter fastWriter;
	int httpCode;
	string sname, scr, server;

	if (server == "" && !getenv("LLAMA_SERVER"))
	{
		cerr << "INFO: missing environment variable: LLAMA_SERVER "
			<< "Using http://localhost:8080/completion by default." << endl;
		server = "http://localhost:8080/completion";
	}

	try
	{
		Json::Value payload;
		payload["prompt"] = "You are a helpful writer of shell scripts for accomplishing tasks. " + cmd;

		struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");

		// This will actually be zero if everything is OK (200).
		httpCode = mycurljson(server, response, "POST", headers, fastWriter.write(payload));
		if (httpCode)
		{
			cerr << YELLOW << "WARN: HTTP " << httpCode << ": " << response << RESET << endl;
			return httpCode;
		}

		scr = response["content"].asString();

		{
			using namespace chrono;
			milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
			sname = getenvsafe("HOME") + "/.aish/llama_" + to_string(ms.count()) + ".sh";
		}

		// This is ugly but seems to be the best way to extract markdown.
		smatch match;
		regex reg("```\n?(.*)```", regex::extended);
		if (regex_search(scr, match, reg))
			scr = match[1];
		
		ofstream script(sname);
		script << scr;
		script.close();

		*logs << "#Llama shell produced the following output which will not be run automatically:" << endl;
		*logs << scr << endl;
		// Don't enable execution yet. ShellLlama has a long way to go.
		// Add -x if we did a runtime -d for debug.
		//sname = (string)"/usr/bin/env bash " + (debug ? "-x ":"") + sname;
		//system(sname.c_str());
	}
	catch (const exception& e)
	{
		cerr <<RED<< "ERROR: " << e.what() <<RESET<< endl;
		return 1;
	}
	return 0;
}

// Simple chat with bard.
int llama(const string &cmd)
{
	static Json::Value message, response;
	Json::FastWriter fastWriter;
	int httpCode;
	string sname, scr, server;
	stringstream stream;

	if (server == "")
	{
		if (getenv("LLAMA_SERVER"))
			server = getenvsafe("LLAMA_SERVER");
		else
		{
			server = "http://localhost:8080/completion";
			cerr << "WARNING: missing environment variable: LLAMA_SERVER "
				<< "Using http://localhost:8080/completion by default." << endl;
		}
	}

	try
	{
		if (message.empty())
		{
			message["stop"] = Json::arrayValue;
			message["stop"].append("</s>");
			message["stop"].append("Llama");
			message["stop"].append("User");
			message["stop"].append("global_thread");
			message["stop"].append("system:");

			message["n_predict"] = 590;
			message["repeat_last_n"] = 4096;
			message["top_p"] = 0.5;
			message["stream"] = true;
			message["temperature"] = temperature;
			message["repeat_penalty"] = 2;
		}

		message["prompt"] = Json::arrayValue;
		message["prompt"].append("This is a conversation between User and Llama, a friendly chatbot. Llama is helpful, kind, honest, good at writing, and never fails to answer any requests immediately and with precision.");
		message["prompt"].append(global_thread);
		message["prompt"].append("User: " + cmd);
		message["prompt"].append("Llama: ");

		// This will actually be zero if everything is OK (200).
		struct curl_slist *headers = curl_slist_append(NULL, "Content-Type: application/json");
		httpCode = mycurl(server, stream, "POST", headers, fastWriter.write(message), streaming_callback);
		if (httpCode)
		{
			cerr << YELLOW << "WARN: HTTP " << httpCode << ": " << response << RESET << endl;
			return httpCode;
		}

		// Output moved to streaming_callback above, but flush newline for PS1.
		*logs << endl;
		global_thread.append("user: " + cmd + "\nllama: " + stream.str());
	}
	catch (const exception& e)
	{
		cerr << "ERROR: " << e.what() << endl;
		return 1;
	}
	return 0;
}

// Register these functions as plugins.
AishPlugin llamaPlugin("llama", llama);
AishPlugin shellllamaPlugin("shellllama", shellllama);
