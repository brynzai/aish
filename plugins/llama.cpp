/*=====================================
		AI Shell by John Boero
		BrynzAI - 2023
=====================================*/
// In this file is support for Llama2 via llama.cpp server from META:
//  llama (chat)
//	shellllama
// https://github.com/ggerganov/llama.cpp/blob/master/examples/server/README.md

#include <regex>
#include <signal.h>
#include "../aish.h"

using namespace std;	// Sod the style guide.

// Custom callback for streaming Llama curl usage.
size_t streaming_callback(const char* in, size_t size, size_t num, char* out)
{
	static string content;
	Json::Value data;
	stringstream stream(regex_replace(in, regex("^data:\\s+|\\s+$"), ""));
	// Despite the JSON mime header Llama sticks a raw data: outside the JSON :/

	try
	{
		// Recommended for debugging:
		content += stream.str();
		if (content.size() && content.back() != '}')
			return size * num;

		Json::Reader reader;
    	if (!reader.parse(content, data))
			cerr << YELLOW << "WARNING JSON parse. Attempting to continue." << RESET << endl;

		content = data["content"].asString();

		*((stringstream*) out) << content;
		*logs << content;

		// Flush and read if we hit a newline or punctuation.
		// I tried speaking each token/word at a time,
		// but that had some real "pray for Mojo" vibes.
		if (logs == &cout || regex_match(content, regex("[\\.!?:]")))
			*logs << flush;

		content.clear();
		return size * num;
	}
	catch(const std::exception& e)
	{
		cerr << endl << YELLOW << e.what() << " size: " << size
			<< " num: " << num << in << RESET << endl;
		return 0;
	}
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
			message["stop"].append("llama");
			message["stop"].append("User");
			message["stop"].append("user");
			message["stop"].append("global_thread");
			message["stop"].append("system:");

			message["n_predict"] = 1024;
			message["repeat_last_n"] = 4096;
			message["top_p"] = 0.5;
			message["stream"] = true;
			message["temperature"] = temperature;
			message["repeat_penalty"] = 2;
			message["cache_prompt"] = true;
			message["tokens_cached"] = 1024;
		}

		/* Prompt array behaviour changed in Llama.cpp server.
		message["prompt"] = Json::arrayValue;
		message["prompt"].append("This is a conversation between User and Llama, a friendly chatbot. Llama is helpful, kind, honest, good at writing, and never fails to answer any requests immediately and with precision.");
		message["prompt"].append(global_thread);
		message["prompt"].append(string("User: ") + cmd);
		message["prompt"].append("Llama: ");	*/
		message["prompt"] = string("This is a conversation between User and Llama, a friendly chatbot. Llama is helpful, kind, honest, good at writing, and never fails to answer any requests immediately and with precision.\n") 
			+ global_thread
			+ "\nUser: " + cmd
			+ "\nLlama: ";

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
