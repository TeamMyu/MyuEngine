
#include "SDAPIs.h"

#include "nlohmannjson.hpp"
#include "easywsclient.hpp"

#include <string>
#include <queue>
#include <future>
#include <cstdlib>

using namespace nlohmann;

using easywsclient::WebSocket;

// property
bool StableDiffusionAPI::_isWorking;
string StableDiffusionAPI::_result;
float StableDiffusionAPI::_progress;

string StableDiffusionAPI::domain;
thread StableDiffusionAPI::eventThread;
std::queue<std::function<void()>> StableDiffusionAPI::taskQueue;
std::mutex StableDiffusionAPI::queueMutex;

StableDiffusionAPI::StableDiffusionAPI()
{
	StableDiffusionAPI::Init("127.0.0.1:8188");
}

bool StableDiffusionAPI::isWorking() {
	return StableDiffusionAPI::_isWorking;
}

string StableDiffusionAPI::Result() {
	if (StableDiffusionAPI::_result.size() != 0)
		return StableDiffusionAPI::_result;
	return nullptr;
}

bool StableDiffusionAPI::Progress() {
	return StableDiffusionAPI::_progress;
}

void StableDiffusionAPI::makeCharacter(GenCharacterParams params) {
	if (StableDiffusionAPI::_isWorking)
		return;

	StableDiffusionAPI::_isWorking = true;


	/*multipart_data = MultipartEncoder(
		fields = {
		  'image': (name, file, 'image/png'),
		  'type' : image_type,
		  'overwrite' : str(overwrite).lower()
		}
	)

		data = multipart_data
		headers = { 'Content-Type': multipart_data.content_type }
		request = urllib.request.Request("http://{}/upload/image".format(server_address), data = data, headers = headers)
		with urllib.request.u*/

	unordered_map<string, string> id2class;
	string k_sampler;

	std::ifstream raw_workflow(params.workflowPath);
	json workflow;
	raw_workflow >> workflow;

	for (auto& el : workflow.items()) {
	    std::cout << el.key() << " : " << el.value()["class_type"] << std::endl;
	    id2class.insert({ el.value()["class_type"],  el.key() });
	}

	auto inputs = workflow[id2class["KSampler"]]["inputs"];
	auto seed = inputs["seed"];
	auto posText = inputs["positive"];
	std::cout << seed << std::endl;
	std::cout << posText << std::endl;
	//k_sampler = j3[id2class["KSampler"]];

	posText = "1 girl, solo, no background";

	workflow[id2class["KSampler"]]["inputs"]["seed"] = rand();
	//workflow[id2class["KSampler"]]["inputs"]["positive"] = posText;
	workflow["6"]["inputs"]["text"] = posText;

	UUID uuid;
	UuidCreate(&uuid);

	char* client_id = nullptr;
	UuidToStringA(&uuid, (RPC_CSTR*)&client_id);

	json prompt;
	prompt["prompt"] = workflow;
	prompt["client_id"] = client_id;

	json result;
	result = json::parse(StableDiffusionAPI::Post("prompt", prompt.dump()));
	string prompt_id = result["prompt_id"];
	std::cout << prompt_id << std::endl;

	std::function<void(std::string)> callback = [this](std::string file) {
		std::cout << "saved!" << std::endl;
		std::ofstream writeFile;
		writeFile.open("image.png", std::ios::binary);
		writeFile.write(file.c_str(), file.size());
		writeFile.close();
		};

	PushTask([client_id, prompt_id, callback]() {
		stringstream url;
		url << "ws://" << StableDiffusionAPI::domain << "/ws?clientId=" << client_id;

		std::unique_ptr<WebSocket> ws(WebSocket::from_url(url.str()));
		auto finished = unordered_map<int, bool>();

		while (ws->getReadyState() != WebSocket::CLOSED) {
			WebSocket::pointer wsp = &*ws;
			ws->poll();
			ws->dispatch([wsp, prompt_id](const std::string& message) {

				try {
					auto result_json = json::parse(message);
					string type = result_json["type"].get<string>();

					if (type == "progress") {
						auto data = result_json["data"];
						float step = data["value"].template get<float>();
						float maxStep = data["max"].template get<float>();
						std::cout << "step: " << step << " of " << maxStep << std::endl;
						StableDiffusionAPI::_progress = step / maxStep * 100.f - 1.f;
					}
					else if (type == "executing") {
						auto data = result_json["data"];
						std::cout << data << std::endl;
						if (data["node"].is_null()) {
							if (data["prompt_id"].get<string>() == prompt_id) {
								StableDiffusionAPI::_progress = 100.f;

								std::cout << "closed." << std::endl;
								wsp->close();
							}
						}
						else {
							string node = data["node"].get<string>();
							std::cout << "node: " << node << std::endl;
						}
					}
				}
				catch(int ex) {

				}

				});
		}

		ws.release();

		string res = StableDiffusionAPI::Get("history/" + prompt_id);

		json result;
		result = json::parse(res);

		for (auto& e : result) {
			for (auto& [key, value] : e["outputs"].items()) {
				if (value.contains("images")) {
					for (auto& image : value["images"]) {
						if (image["type"] == "output") {
							auto filename = image["filename"].template get<std::string>();
							auto subfolder = image["subfolder"].template get<std::string>();
							auto type = image["type"].template get<std::string>();

							stringstream params;
							params << "filename=" << filename << "&subfolder=" << subfolder << "&type=" << type;

							string image = StableDiffusionAPI::Get("view?" + params.str());
							StableDiffusionAPI::_result = { image.begin(), image.end() };

							std::ofstream writeFile;
							writeFile.open(filename, std::ios::binary);
							writeFile.write(image.c_str(), image.size());
							writeFile.close();
							//callback(r);
						}
					}
				}
			}
		}
		});

	StableDiffusionAPI::_isWorking = false;
    return;
}

