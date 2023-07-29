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
size_t callback(const char* in, size_t size, size_t num, char* out)
{
	string data(in, (size_t) size * num);
	*((stringstream*) out) << data;

	#if DEBUG
	*logs << GREEN << data << RESET << endl;
	#endif
	return size * num;
}

int	mycurl(string url, stringstream &httpData, string request, struct curl_slist *headers, const string post = "")
{
	int res = 0, httpCode = 0;
	static CURL* curl = curl_easy_init();

	#if DEBUG
	*logs << CYAN << url << " POST: " << post << RESET << endl;
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

		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 20L);
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);
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

int	mycurljson(string url, Json::Value &jsonData, string request = "GET", struct curl_slist *headers = NULL, string post = "")
{
	stringstream stream;
	Json::CharReaderBuilder jsonReader;
	int	res = 0;

	if ((res = mycurl(url, stream, request, headers, post)))
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
