#include "httplib.h"
//#define CPPHTTPLIB_OPENSSL_SUPPORT FALSE

using namespace httplib;
using namespace std;

#include <vector>
#include <string>
#include <future>
#include <queue>

struct T2iParams
{
	string workflowPath;
	string positivePrompt;
	string negativePrompt;
	int width;
	int height;
};

struct PoseT2iParams
{
	string workflowPath;
    string positivePrompt;
	string negativePrompt;
	int width;
	int height;

	string posePath;
};

struct RegionRedrawParams
{
	PoseT2iParams prompt;

	bool multiRegion; // 수정 구역이 여러 곳인 경우
	string imagePath;
	float strength;
	string redrawRegion;
};

struct RefRedrawParams
{
	RegionRedrawParams region;

	string referencePath;
	string referenceRegion;
};

struct FaceRefRedrawParams
{
	RegionRedrawParams region;

	string referencePath;
	string referenceRegion;
};

struct HandsRedrawParams
{
	RegionRedrawParams region;
};

class StableDiffusionAPI {
public:
    StableDiffusionAPI();
	void callSD(T2iParams params);
    void callSD(PoseT2iParams params);
	void callSD(RegionRedrawParams params);
	void callSD(RefRedrawParams params);
	void callSD(FaceRefRedrawParams params);
	void callSD(HandsRedrawParams params);

	bool isWorking();
	string Result();
	float Progress();

 /*   bool setModel(string model);
    float getProgress();
    bool Init();

    float progress;
    bool isStarted;*/
private:
	string uploadImage(char const* filePath);
	void getImages(string client_id, string prompt_id);

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
	static void PushTask(function<void()> task);

	static string domain;
	static thread eventThread;
	static queue<function<void()>> taskQueue;
	static mutex queueMutex;
	static bool _isWorking;
	static string _result;
	static float _progress;
};