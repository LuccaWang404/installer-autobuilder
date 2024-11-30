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

//�ж������ļ��Ƿ�����ҿɶ�д
bool IsConfigExist() {
	if (_access("config.json", RW) == 0) {
		return true;
	}
	else return false;
}

//���ö�ȡ������ʹ��rapidjson
//��ȡ�󱻸�ֵ������baseURLHead, baseURLHeadFinal, mirrorURLHead, owner, repo, releaseNameBeforeTag, releaseNameAfterTag, innoScriptName, isccBinDir, EULA, icon
//test passed. debug passed.
void ReadConfig() {

	//rapidjson�ļ�������
	ifstream config("config.json");
	IStreamWrapper configISW(config);
	Document mainConfig;
	mainConfig.ParseStream(configISW);

	//������ֵ
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

//�жϱ���ű��Ƿ�����ҿɶ�д
bool IsScriptExist() {
	if (_access(innoScriptName.c_str(), RW) == 0) {
		return true;
	}
	else return false;
}

//�ж�inno setup�������Ƿ�����ҿɶ�ȡ
bool IsInnoSetupExist() {
	if (_access(isccBin.c_str(), R) == 0) {
		return true;
	}
	else return false;
}

//�������ɺ�����ʹ��rapidjson
//test passed. debug passed.
void GenerateConfig() {
//------------------------------------------���ɲ���------------------------------------------
	Document mainConfig;
	mainConfig.SetObject();
	Document::AllocatorType& configJsonAllocator = mainConfig.GetAllocator();

	//github release�������
	mainConfig.AddMember("mirror", false, configJsonAllocator);
	//mainConfig.AddMember("base_url", "https://github.com/", configJsonAllocator);
	//mainConfig.AddMember("mirror_url", "https://github.moeyy.xyz/", configJsonAllocator);
	mainConfig.AddMember("mirror_url", "", configJsonAllocator);
	mainConfig.AddMember("owner", "", configJsonAllocator);
	mainConfig.AddMember("repo", "", configJsonAllocator);
	mainConfig.AddMember("7z_bin", "\.\\bin\\7z\\7z.exe", configJsonAllocator);

	//�ļ������������
	Value releaseObj(kObjectType);
	releaseObj.AddMember("before_tag", "", configJsonAllocator);
	releaseObj.AddMember("after_tag", "", configJsonAllocator);
	mainConfig.AddMember("release_name", releaseObj, configJsonAllocator);
	
	//�������
	mainConfig.AddMember("iscc_bin", "\.\\bin\\inno-setup-6\\ISCC.exe", configJsonAllocator);
	mainConfig.AddMember("inno_script", "", configJsonAllocator);
	//mainConfig.AddMember("eula", "assets/", configJsonAllocator);
	//mainConfig.AddMember("icon", "assets/", configJsonAllocator);
	mainConfig.AddMember("last_compiled_version", 0, configJsonAllocator);

	//���뱸�����
	mainConfig.AddMember("git_bin", "\.\\bin\\git-2.40.0\\bin\\git.exe", configJsonAllocator);

	StringBuffer configSB;
	Writer<StringBuffer> configWriter(configSB);
	mainConfig.Accept(configWriter);

//------------------------------------------д�벿��------------------------------------------
	mainConfig.Parse(configSB.GetString());
	FILE* config;
	int flag = fopen_s(&config, "config.json", "w");
	char configWriterBuffer[65536];
	FileWriteStream configFileWriter(config, configWriterBuffer, sizeof(configWriterBuffer));
	PrettyWriter<FileWriteStream> fsConfigWriter(configFileWriter);
	mainConfig.Accept(fsConfigWriter);
	fclose(config);
}

//������ҪHTTP����
void GenerateURLHead() {

	ReadConfig();
	apiURLHead = "https://api.github.com/repos/" + owner + "/" + repo + "/releases/latest";

	//��û�о�����վ����ʹ��GitHubֱ������
	if (mirror) baseURLHeadFinal = mirrorURLHead + "https://github.com/";
	else baseURLHeadFinal = "https://github.com/";

	dlURLHead = baseURLHeadFinal + owner + "/" + repo + "/releases/download/";
	srcArchiveURLHead = baseURLHeadFinal + owner + "/" + repo + "/archive/refs/tags/";

}


//���뻷�����
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

//ÿ�α�����ɺ��������ļ���д�뵱ǰ�汾
//debug passed.
void UpdateConfigVersionNumber(int num) {

	cout << "\033[32mWriting compiled version config...\033[0m" << endl;
	
	//���ļ���
	FILE* config;
	int flag = fopen_s(&config, "config.json", "r");
	char configReaderBuffer[65536];
	FileReadStream configFileReader(config, configReaderBuffer, sizeof(configReaderBuffer));
	Document mainConfig;
	mainConfig.ParseStream(configFileReader);
	fclose(config);
	
	//�޸�"last_compiled_version"��ֵ
	mainConfig["last_compiled_version"].SetInt(num);

	//д�������ļ�
	flag = fopen_s(&config, "config.json", "w");
	char configWriterBuffer[65536];
	FileWriteStream configFileWriter(config, configWriterBuffer, sizeof(configWriterBuffer));
	PrettyWriter<FileWriteStream> fsConfigWriter(configFileWriter);
	mainConfig.Accept(fsConfigWriter);
	fclose(config);

}