#include <iostream>
#include <fstream>
#include <sstream>
#include <windows.h>
#include "include/rapidjson/document.h"
#include "include/rapidjson/stringbuffer.h"
#include "include/rapidjson/writer.h"
#include "include/rapidjson/istreamwrapper.h"
#include "include/rapidjson/filewritestream.h"
#include "include/rapidjson/filereadstream.h"
#include "include/rapidjson/prettywriter.h"

using namespace std;
using namespace rapidjson;

extern string apiURLHead;

void GenerateURLHead();

string GetLatestVersionTag() {
	
	GenerateURLHead();
	string getAPICommand = "curl " + apiURLHead + " --output version-info.json";
	//��GitHub API���������°汾��json�ļ�
	system(getAPICommand.c_str());

	//����json�ļ������ذ汾��
	ifstream versionJson("version-info.json");
	IStreamWrapper isw(versionJson);
	Document versionJsonFile;
	versionJsonFile.ParseStream(isw);
	//const char*תstring
	string versionTag(versionJsonFile["tag_name"].GetString());

	return versionTag;

}