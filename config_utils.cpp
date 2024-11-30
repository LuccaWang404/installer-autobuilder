#include <iostream>
#include <fstream>
#include <windows.h>
#include <io.h>
#include "include/rapidjson/document.h"
#include "include/rapidjson/stringbuffer.h"
#include "include/rapidjson/writer.h"
#include "include/rapidjson/istreamwrapper.h"
#include "include/rapidjson/filewritestream.h"
#include "include/rapidjson/filereadstream.h"
#include "include/rapidjson/prettywriter.h"

#define RW 06
#define R 04
#define W 02

using namespace std;
using namespace rapidjson;

string baseURLHead, baseURLHeadFinal, mirrorURLHead, srcArchiveURLHead, owner, repo, sevenZipBin, releaseNameBeforeTag, releaseNameAfterTag, innoScriptName, isccBin, gitBin, apiURLHead, dlURLHead;
//EULA, icon,
bool mirror;

//判断配置文件是否存在且可读写
bool IsConfigExist() {
	if (_access("config.json", RW) == 0) {
		return true;
	}
	else return false;
}

//配置读取函数，使用rapidjson
//读取后被赋值变量：baseURLHead, baseURLHeadFinal, mirrorURLHead, owner, repo, releaseNameBeforeTag, releaseNameAfterTag, innoScriptName, isccBinDir, EULA, icon
//test passed. debug passed.
void ReadConfig() {

	//rapidjson文件流操作
	ifstream config("config.json");
	IStreamWrapper configISW(config);
	Document mainConfig;
	mainConfig.ParseStream(configISW);

	//变量赋值
	mirror = mainConfig["mirror"].GetBool();
	//baseURLHead = mainConfig["base_url"].GetString();
	mirrorURLHead = mainConfig["mirror_url"].GetString();
	owner = mainConfig["owner"].GetString();
	repo = mainConfig["repo"].GetString();
	sevenZipBin = mainConfig["7z_bin"].GetString();

	Value& releaseName = mainConfig["release_name"];
	releaseNameBeforeTag = releaseName["before_tag"].GetString();
	releaseNameAfterTag = releaseName["after_tag"].GetString();

	isccBin = mainConfig["iscc_bin"].GetString();
	innoScriptName = mainConfig["inno_script"].GetString();
	//EULA = mainConfig["eula"].GetString();
	//icon = mainConfig["icon"].GetString();

	gitBin = mainConfig["git_bin"].GetString();

}

//判断编译脚本是否存在且可读写
bool IsScriptExist() {
	if (_access(innoScriptName.c_str(), RW) == 0) {
		return true;
	}
	else return false;
}

//判断inno setup程序本体是否存在且可读取
bool IsInnoSetupExist() {
	if (_access(isccBin.c_str(), R) == 0) {
		return true;
	}
	else return false;
}

//配置生成函数，使用rapidjson
//test passed. debug passed.
void GenerateConfig() {
//------------------------------------------生成部分------------------------------------------
	Document mainConfig;
	mainConfig.SetObject();
	Document::AllocatorType& configJsonAllocator = mainConfig.GetAllocator();

	//github release下载相关
	mainConfig.AddMember("mirror", false, configJsonAllocator);
	//mainConfig.AddMember("base_url", "https://github.com/", configJsonAllocator);
	//mainConfig.AddMember("mirror_url", "https://github.moeyy.xyz/", configJsonAllocator);
	mainConfig.AddMember("mirror_url", "", configJsonAllocator);
	mainConfig.AddMember("owner", "", configJsonAllocator);
	mainConfig.AddMember("repo", "", configJsonAllocator);
	mainConfig.AddMember("7z_bin", "\.\\bin\\7z\\7z.exe", configJsonAllocator);

	//文件命名规则相关
	Value releaseObj(kObjectType);
	releaseObj.AddMember("before_tag", "", configJsonAllocator);
	releaseObj.AddMember("after_tag", "", configJsonAllocator);
	mainConfig.AddMember("release_name", releaseObj, configJsonAllocator);
	
	//编译相关
	mainConfig.AddMember("iscc_bin", "\.\\bin\\inno-setup-6\\ISCC.exe", configJsonAllocator);
	mainConfig.AddMember("inno_script", "", configJsonAllocator);
	//mainConfig.AddMember("eula", "assets/", configJsonAllocator);
	//mainConfig.AddMember("icon", "assets/", configJsonAllocator);
	mainConfig.AddMember("last_compiled_version", 0, configJsonAllocator);

	//代码备份相关
	mainConfig.AddMember("git_bin", "\.\\bin\\git-2.40.0\\bin\\git.exe", configJsonAllocator);

	StringBuffer configSB;
	Writer<StringBuffer> configWriter(configSB);
	mainConfig.Accept(configWriter);

//------------------------------------------写入部分------------------------------------------
	mainConfig.Parse(configSB.GetString());
	FILE* config;
	int flag = fopen_s(&config, "config.json", "w");
	char configWriterBuffer[65536];
	FileWriteStream configFileWriter(config, configWriterBuffer, sizeof(configWriterBuffer));
	PrettyWriter<FileWriteStream> fsConfigWriter(configFileWriter);
	mainConfig.Accept(fsConfigWriter);
	fclose(config);
}

//生成主要HTTP链接
void GenerateURLHead() {

	ReadConfig();
	apiURLHead = "https://api.github.com/repos/" + owner + "/" + repo + "/releases/latest";

	//如没有镜像网站，则使用GitHub直连下载
	if (mirror) baseURLHeadFinal = mirrorURLHead + "https://github.com/";
	else baseURLHeadFinal = "https://github.com/";

	dlURLHead = baseURLHeadFinal + owner + "/" + repo + "/releases/download/";
	srcArchiveURLHead = baseURLHeadFinal + owner + "/" + repo + "/archive/refs/tags/";

}


//编译环境检查
void CompileEnvCheck() {
	if (!IsConfigExist()) {
		GenerateConfig();
		cout << "\033[0m\033[1;31mConfig file not found! Re-run the program after modifying \"config.json\"!\033[0m" << endl;
		system("pause");
		exit(1);
	}
	ReadConfig();
	if (!IsScriptExist()) {
		cout << "\033[0m\033[1;31mInno Setup .iss script not found!\033[0m" << endl;
		system("pause");
		exit(1);
	}
}

//每次编译完成后向配置文件中写入当前版本
//debug passed.
void UpdateConfigVersionNumber(int num) {

	cout << "\033[32mWriting compiled version config...\033[0m" << endl;
	
	//打开文件流
	FILE* config;
	int flag = fopen_s(&config, "config.json", "r");
	char configReaderBuffer[65536];
	FileReadStream configFileReader(config, configReaderBuffer, sizeof(configReaderBuffer));
	Document mainConfig;
	mainConfig.ParseStream(configFileReader);
	fclose(config);
	
	//修改"last_compiled_version"键值
	mainConfig["last_compiled_version"].SetInt(num);

	//写入配置文件
	flag = fopen_s(&config, "config.json", "w");
	char configWriterBuffer[65536];
	FileWriteStream configFileWriter(config, configWriterBuffer, sizeof(configWriterBuffer));
	PrettyWriter<FileWriteStream> fsConfigWriter(configFileWriter);
	mainConfig.Accept(fsConfigWriter);
	fclose(config);

}