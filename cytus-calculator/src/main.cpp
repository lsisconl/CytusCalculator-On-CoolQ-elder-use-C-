#include <cqcppsdk/cqcppsdk.h>

#include <iostream>
#include<cstdlib>
#include<string>
#include <set>
#include <sstream>

using namespace cq;
using namespace std;
using Message = cq::message::Message;
using MessageSegment = cq::message::MessageSegment;

string helpInfo = "查询cytus指令格式如下\n/cytus [perfect] [good] [bad] [miss] [tp]\n其中只需打出[]内对应数值\nHave a nice day!";

string ScoreCalc(double perfectNote, double goodNote, double badNote, double missNote, double playerTp) {
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

CQ_INIT {
    on_enable([] { logging::info("启用", "cytus计算器模块已启用"); });

    on_private_message([](const PrivateMessageEvent &e) {
        try 
        {
            if (e.message.substr(0, 5) == "/cytus" || e.message.substr(0, 3) == "/cy") //判断是否具有 /cutus 或 /cy 的前缀
            {
                string blank=" ";
                int blankCount = 0;
                int searchPosition = -1;
                int blankPosition[10];
                while ((searchPosition = e.message.find(blank, searchPosition + 1)) != string::npos) 
                {
                    blankCount++;
                    blankPosition[blankCount] = searchPosition;
                    searchPosition = searchPosition + blank.length();
                }

                if (blankCount == 5) //如果消息合法则开始处理
                {
                    double perfectNote = 0;
                    double goodNote = 0;
                    double badNote = 0;
                    double missNote = 0;
                    double playerTp = 0;
                    perfectNote = atof(e.message.substr(blankPosition[1] + 1, blankPosition[2]-1).c_str());//处理消息中的信息并从string转换为double格式
                    goodNote = atof(e.message.substr(blankPosition[2] + 1, blankPosition[3] - 1).c_str());
                    badNote = atof(e.message.substr(blankPosition[3] + 1, blankPosition[4] - 1).c_str());
                    missNote = atof(e.message.substr(blankPosition[4] + 1, blankPosition[5] - 1).c_str());
                    playerTp = atof(e.message.substr(blankPosition[5] + 1,e.message.length()-1).c_str());
                    send_private_message(e.user_id, ScoreCalc(perfectNote,goodNote,badNote,missNote,playerTp));//发送成绩
                } 
                else 
                {
                    send_private_message(e.user_id, helpInfo);//如果具有合法前缀但是信息残缺显示help指令
                }
                
            } 
            else //如果不具有前缀则直接不处理
            {
                return;
            }

        } catch (ApiError &e) 
        {
            logging::warning("私聊", "私聊消息复读失败, 错误码: " + to_string(e.code));
        }
    });

    on_message([](const MessageEvent &e) {
        logging::debug("消息", "收到消息: " + e.message + "\n实际类型: " + typeid(e).name());
    });

    on_group_message([](const GroupMessageEvent &e) {
        try 
        {
            if (e.message.substr(0, 5) == "/cytus"
                || e.message.substr(0, 3) == "/cy") //判断是否具有 /cutus 或 /cy 的前缀
            {
                string blank = " ";
                int blankCount = 0;
                int searchPosition = -1;
                int blankPosition[10];
                while ((searchPosition = e.message.find(blank, searchPosition + 1)) != string::npos) {
                    blankCount++;
                    blankPosition[blankCount] = searchPosition;
                    searchPosition = searchPosition + blank.length();
                }

                if (blankCount == 5) //如果消息合法则开始处理
                {
                    double perfectNote = 0;
                    double goodNote = 0;
                    double badNote = 0;
                    double missNote = 0;
                    double playerTp = 0;
                    perfectNote = atof(e.message.substr(blankPosition[1] + 1, blankPosition[2] - 1)
                                           .c_str()); //处理消息中的信息并从string转换为double格式
                    goodNote = atof(e.message.substr(blankPosition[2] + 1, blankPosition[3] - 1).c_str());
                    badNote = atof(e.message.substr(blankPosition[3] + 1, blankPosition[4] - 1).c_str());
                    missNote = atof(e.message.substr(blankPosition[4] + 1, blankPosition[5] - 1).c_str());
                    playerTp = atof(e.message.substr(blankPosition[5] + 1, e.message.length() - 1).c_str());
                    send_group_message(e.group_id,
                                         ScoreCalc(perfectNote, goodNote, badNote, missNote, playerTp)); //发送成绩
                } else {
                    send_group_message(e.group_id, helpInfo); //如果具有合法前缀但是信息残缺显示help指令
                }

            } else //如果不具有前缀则直接不处理
            {
                return;
            }
        } 
        catch (ApiError &e) 
        {
            logging::warning("私聊", "私聊消息复读失败, 错误群: " + to_string(e.code));
        }
        e.block(); // 阻止当前事件传递到下一个插件
    });
}
