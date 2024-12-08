
#include "SDAPIs.h"

#include "nlohmannjson.hpp"

//#include "../vendor/easywsclient/easywsclient/easywsclient.hpp"
#include "easywsclient.hpp"

#include <string>
#include <queue>
#include <future>
#include <cstdlib>
#include <filesystem>

using namespace nlohmann;
using easywsclient::WebSocket;
namespace fs = std::filesystem;

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

float StableDiffusionAPI::Progress() {
	return StableDiffusionAPI::_progress;
}

string StableDiffusionAPI::uploadImage(char const* filePath) {
	Client client("http://" + StableDiffusionAPI::domain);

	fs::path path{ fs::path(filePath) };
	std::string fileName{ path.filename().string() };

	const char* ext;
	ext = strrchr(fileName.c_str(), '.');
	std::string fileExt(++ext);

	ifstream ifs(filePath, ios::binary | ios::ate);
	ifstream::pos_type pos = ifs.tellg();

	std::vector<char> result(pos);

	ifs.seekg(0, ios::beg);
	ifs.read(&result[0], pos);

	string imgBinary(reinterpret_cast<char*>(result.data()), result.size());

	httplib::MultipartFormDataItems items = {
		{ "image", imgBinary, fileName, "image/" + fileExt },
	};

	auto res = client.Post("/upload/image", items);

	return fileName;
}

void StableDiffusionAPI::getImages(string client_id, string prompt_id) {
	stringstream url;
	url << "ws://" << StableDiffusionAPI::domain << "/ws?clientId=" << client_id;
	unique_ptr<WebSocket> ws(WebSocket::from_url(url.str()));
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
					StableDiffusionAPI::_progress = step / maxStep;
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
			catch (int ex) {

			}
			});
	}

	ws.release();

	// get images
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

						std::ofstream writeFile;
						writeFile.open(filename, std::ios::binary);
						writeFile.write(image.c_str(), image.size());
						writeFile.close();

						StableDiffusionAPI::_result = { image.begin(), image.end() };
					}
				}
			}
		}
	}
}

void StableDiffusionAPI::callSD(PoseT2iParams params) {
	if (StableDiffusionAPI::_isWorking)
		return;

	StableDiffusionAPI::_isWorking = true;

	string img1 = uploadImage(params.posePath.c_str());

	std::ifstream raw_workflow(params.workflowPath);
	json workflow;
	raw_workflow >> workflow;

	UUID uuid;
	UuidCreate(&uuid);

	char* client_id = nullptr;
	UuidToStringA(&uuid, (RPC_CSTR*)&client_id);

	srand(time(0));
	workflow["3"]["inputs"]["seed"] = rand();
	workflow["6"]["inputs"]["text"] = params.positivePrompt;
	workflow["7"]["inputs"]["text"] = params.negativePrompt;

	workflow["5"]["inputs"]["width"] = params.width;
	workflow["5"]["inputs"]["height"] = params.height;

	workflow["75"]["inputs"]["image"] = img1; // load pose

	json prompt;
	prompt["prompt"] = workflow;
	prompt["client_id"] = client_id;

	std::cout << prompt << std::endl;

	json result;
	result = json::parse(StableDiffusionAPI::Post("prompt", prompt.dump()));
	string prompt_id = result["prompt_id"];

	function<void(string)> callback = [this](string file) {
	};

	PushTask([this, client_id, prompt_id, callback]() {
		getImages(client_id, prompt_id);
	});

	StableDiffusionAPI::_isWorking = false;
    return;
}

void StableDiffusionAPI::callSD(RefRedrawParams params) {
	if (StableDiffusionAPI::_isWorking)
		return;

	StableDiffusionAPI::_isWorking = true;

	std::ifstream raw_workflow(params.region.prompt.workflowPath);
	json workflow;
	raw_workflow >> workflow;

	UUID uuid;
	UuidCreate(&uuid);

	char* client_id = nullptr;
	UuidToStringA(&uuid, (RPC_CSTR*)&client_id);

	srand(time(0));
	workflow["3"]["inputs"]["seed"] = rand();
	workflow["3"]["inputs"]["denoise"] = params.region.strength;

	workflow["6"]["inputs"]["text"] = params.region.prompt.positivePrompt;
	workflow["7"]["inputs"]["text"] = params.region.prompt.negativePrompt;

	workflow["97"]["inputs"]["image"] = uploadImage(params.region.imagePath.c_str()); // default image
	workflow["125"]["inputs"]["image"] = uploadImage(params.region.prompt.posePath.c_str()); // load pose
	workflow["114"]["inputs"]["image"] = uploadImage(params.referencePath.c_str()); // load reference

	workflow["102"]["inputs"]["text_input"] = params.region.redrawRegion;
	workflow["117"]["inputs"]["text_input"] = params.referenceRegion;

	json prompt;
	prompt["prompt"] = workflow;
	prompt["client_id"] = client_id;

	cout << prompt << endl;

	json result;
	result = json::parse(StableDiffusionAPI::Post("prompt", prompt.dump()));
	string prompt_id = result["prompt_id"];

	function<void(string)> callback = [this](string file) {
		};

	PushTask([this, client_id, prompt_id, callback]() {
		getImages(client_id, prompt_id);
		});

	StableDiffusionAPI::_isWorking = false;
	return;
}

