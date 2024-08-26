#pragma once

#include <string>

//#define CPPHTTPLIB_OPENSSL_SUPPORT FALSE
#include "httplib.h"

using namespace httplib;
using namespace std;

class APIClient {
public:
	static void Init(string domain);
	static bool isValid();
	static string Get(const string& url);
	static string Get(const string& url, Headers headers);
	static string Post(const string& url, const string& data);
	static void PostAsync(const string& url, const string& data, std::function<void(string)> callback);
	static string Post(const string& url, Headers headers, Params data);
	static string UrlEncode(const string& text);

	static string domain; 

private:
	static void eventLoop();
};
