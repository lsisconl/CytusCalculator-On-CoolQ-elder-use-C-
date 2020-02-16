#include <cqcppsdk/cqcppsdk.h>

#include <cstdlib>
#include <iostream>
#include <set>
#include <sstream>
#include <string>

using namespace cq;
using namespace std;
using Message = cq::message::Message;
using MessageSegment = cq::message::MessageSegment;

string helpInfo =
    "查询cytus指令格式如下\n1./cytus (或/cy) [perfect] [good] [bad] [miss] [tp]来查询彩P黑P\n2./cytus (或/cy) least "
    "[perfect] [tp]来查询在Million Master情况下达到预期TP最少需要的黑P数\nTips：其中只需打出[]内对应数值即可\nHave a "
    "nice day!OvO";
string errorInfo = "参数不正确或为非法参数，请输入/cy或/cytus以获取指令";

string ProcessScore(string message, int blankPosition[10]);
string ScoreCalc(double perfectNote, double goodNote, double badNote, double missNote, double playerTp);
string JudgeMessage(string message);

string ScoreCalc(double perfectNote, double goodNote, double badNote, double missNote, double playerTp) //计算彩P黑P
{
    string info = "";
    double colourPerfect = 0;
    double blackPerfect = 0;
    double noteNumber = 0;
    double realTp = 0;
    noteNumber = perfectNote + goodNote + badNote + missNote;
    blackPerfect = (((perfectNote * double(100)) / noteNumber) - (playerTp - (goodNote * double(30)) / noteNumber))
                   / (double(30) / noteNumber);
    colourPerfect = perfectNote - blackPerfect;
    realTp = (int(colourPerfect + double(0.5)) * double(100)) / noteNumber
             + (int(blackPerfect + double(0.5)) * double(70)) / noteNumber + (int(goodNote) * double(30)) / noteNumber;
    info = "您的ColourPerfect是: " + to_string(int(colourPerfect + double(0.5))) + "\n" + "您的BlackPerfect是: "
           + to_string(int(blackPerfect + double(0.5))) + "\n" + "您的真实Tp为: " + to_string(realTp);
    return info;
}

string JudgeMessage(string message) //判断消息是否为调用插件
{
    if (message.substr(0, 5) == "/cytus" || message.substr(0, 3) == "/cy") //判断是否具有 /cutus 或 /cy 的前缀
    {
        string blank = " ";
        int blankCount = 0;
        int searchPosition = -1;
        int blankPosition[10];
        while ((searchPosition = message.find(blank, searchPosition + 1)) != string::npos) {
            blankCount++;
            blankPosition[blankCount] = searchPosition;
            searchPosition = searchPosition + blank.length();
        }
        if (blankCount == 5) //如果消息合法且为计算彩P黑P则开始处理
        {
            return ProcessScore(message, blankPosition);
        } else if (blankCount == 3
                   && (message.substr(blankPosition[1] + 1, 5) == "least")) //计算需要多少黑P来达到预期TP
        {
            double noteNumber;
            double playerTp;
            double leastBlackPerfect;
            noteNumber = atof(message.substr(blankPosition[2] + 1, blankPosition[3] - blankPosition[2] - 1).c_str());
            playerTp = atof(message.substr(blankPosition[3] + 1, message.length() - blankPosition[3] - 1).c_str());
            leastBlackPerfect = double(noteNumber)*(double(100) - playerTp) / double(30) ;
            return "在Million Master的情况下，您需要至少：" + to_string(int(leastBlackPerfect + double(0.5)))
                       + "个BlackPerfect来达到TP：" + to_string(playerTp);
        } else {
            return "HELP";
        }

    } else //如果不具有前缀则直接不处理
    {
        return "MISS";
    }
}

string ProcessScore(string message, int blankPosition[10]) {
    double perfectNote = 0;
    double goodNote = 0;
    double badNote = 0;
    double missNote = 0;
    double playerTp = 0;
    perfectNote = atof(message.substr(blankPosition[1] + 1, blankPosition[2] - blankPosition[1] - 1)
                           .c_str()); //处理消息中的信息并从string转换为double格式
    goodNote = atof(message.substr(blankPosition[2] + 1, blankPosition[3] - blankPosition[2] - 1).c_str());
    badNote = atof(message.substr(blankPosition[3] + 1, blankPosition[4] - blankPosition[3] - 1).c_str());
    missNote = atof(message.substr(blankPosition[4] + 1, blankPosition[5] - blankPosition[4] - 1).c_str());
    playerTp = atof(message.substr(blankPosition[5] + 1, message.length() - blankPosition[5] - 1).c_str());
    return ScoreCalc(perfectNote, goodNote, badNote, missNote, playerTp);
    
}

CQ_INIT {
    on_enable([] { logging::info("启用", "cytus计算器模块已启用"); });

    on_private_message([](const PrivateMessageEvent &e) {
        try {
            string returnInfo = "";
            returnInfo = JudgeMessage(e.message);
            if (returnInfo == "MISS") {
                return;
            } else if (returnInfo == "HELP") {
                send_private_message(e.user_id, helpInfo);
                return;
            }
            send_private_message(e.user_id, returnInfo);

        } catch (ApiError &e) {
            logging::warning("私聊", "消息发送失败, 错误码: " + to_string(e.code));
        }
    });

    on_message([](const MessageEvent &e) {
        logging::debug("消息", "收到消息: " + e.message + "\n实际类型: " + typeid(e).name());
    });

    on_group_message([](const GroupMessageEvent &e) {
        try {
            string returnInfo;
            returnInfo = JudgeMessage(e.message);
            if (returnInfo == "MISS") {
                return;
            } else if (returnInfo == "HELP") {
                send_group_message(e.group_id, helpInfo);
                return;
            }
            send_group_message(e.group_id, returnInfo);
        } catch (ApiError &e) {
            logging::warning("群聊", "消息发送失败, 错误码: " + to_string(e.code));
        }
        e.block(); // 阻止当前事件传递到下一个插件
    });
}
