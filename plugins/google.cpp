/*=====================================
		AI Shell by John Boero
		BrynzAI - 2023
=====================================*/
// In this file is support for:
//  Google Bison code completion (default mode)
//  Bard chat with GCP support.
// https://cloud.google.com/vertex-ai/docs/reference/rest

#include <regex>
#include "../aish.h"

using namespace std;	// Sod the style guide.

void vertexappend(Json::Value &thread, const string &role, const string &message)
{
	Json::Value entry;
	entry["author"] = role;
	entry["content"] = message;
	thread.append(entry);
	global_thread.append(role + ": " + message + '\n');
}

// Vertex's Bison mode is for code generation.
int shellbard(const string &cmd)
{
	static Json::Value thread;
	Json::Value message, response;
	Json::FastWriter fastWriter;
	int httpCode;
	string sname, scr;

	if (!getenv("CLOUDSDK_CORE_PROJECT") || !getenv("GOOGLE_APPLICATION_CREDENTIALS"))
	{
		cerr << RED << "ERROR: missing environment variable. Please define: "
			<< "CLOUDSDK_CORE_PROJECT and your auth token GOOGLE_APPLICATION_CREDENTIALS and try again." 
			<< RESET << endl;
		return 1;
	}

	try
	{
		if (thread.empty())
		{
			Json::Value prefix, params;
			prefix["context"] = "You are a codebot for writing primarily bash. Optionally use my cloud OIDC token: " 
				+ getenvsafe("GOOGLE_APPLICATION_CREDENTIALS");
			
			params["temperature"] = temperature;
			params["maxOutputTokens"] = 2048;

			thread["instances"] = Json::Value(Json::arrayValue);
			thread["instances"].append(prefix);
			thread["parameters"] = params;
		}

		thread["instances"][0]["prefix"] = global_thread + "Please write me a bash script to do the following: " + cmd;

		// As of 16-JUL-2023, this only seems to be available in us-central1
		string url = "https://us-central1-aiplatform.googleapis.com/v1/projects/" 
		 + getenvsafe("CLOUDSDK_CORE_PROJECT")
		 + "/locations/us-central1/publishers/google/models/code-bison:predict";
		
		string bearer = "Authorization: Bearer " + getenvsafe("GOOGLE_APPLICATION_CREDENTIALS");
		struct curl_slist *headers = curl_slist_append(NULL, bearer.c_str());
		headers = curl_slist_append(headers, "Content-Type: application/json; charset=utf-8");

		// This will actually be zero if everything is OK (200).
		httpCode = mycurljson(url, response, "POST", headers, fastWriter.write(thread));
		if (httpCode)
		{
			cerr << YELLOW << "WARN: HTTP " << httpCode << ": " << response << RESET << endl;
			return httpCode;
		}

		scr = response["predictions"][0]["content"].asString();
		{
			using namespace chrono;
			milliseconds ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch());
			sname = getenvsafe("HOME") + "/.aish/bard_" + to_string(ms.count()) + ".sh";
		}

		global_thread.append("user: " + cmd + '\n' + "bard: " + scr + '\n');
		
		// This is ugly but seems to be the best way to extract markdown.
		smatch match;
		regex reg("```\n?(.*)```", regex::extended);
		if (regex_search(scr, match, reg))
			scr = match[1];
		
		ofstream script(sname);
		script << scr;
		script.close();

		// Add -x if we did a runtime -d for debug.
		sname = (string)"/usr/bin/env bash " + (debug ? "-x ":"") + sname;
		system(sname.c_str());
	}
	catch (const exception& e)
	{
		cerr << RED << "ERROR: " << e.what() << RESET << endl;
		return 1;
	}
	return 0;
}

// Simple chat with bard.
int chatbard(const string &cmd)
{
	// A static running thread of the chat history.
	static Json::Reader reader;
	static Json::Value thread, response;
	Json::FastWriter fastWriter;
	string sname;
	int httpCode;

	if (!getenv("CLOUDSDK_CORE_PROJECT") || !getenv("GOOGLE_APPLICATION_CREDENTIALS"))
	{
		cerr << YELLOW << "WARNING: missing environment variable. Please define: "
			<< "CLOUDSDK_CORE_PROJECT, GOOGLE_APPLICATION_CREDENTIALS, and "
			<< "try again." << RESET << endl;
			return 1;
	}

	try
	{
		// Initialize static thread if it's empty.
		if (thread.empty())
		{
			Json::Value instance, params;
			instance["context"] = "You are a chatbot with access to cloud resources in my project: "
				+ getenvsafe("CLOUDSDK_CORE_PROJECT")
				+ " using the OIDC token: " 
				+ getenvsafe("GOOGLE_APPLICATION_CREDENTIALS");
			instance["messages"] = Json::Value(Json::arrayValue);

			params["temperature"] = temperature;
			params["maxOutputTokens"] = 2048;

			thread["instances"] = Json::Value(Json::arrayValue);
			thread["instances"].append(instance);
			thread["parameters"] = params;
		}

		// Attach history outside of this to avoid double history
		Json::Value global_history;
		global_history["author"] = "user";
		global_history["content"] = "Hello Bard. This is my conversation history.";
		thread["instances"][0]["messages"].append(global_history);
		global_history["author"] = "system";
		global_history["content"] = global_thread;
		thread["instances"][0]["messages"].append(global_history);

		vertexappend(thread["instances"][0]["messages"], "user", cmd);

		// As of 16-JUL-2023, this only seems to be available in us-central1
		string url = "https://us-central1-aiplatform.googleapis.com/v1/projects/" 
			+ getenvsafe("CLOUDSDK_CORE_PROJECT")
			+ "/locations/us-central1/publishers/google/models/"
			+ getenvsafe("AISH_BARD_MODEL", "textembedding-gecko")
			+ ":predict";
		string bearer = "Authorization: Bearer " + getenvsafe("GOOGLE_APPLICATION_CREDENTIALS");

		struct curl_slist *headers = curl_slist_append(NULL, bearer.c_str());
		headers = curl_slist_append(headers, "Content-Type: application/json; charset=utf-8");

		// This will actually be zero if everything is OK (200).
		string request = fastWriter.write(thread);
		httpCode = mycurljson(url, response, "POST", headers, request);
		if (httpCode)
			cerr << YELLOW << "WARN: HTTP " << httpCode << ": " << response << RESET << endl;
		
		Json::Value newRes = response["predictions"][0]["candidates"][0];
		vertexappend(thread["instances"][0]["messages"], newRes["author"].asString(), newRes["content"].asString());

		*logs << newRes["content"].asString() << endl;
	}
	catch (const exception& e)
	{
		cerr << "ERROR: " << e.what() << endl;
		return 1;
	}
	return 0;
}

// A brilliant TODO stub worthy of GenAI
int chatgemini(const string &cmd)
{
	// Just abstract the model for reusability.
	setenv("AISH_BARD_MODEL", "gemini-pro", 1);
	return chatbard(cmd);
}

// Register these functions as plugins.
AishPlugin geminiPlugin("chatgemini", chatgemini);
AishPlugin bardPlugin("chatbard", chatbard);
AishPlugin shellbardPlugin("shellbard", shellbard);
