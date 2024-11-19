
#include "APIClient.h"

#include "easywsclient.hpp"


#include <queue>
#include <future>

#include "nlohmannjson.hpp"
using namespace nlohmann;

using easywsclient::WebSocket;

string APIClient::domain;
thread eventThread;
std::queue<std::function<void()>> taskQueue;
std::mutex queueMutex;

std::string UrlEncode(const std::string& s)
{
	const std::string unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";

	std::string escaped = "";
	for (size_t i = 0; i < s.length(); i++)
	{
		if (unreserved.find_first_of(s[i]) != std::string::npos)
		{
			escaped.push_back(s[i]);
		}
		else
		{
			escaped.append("%");
			char buf[3];
			sprintf(buf, "%.2X", (unsigned char)s[i]);
			escaped.append(buf);
		}
	}
	return escaped;
}

void pushTask(std::function<void()> task) {
	try {
		while (!queueMutex.try_lock());
		taskQueue.push(task);
		queueMutex.unlock();
	}
	catch (const std::system_error& e) {
		std::cerr << "Failed to lock mutex: " << e.what() << std::endl;
		return;
	}
}

void APIClient::eventLoop() {
	std::cout << "eventLoop: " << std::this_thread::get_id() << std::endl;
	while (true)
	{
		while (!taskQueue.empty()) {
			auto task = taskQueue.front();
			taskQueue.pop();
			while (!queueMutex.try_lock());
			std::thread(task).detach();
			queueMutex.unlock();
		}
		Sleep(10);
	}
}

void APIClient::test(string client_id, string prompt_id, std::function<void(string)> callback) {
	pushTask([client_id, prompt_id, callback]() {
		stringstream url;
		url << "ws://" << "127.0.0.1:8188" << "/ws?clientId=" << client_id;

		std::cout << url.str() << std::endl;

		std::unique_ptr<WebSocket> ws(WebSocket::from_url(url.str()));

		while (ws->getReadyState() != WebSocket::CLOSED) {
			WebSocket::pointer wsp = &*ws;
			ws->poll();
			ws->dispatch([wsp](const std::string& message) {

				auto result_json = json::parse(message);
				string type = result_json["type"].get<string>();

				if (type == "progress") {
					// execution_success
				}
				else if (type == "execution_cached") {

				}
				else if (type == "executing") {
					auto data = result_json["data"];

					if (data["node"].is_null()) {
						std::cout << "closed." << std::endl;
						wsp->close();
					}
					else {
						string node = data["node"].get<string>();
						std::cout << "node: " << node << std::endl;
					}
				}
			});
		}

		ws.release();
		// with urllib.request.urlopen("http://{}/history/{}".format(server_address, prompt_id)) as response:
		string res = APIClient::Get("history/" + prompt_id);

		json result;
		result = json::parse(res);

		for (auto& e : result) {
			//std::cout <<  << std::endl;
			for (auto& [key, value] : e["outputs"].items()) {

				if (value.contains("images")) {
					for (auto& image : value["images"]) {
						if (image["type"] == "output") {
							stringstream params;
							params << "filename=" << image["filename"].template get<std::string>() << "&subfolder=" << image["subfolder"].template get<std::string>() << "&type=" << image["type"].template get<std::string>();

							std::cout << params.str() << std::endl;
							std::cout << "view?" + UrlEncode(params.str()) << std::endl;

							string result = APIClient::Get("view?filename=ComfyUI_00044_.png&subfolder=&type=output");

							std::cout << result << std::endl;
							callback(result);
						}
					}

				}
			}
		}
	});
}

void APIClient::Init(string domain) { 
	APIClient::domain = domain; 
	// fix me!
	eventThread = thread(&APIClient::eventLoop);
	eventThread.detach();
};

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

	if (res == nullptr)
		return nullptr;
	if (res->status != StatusCode::OK_200)
		return nullptr;

	return res->body;
}

string APIClient::Get(const string& url, Headers headers)
{
	Client client(APIClient::domain);
	auto res = client.Get("/" + url, headers);

	if (res == nullptr)
		return nullptr;
	if (res->status != StatusCode::OK_200)
		return nullptr;

	return res->body;
}

string APIClient::Post(const string& url, const string& data)
{
	Client client(APIClient::domain);

	client.set_read_timeout(360, 0); // 5 seconds
	client.set_write_timeout(360, 0); // 5 seconds

	std::cout << url << std::endl;
	auto res = client.Post("/" + url, data, "application/json");

	if (res == nullptr) 
		return nullptr;
	if (res->status != StatusCode::OK_200) 
		return nullptr;

	return res->body;
}

string APIClient::Post(const string& url, Headers headers, Params data)
{
	Client client(APIClient::domain);
	auto res = client.Post("/" + url, headers, data);

	if (res == nullptr)
		return nullptr;
	if (res->status != StatusCode::OK_200)
		return nullptr;
	
	return res->body;
}

string APIClient::Post(const string& url, const httplib::MultipartFormDataItems& params) {
	Client client(APIClient::domain);

	client.set_read_timeout(360, 0); // 5 seconds
	client.set_write_timeout(360, 0); // 5 seconds

	auto res = client.Post("/" + url, params);

	if (res == nullptr)
		return nullptr;
	if (res->status != StatusCode::OK_200)
		return nullptr;

	return res->body;
}

void APIClient::PostAsync(const string& url, const string& data, std::function<void(string)> callback) {
	pushTask([url, data, callback]() {
		string result = APIClient::Post(url, data);
		callback(result);
		});
}

void APIClient::PostAsync(const string& url, const httplib::MultipartFormDataItems& params, std::function<void(string)> callback) {
	pushTask([url, params, callback]() {
		string result = APIClient::Post(url, params);
		callback(result);
		});
}
