/*=====================================
		AI Shell by John Boero
		BrynzAI - 2023
=====================================*/
// In this file is support for:
//  Google Bison code completion (default mode)
//  Bard chat with GCP support.

#include "aish.h"
#include <regex>

using namespace std;	// Sod the style guide.

void vertexappend(Json::Value &thread, const std::string &role, const std::string &message)
{
	Json::Value entry;
	entry["author"] = role;
	entry["content"] = message;
	thread.append(entry);
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
			// C++11 literals are great but tricky when inserting values.
			string initial = R"JSON({
			 "instances": [{"prefix": "Please write me a bash script to do the following: )JSON" 
			 + regex_replace(cmd, regex("\""), "\\\"") + R"JSON("},
			{"context": "You are a chatbot with access to cloud resources using the oidc token TOKEN."}],
			  "parameters": {"temperature": 0.2, "maxOutputTokens": 2048}})JSON";

			initial = regex_replace(initial, regex("TOKEN"), getenvsafe("GOOGLE_APPLICATION_CREDENTIALS"));

			Json::Reader reader;
			if (!reader.parse(initial.c_str(), thread))
			{
				cerr << "ERROR failed to set up initial thread: "
					<< reader.getFormattedErrorMessages() << endl;
				exit(1);
			}

			thread["parameters"]["temperature"] = temperature;
		}

		// Forget JsonValue building. This is just simpler.
		string payload = (string)"{\"instances\": [{\"prefix\": \"" 
		 + "Please write me a bash script with top line shebang to do the following: " 
		 + regex_replace(cmd, regex("\""), "\\\"")
		 + "\"}],  \"parameters\": {\"temperature\": 0.2, \"maxOutputTokens\": 2048}}";

		// As of 16-JUL-2023, this only seems to be available in us-central1
		string url = "https://us-central1-aiplatform.googleapis.com/v1/projects/" 
		 + getenvsafe("CLOUDSDK_CORE_PROJECT")
		 + "/locations/us-central1/publishers/google/models/code-bison:predict";
		
		string bearer = "Authorization: Bearer " + getenvsafe("GOOGLE_APPLICATION_CREDENTIALS");
		struct curl_slist *headers = curl_slist_append(NULL, bearer.c_str());
		headers = curl_slist_append(headers, "Content-Type: application/json; charset=utf-8");

		// This will actually be zero if everything is OK (200).
		httpCode = mycurljson(url, response, "POST", headers, payload);
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
		cerr <<RED<< "ERROR: " << e.what() <<RESET<< endl;
		return 1;
	}
	return 0;
}

// Simple chat with bard.
int chatbard(const string &cmd)
{
	// A static running thread of the chat history.
	static Json::Reader reader;
	static Json::Value thread;
	string sname;

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
			// C++11 literals are great but tricky when inserting values.
			string initial = R"JSON({
			 "instances": [
				{"context": "You are a chatbot with access to cloud resources using the oidc token TOKEN.",
				"messages":[]}], "parameters": {"temperature": 0.1, "maxOutputTokens": 1024}})JSON";

			initial = regex_replace(initial, regex("TOKEN"), getenvsafe("GOOGLE_APPLICATION_CREDENTIALS"));

			Json::Reader reader;
			if (!reader.parse(initial.c_str(), thread))
			{
				cerr << "ERROR failed to set up initial thread: "
					<< reader.getFormattedErrorMessages() << endl;
				exit(1);
			}
			thread["parameters"]["temperature"] = temperature;
		}

		vertexappend(thread["instances"][0]["messages"], "user", cmd);

		// The immediate response
		Json::Value response;
		int httpCode;

		// As of 16-JUL-2023, this only seems to be available in us-central1
		string url = "https://us-central1-aiplatform.googleapis.com/v1/projects/" 
		 + getenvsafe("CLOUDSDK_CORE_PROJECT")
		 + "/locations/us-central1/publishers/google/models/chat-bison:predict";
		string bearer = "Authorization: Bearer " + getenvsafe("GOOGLE_APPLICATION_CREDENTIALS");

		struct curl_slist *headers = curl_slist_append(NULL, bearer.c_str());
		headers = curl_slist_append(headers, "Content-Type: application/json; charset=utf-8");

		// This will actually be zero if everything is OK (200).
		Json::FastWriter fastWriter;
		string request = fastWriter.write(thread);
		httpCode = mycurljson(url, response, "POST", headers, request);
		if (httpCode)
			cerr << YELLOW << "WARN: HTTP " << httpCode << ": " << response << RESET << endl;
		
		Json::Value newRes = response["predictions"][0]["candidates"][0];
		thread["instances"][0]["messages"].append(newRes);
		cout <<CYAN<< newRes["content"].asString() <<RESET<< endl;
	}
	catch (const exception& e)
	{
		cerr << "ERROR: " << e.what() << endl;
		return 1;
	}
	return 0;
}