void StableDiffusionAPI::callSD(FaceRefRedrawParams params) {
	if (StableDiffusionAPI::_isWorking)
		return;

	StableDiffusionAPI::_isWorking = true;

	std::ifstream raw_workflow(params.region.prompt.workflowPath);
	json workflow;
	raw_workflow >> workflow;

	UUID uuid;
	UuidCreate(&uuid);

	char* client_id = nullptr;
	UuidToStringA(&uuid, (RPC_CSTR*)&client_id);

	srand(time(0));
	workflow["3"]["inputs"]["seed"] = rand();
	workflow["3"]["inputs"]["denoise"] = params.region.strength;

	workflow["6"]["inputs"]["text"] = params.region.prompt.positivePrompt;
	workflow["7"]["inputs"]["text"] = params.region.prompt.negativePrompt;

	workflow["97"]["inputs"]["image"] = uploadImage(params.region.imagePath.c_str()); // default image
	workflow["114"]["inputs"]["image"] = uploadImage(params.referencePath.c_str()); // load reference

	workflow["102"]["inputs"]["text_input"] = params.region.redrawRegion;
	workflow["117"]["inputs"]["text_input"] = params.referenceRegion;

	json prompt;
	prompt["prompt"] = workflow;
	prompt["client_id"] = client_id;


	cout << prompt << endl;

	json result;
	result = json::parse(StableDiffusionAPI::Post("prompt", prompt.dump()));
	string prompt_id = result["prompt_id"];

	function<void(string)> callback = [this](string file) {
		};

	PushTask([this, client_id, prompt_id, callback]() {
		getImages(client_id, prompt_id);
		});

	StableDiffusionAPI::_isWorking = false;
	return;
}

void StableDiffusionAPI::callSD(RegionRedrawParams params) {
	if (StableDiffusionAPI::_isWorking)
		return;

	StableDiffusionAPI::_isWorking = true;

	std::ifstream raw_workflow(params.prompt.workflowPath);
	json workflow;
	raw_workflow >> workflow;

	UUID uuid;
	UuidCreate(&uuid);

	char* client_id = nullptr;
	UuidToStringA(&uuid, (RPC_CSTR*)&client_id);

	srand(time(0));
	workflow["36"]["inputs"]["seed"] = rand();
	workflow["36"]["inputs"]["denoise"] = params.strength;

	workflow["23"]["inputs"]["text"] = params.prompt.positivePrompt;
	workflow["24"]["inputs"]["text"] = params.prompt.negativePrompt;

	workflow["10"]["inputs"]["image"] = uploadImage(params.imagePath.c_str()); // default image
	workflow["60"]["inputs"]["image"] = uploadImage(params.prompt.posePath.c_str()); // load pose

	workflow["12"]["inputs"]["text_input"] = params.redrawRegion;

	workflow["63"]["inputs"]["value"] = params.multiRegion;

	json prompt;
	prompt["prompt"] = workflow;
	prompt["client_id"] = client_id;

	json result;
	result = json::parse(StableDiffusionAPI::Post("prompt", prompt.dump()));
	string prompt_id = result["prompt_id"];

	function<void(string)> callback = [this](string file) {
		};

	PushTask([this, client_id, prompt_id, callback]() {
		getImages(client_id, prompt_id);
		});

	StableDiffusionAPI::_isWorking = false;
	return;
}

void StableDiffusionAPI::callSD(HandsRedrawParams params) {
	if (StableDiffusionAPI::_isWorking)
		return;

	StableDiffusionAPI::_isWorking = true;

	std::ifstream raw_workflow(params.region.prompt.workflowPath);
	json workflow;
	raw_workflow >> workflow;

	UUID uuid;
	UuidCreate(&uuid);

	char* client_id = nullptr;
	UuidToStringA(&uuid, (RPC_CSTR*)&client_id);

	srand(time(0));
	workflow["130"]["inputs"]["seed"] = rand();
	workflow["130"]["inputs"]["denoise"] = params.region.strength;

	workflow["103"]["inputs"]["text"] = params.region.prompt.positivePrompt;
	workflow["104"]["inputs"]["text"] = params.region.prompt.negativePrompt;

	workflow["72"]["inputs"]["image"] = uploadImage(params.region.imagePath.c_str()); // default image
	//workflow["60"]["inputs"]["image"] = uploadImage(params.region.prompt.posePath.c_str()); // load pose

	workflow["74"]["inputs"]["value"] = params.region.multiRegion;

	json prompt;
	prompt["prompt"] = workflow;
	prompt["client_id"] = client_id;

	json result;
	result = json::parse(StableDiffusionAPI::Post("prompt", prompt.dump()));
	string prompt_id = result["prompt_id"];

	function<void(string)> callback = [this](string file) {
		};

	PushTask([this, client_id, prompt_id, callback]() {
		getImages(client_id, prompt_id);
		});

	StableDiffusionAPI::_isWorking = false;
	return;
}

void StableDiffusionAPI::callSD(T2iParams params) {
	if (StableDiffusionAPI::_isWorking)
		return;

	StableDiffusionAPI::_isWorking = true;

	std::ifstream raw_workflow(params.workflowPath);
	json workflow;
	raw_workflow >> workflow;

	UUID uuid;
	UuidCreate(&uuid);

	char* client_id = nullptr;
	UuidToStringA(&uuid, (RPC_CSTR*)&client_id);

	srand(time(0));
	workflow["3"]["inputs"]["seed"] = rand();

	workflow["6"]["inputs"]["text"] = params.positivePrompt;
	workflow["7"]["inputs"]["text"] = params.negativePrompt;

	workflow["5"]["inputs"]["width"] = params.width;
	workflow["5"]["inputs"]["height"] = params.height;

	json prompt;
	prompt["prompt"] = workflow;
	prompt["client_id"] = client_id;

	json result;
	result = json::parse(StableDiffusionAPI::Post("prompt", prompt.dump()));
	string prompt_id = result["prompt_id"];

	function<void(string)> callback = [this](string file) {
		};

	PushTask([this, client_id, prompt_id, callback]() {
		getImages(client_id, prompt_id);
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