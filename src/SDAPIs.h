#include "httplib.h"
//#define CPPHTTPLIB_OPENSSL_SUPPORT FALSE

using namespace httplib;
using namespace std;

#include <vector>
#include <string>
#include <future>
#include <queue>

struct GenCharacterParams
{
	string workflowPath;
    string positivePrompt;
	string negativePrompt;
};

class StableDiffusionAPI {
public:
    StableDiffusionAPI();
    void makeCharacter(GenCharacterParams params);

	bool isWorking();
	string Result();
	bool Progress();

 /*   bool setModel(string model);
    float getProgress();
    bool Init();

    float progress;
    bool isStarted;*/
private:
	static void Init(const string& domain);
	static bool isValid();
	static string Get(const string& url);
	static string Get(const string& url, Headers headers);
	static string Post(const string& url, const string& data);
	static string Post(const string& url, Headers headers, Params data);
	static string Post(const string& url, const MultipartFormDataItems& params);
	static void PostAsync(const string& url, const string& data, function<void(string)> callback);
	static void PostAsync(const string& url, const MultipartFormDataItems& params, function<void(string)> callback);

	static void EventLoop();
	static void PushTask(std::function<void()> task);

	static string domain;
	static thread eventThread;
	static std::queue<std::function<void()>> taskQueue;
	static std::mutex queueMutex;
	static bool _isWorking;
	static string _result;
	static float _progress;
};