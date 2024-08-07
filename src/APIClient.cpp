
#include "APIClient.h"

string APIClient::domain = "http://127.0.0.1";

bool APIClient::isValid() {

	try {
		Client client(APIClient::domain);
		auto res = client.Get("/");

		if (res == nullptr)
			return false;

		if (res->status == StatusCode::OK_200)
			return true;
		else
			return false;
	}
	catch (exception e) {
		return false;
	}

	return false;
}

string APIClient::Get(const string& url)
{
	Client client(APIClient::domain);
	auto res = client.Get("/" + url);

	if (res == nullptr ||
		res->status != StatusCode::OK_200)
		return nullptr;

	std::cout << "ok" << std::endl;

	return res->body;
}

string APIClient::Get(const string& url, Headers headers)
{
	Client client(APIClient::domain);
	auto res = client.Get("/" + url, headers);

	if (res == nullptr ||
		res->status != StatusCode::OK_200)
		return nullptr;

	std::cout << "ok" << std::endl;

	return res->body;
}

string APIClient::Post(const string& url, const string& data)
{
	Client client(APIClient::domain);

	client.set_read_timeout(360, 0); // 5 seconds
	client.set_write_timeout(360, 0); // 5 seconds

	cout << "1" << endl;

	auto res = client.Post("/" + url, data, "text/plain");

	cout << "2" << endl;

	if (res == nullptr ||
		res->status != StatusCode::OK_200)
		return nullptr;

	return res->body;
}

string APIClient::Post(const string& url, Headers headers, Params data)
{
	Client client(APIClient::domain);
	auto res = client.Post("/" + url, headers, data);

	if (res == nullptr ||
		res->status != StatusCode::OK_200)
		return nullptr;

	return res->body;
}

string APIClient::UrlEncode(const string& url)
{
	const string unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";

	string escaped = "";
	for (size_t i = 0; i < url.length(); i++)
	{
		if (unreserved.find_first_of(url[i]) != std::string::npos)
		{
			escaped.push_back(url[i]);
		}
		else
		{
			escaped.append("%");
			char buf[3];

			sprintf_s(buf, "%.2X", (unsigned char)url[i]);
			escaped.append(buf);
		}
	}
	return escaped;
}