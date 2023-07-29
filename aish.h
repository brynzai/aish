/*=====================================
		AI Shell by John Boero
		BrynzAI - 2023
=====================================

General helpers. Trying to avoid Boost dependency for now.
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
extern bool paragraph, debug;
extern float temperature;

enum aishmode
{
	UNSUPPORTED,
	SHELLBARD,
	CHATBARD,
	SHELLGPT,
	CHATGPT
};

std::string getenvsafe(std::string varname);

int	mycurljson(std::string url, Json::Value &jsonData, std::string request, struct curl_slist *headers, std::string post);

#endif