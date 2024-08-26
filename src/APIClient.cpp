
#include "APIClient.h"
#include <queue>
#include <future>

string APIClient::domain = "http://127.0.0.1";
thread eventThread;
std::queue<std::function<void()>> taskQueue;
std::mutex queueMutex;

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

void APIClient::PostAsync(const string& url, const string& data, std::function<void(string)> callback) {
	std::cout << "PostAsync: " << std::this_thread::get_id() << std::endl;
	pushTask([url, data, callback]() {
		std::cout << "callback call: " << std::this_thread::get_id() << std::endl;
		string result = APIClient::Post(url, data);
		callback(result);
		std::cout << "callback call end: " << std::this_thread::get_id() << std::endl;
	});
}

void APIClient::Init(string domain) { 
	APIClient::domain = domain; 

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