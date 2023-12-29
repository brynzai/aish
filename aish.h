/*=====================================
		AI Shell by John Boero
		BrynzAI - 2023
=====================================
This file contains common declarations for helper functions and the AishPlugins class.

v0.2.0: Added AishPlugins class.
Plugins currently are a simple singleton which makes it easy to add a plugin
without editing any other files. This code is super ugly for now but works great.

*/

#ifndef _CURLTRIM_
#define _CURLTRIM_

#include <iostream>
#include <fstream>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <string>
#include <unistd.h>
#include <chrono>

#include <curl/curl.h>
#include <json/json.h>

// Term colors for stdout
const char RESET[]	= "\e[0m";
const char RED[]	= "\e[1;31m";
const char YELLOW[]	= "\e[0;33m";
const char CYAN[]	= "\e[1;36m";
const char PURPLE[]	= "\e[0;35m";
const char GREEN[]	= "\e[0;32m";
const char BLUE[]	= "\e[0;34m";
const char GREY[]   = "\033[0;37m";

// Yes this is ugly I know..
extern std::ostream *logs;
extern std::string global_thread;
extern bool paragraph, debug;
extern float temperature;

// Map of plugins to functions (currently not using OOP)

std::string getenvsafe(const std::string &varname, const std::string &def = "");
size_t standard_callback(const char* in, size_t size, size_t num, char* out);

// Added optional function pointer to handler for asynch streaming requests.
int	mycurl(std::string url, std::stringstream &httpData, std::string request, struct curl_slist *headers, const std::string post = "",
	size_t (*handler)(const char* in, size_t size, size_t num, char* out) = standard_callback);
int	mycurljson(std::string url, Json::Value &jsonData, std::string request, struct curl_slist *headers, std::string post = "",
	size_t (*handler)(const char* in, size_t size, size_t num, char* out) = standard_callback);

// This ugly singleton keeps a static list of all plugins.
// A map of strings (name) to function pointer [int func(string cmd)] is ugly
// but there is no need to write a class or use inheritance to simply handle an AI conversation.
class AishPlugin
{
	public:
	// Plugin functions take a string as input and return an error code (0 == SUCCESS)
	typedef int (*plugFunc)(const std::string &cmd);
	typedef std::map<std::string, plugFunc> plugins;

	// Any constructor will automatically register your function with the plugins list.
	AishPlugin(const std::string &name, plugFunc plugin);
	static plugFunc GetPlugin(const std::string &mode);
	static void PrintAll();
	
	private:
	static plugins allPlugins;
};

#endif