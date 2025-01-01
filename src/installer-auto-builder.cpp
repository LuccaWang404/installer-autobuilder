// installer-auto-builder.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <windows.h>

using namespace std;

extern string apiURLHead;
string GetLatestVersionTag(), endTag;

void CompileEnvCheck(), ReadConfig(), GenerateURLHead(), CompileAndBackup(int head_version, int end_version), CleanAndFinish();
int historyVersionNum, latestVersionNum, ReadHistoryVersionNum();

int main()
{
    //std::cout << "Hello World!\n" << endl;
    CompileEnvCheck();
    endTag = GetLatestVersionTag();
    ReadConfig();
    historyVersionNum = ReadHistoryVersionNum(), latestVersionNum = atoi(endTag.substr(4).c_str());    //由于解析出的版本号为1.2.*，故要从第五位开始截取作为返回值
    cout << "\033[34m\033[1mThe latest version is: " << latestVersionNum << "\033[0m" << endl;
    cout << "\033[34m\033[1mLast compiled version is: " << historyVersionNum << "\033[0m" << endl;
    GenerateURLHead();
    if (historyVersionNum == latestVersionNum) {
        cout << "\033[32mNo newer version found.\033[0m" << endl;
        CleanAndFinish();
        Sleep(10000);
        exit(1);
    }
    else {
        CompileAndBackup(historyVersionNum + 1, latestVersionNum);
        CleanAndFinish();
        Sleep(10000);
        exit(0);
    }
}

// 运行程序: Ctrl + F5 或调试 >“开始执行(不调试)”菜单
// 调试程序: F5 或调试 >“开始调试”菜单

// 入门使用技巧: 
//   1. 使用解决方案资源管理器窗口添加/管理文件
//   2. 使用团队资源管理器窗口连接到源代码管理
//   3. 使用输出窗口查看生成输出和其他消息
//   4. 使用错误列表窗口查看错误
//   5. 转到“项目”>“添加新项”以创建新的代码文件，或转到“项目”>“添加现有项”以将现有代码文件添加到项目
//   6. 将来，若要再次打开此项目，请转到“文件”>“打开”>“项目”并选择 .sln 文件
