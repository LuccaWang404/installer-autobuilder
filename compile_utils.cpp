#include <iostream>
#include <fstream>
#include <sstream>
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

extern string Version, head_url, owner, repo, sevenZipBin, gitBin, baseURLHeadFinal, dlURLHead, srcArchiveURLHead, releaseNameBeforeTag, releaseNameAfterTag, isccBin, innoScriptName, endTag;
extern int historyVersionNum, latestVersionNum;
int head_version, end_version, i, GetLatestVersionNum();
void UpdateConfigVersionNumber(int num);
string GetLatestVersionTag();


//从文件中读取上次编译版本号
int ReadHistoryVersionNum() {

	//rapidjson文件流操作
	ifstream config("config.json");
	IStreamWrapper configISW(config);
	Document mainConfig;
	mainConfig.ParseStream(configISW);
	return mainConfig["last_compiled_version"].GetInt();

}

//编译后清理临时文件并向控制台输出最后编译版本
void CleanAndFinish() {

	cout << "\033[32mCleaning temporary .iss file... \033[0m" << endl;
	system("del /f /s tmp.script.iss");
	system("del /f /s version-info.json");
	cout << "\033[32mLoop finished with number: " << i - 1 << "\033[0m" << endl;
	
}

//编译并备份Releases至Archives文件夹、源代码存档至Source.Backup文件夹
void CompileAndBackup(int head_version, int end_version) {

	string tagHead = endTag.substr(0, 4);
	system("md Archives");
	system("md Source.Backup");
	string removePreviousRepo, cloneRemoteRepo, pullRemoteRepo;
	removePreviousRepo = "rmdir /s /q " + repo;
	cloneRemoteRepo = gitBin + " clone " + baseURLHeadFinal + owner + "/" + repo;
	pullRemoteRepo = "cd "+ repo + " && \..\\"+ gitBin + " pull && cd ..";
	if (_access(repo.c_str(), RW) == 0) {
		cout << "\033[32mR/W project repository found. Pulling changes...\033[0m" << endl;
		system(pullRemoteRepo.c_str());
	}
	else {
		cout << "\033[0m\033[1;31mNo R/W project repository found! Cloning into " + repo + "...\033[0m" << endl;
		system(removePreviousRepo.c_str());
		system(cloneRemoteRepo.c_str());
	}
	for (i = head_version; i <= end_version; i++) {
		cout << "\033[34m\033[1mLoop number: " << i << "\033[0m" << endl << endl;
		string versionNum = to_string(i), releaseName, extractFolderName, dlRelease, dlSrcArchive, backupSrcArchive, extractRelease, insertVersionInfo, typeTMPScript, compileInstallerEXE, cleanTMP, backupRelease;
		releaseName = releaseNameBeforeTag + tagHead + versionNum + releaseNameAfterTag;
		extractFolderName = releaseName.substr(0, releaseName.length() - 4);
		dlRelease = "curl -O " + dlURLHead + tagHead + versionNum + "/" + releaseName;
		dlSrcArchive = "curl -o " + repo + "-source-" + tagHead + versionNum + ".zip " + srcArchiveURLHead + tagHead + versionNum + "/" + tagHead + versionNum + ".zip";
		extractRelease = sevenZipBin + " x " + releaseName + " -o" + extractFolderName;
		insertVersionInfo = "echo #define ExecutableVersion \"" + versionNum + "\" > tmp.script.iss";
		typeTMPScript = "type " + innoScriptName + " >> tmp.script.iss";
		compileInstallerEXE = isccBin + " tmp.script.iss";
		cleanTMP = "rmdir /s /q " + extractFolderName;
		backupRelease = "move " + releaseName + " Archives";
		backupSrcArchive = "move " + repo + "-source-" + tagHead + versionNum + ".zip Source.Backup";
		system(dlRelease.c_str());
		system(dlSrcArchive.c_str());
		system(extractRelease.c_str());
		system(insertVersionInfo.c_str());
		system(typeTMPScript.c_str());
		system(compileInstallerEXE.c_str());
		cout << endl;
		cout << "\033[34m\033[1mCleaning caches...\033[0m" << endl;
		system(cleanTMP.c_str());
		cout << "\033[34m\033[1mBacking up release...\033[0m" << endl;
		system(backupRelease.c_str());
		system(backupSrcArchive.c_str());
		cout << endl;
		UpdateConfigVersionNumber(i);
	}
}