/*=====================================
		AI Shell by John Boero
		BrynzAI - 2023
=====================================
This file contains definitions for helper functions and the AishPlugins class.

v0.2.0: Added AishPlugins class.
Plugins currently are a simple singleton which makes it easy to add a plugin
without editing any other files. This code is super ugly for now but works great.

*/

#include "aish.h"
#include <regex>

using namespace std;	// Style guide be damned...

// Put a basic wrapper on this.
string getenvsafe(string varname)
{
	if (getenv(varname.c_str()))
		return string(getenv(varname.c_str()));
	else
		return "";
}

// Callbacks for simplified curl usage.
size_t standard_callback(const char* in, size_t size, size_t num, char* out)
{
	string data(in, (size_t) size * num);
	*((stringstream*) out) << data;

	#if DEBUG
	cerr << GREEN << data << RESET << endl;
	#endif
	return size * num;
}

int	mycurl(string url, stringstream &httpData, string request, struct curl_slist *headers, const string post,
	size_t (*handler)(const char* in, size_t size, size_t num, char* out))
{
	int res = 0, httpCode = 0;
	static CURL* curl = curl_easy_init();

	#if DEBUG
	cerr << CYAN << url << " POST: " << post << RESET << endl;
	#endif

	{
		if (!curl)
			return -1;

		if ((res = curl_easy_setopt(curl, CURLOPT_URL, url.c_str())))
			return res;

		if ((res = curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request.c_str())))
			return res;

		if (request == "POST" && post != "")
		{
			if ((res = curl_easy_setopt(curl, CURLOPT_POST, 1)))
				return res;

			if ((res = curl_easy_setopt(curl, CURLOPT_POSTFIELDS, post.c_str())))
				return res;
	 	}
		
		if ((res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers)))
			return res;

		// Can't set timeout via ~/.curlrc: connect-timeout = 25
		// Careful invalid integer string behavior is undefined.
		if (getenv("CURLOPT_TIMEOUT"))
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, atoi(getenv("CURLOPT_TIMEOUT")));
		else
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, 120L);

		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, handler);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &httpData);

		CURLcode code = curl_easy_perform(curl);
		if (code)
			cerr << "WARNING libcurl curl_easy_perform code: " << code << endl;

		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
		curl_slist_free_all(headers);
		headers = NULL;
	}

	if (httpCode < 200 || httpCode >= 300)
	{
		cerr << RED 
			<< "Couldn't " << request << " " << post << " -> " 
			<< url << " HTTP " << httpCode << endl << httpData.str()
			<< RESET << endl;
		return httpCode;
	}

	return res;
}

int	mycurljson(string url, Json::Value &jsonData, string request = "GET", struct curl_slist *headers = NULL, string post,
	size_t (*handler)(const char* in, size_t size, size_t num, char* out))
{
	stringstream stream;
	int	res = 0;

	if ((res = mycurl(url, stream, request, headers, post, handler)))
		return res;

	try
	{
		stream >> jsonData;
	}
	catch (exception  const &e)
	{
		cerr << YELLOW << "WARNING JSON problem: " 
			<< e.what() << RESET << endl;
	}

	return 0;
}

// Plugins singleton class definition:
// Static initialization ugliness...
AishPlugin::plugins AishPlugin::allPlugins = AishPlugin::plugins();

// Simply add to our static singleton list of plugins.
AishPlugin::AishPlugin(const std::string &name, plugFunc plugin)
{
	allPlugins[name] = plugin;
}

// Get the plugin function pointer from singleton map via mode name.
// If the plugin/mode is not supported then NULL is returned.
AishPlugin::plugFunc AishPlugin::GetPlugin(const std::string &mode)
{
	try
	{
		return allPlugins[mode];
	}
	catch(const std::exception& e)
	{
		return NULL;
	}
}

// Print all supported plugins in help menu.
void AishPlugin::PrintAll()
{
	for(std::map<std::string, plugFunc>::iterator iter = allPlugins.begin(); iter != allPlugins.end(); ++iter)
		std::cout << "	" << iter->first << endl;
	
	std::cout << endl;
}