void StableDiffusionAPI::PushTask(std::function<void()> task) {
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

void StableDiffusionAPI::EventLoop() {
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

void StableDiffusionAPI::Init(const string& domain) {
	StableDiffusionAPI::domain = domain;

	eventThread = thread(&StableDiffusionAPI::EventLoop);
	eventThread.detach();
};

bool StableDiffusionAPI::isValid() {

	try {
		Client client("http://" + StableDiffusionAPI::domain);
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

string StableDiffusionAPI::Get(const string& url)
{
	Client client("http://" + StableDiffusionAPI::domain);
	auto res = client.Get("/" + url);

	if (res == nullptr)
		return nullptr;
	if (res->status != StatusCode::OK_200)
		return nullptr;

	return res->body;
}

string StableDiffusionAPI::Get(const string& url, Headers headers)
{
	Client client("http://" + StableDiffusionAPI::domain);
	auto res = client.Get("/" + url, headers);

	if (res == nullptr)
		return nullptr;
	if (res->status != StatusCode::OK_200)
		return nullptr;

	return res->body;
}

string StableDiffusionAPI::Post(const string& url, const string& data)
{
	Client client("http://" + StableDiffusionAPI::domain);

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

string StableDiffusionAPI::Post(const string& url, Headers headers, Params data)
{
	Client client("http://" + StableDiffusionAPI::domain);
	auto res = client.Post("/" + url, headers, data);

	if (res == nullptr)
		return nullptr;
	if (res->status != StatusCode::OK_200)
		return nullptr;

	return res->body;
}

string StableDiffusionAPI::Post(const string& url, const httplib::MultipartFormDataItems& params) {
	Client client("http://" + StableDiffusionAPI::domain);

	client.set_read_timeout(360, 0); // 5 seconds
	client.set_write_timeout(360, 0); // 5 seconds

	auto res = client.Post("/" + url, params);

	if (res == nullptr)
		return nullptr;
	if (res->status != StatusCode::OK_200)
		return nullptr;

	return res->body;
}

void StableDiffusionAPI::PostAsync(const string& url, const string& data, std::function<void(string)> callback) {
	PushTask([url, data, callback]() {
		string result = StableDiffusionAPI::Post(url, data);
		callback(result);
		});
}

void StableDiffusionAPI::PostAsync(const string& url, const httplib::MultipartFormDataItems& params, std::function<void(string)> callback) {
	PushTask([url, params, callback]() {
		string result = StableDiffusionAPI::Post(url, params);
		callback(result);
		});
}