/*
Data Warehousing and Data Mining
Project：数据挖掘经典算法的实现——以DBLP数据集为例
Author：张彧
StudentID：1300012730
Date：2016.6 
*/

/*
Preprocess.cpp 数据预处理部分
*/
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#define MAXNAME 30000

string namelist[MAXNAME];    //作者的姓名列表

int main()
{
    ifstream fin("Dataset.txt",ios::in);    //输入：从xml中解析出的结构数据
    ofstream fout("Network.txt",ios::out);    //输出：作者们的合作网络，用于链接分析和影响力最大化分析
    ofstream fp("Namelist.txt",ios::out);    //输出：作者的姓名列表，建立id与name的对应
    ofstream fp2("Translist.txt",ios::out);    //输出：事务列表，用于频繁模式挖掘和关联规则挖掘

    string s;
    int num = 0,
        num0 = 0, 
        tmplist[40] = {0};    //当前论文的作者列表

    //逐行处理结构数据
    while (getline(fin,s)){
    	int pos1 = s.find(",");
    	int pos2 = s.find(",",pos1+1);
    	int pos3 = s.find(",",pos2+1);
    	string attr = s.substr(pos2+1,pos3-pos2-1);
    	if (attr == "author"){    //当前行表示论文的1位作者
    		string name = s.substr(pos3+1);
    		int i;
    		for (i = 0; i < num; i++)
    			if (namelist[i] == name) break;    //查找该作者是否已出现过
    		if (i >= num){    //如果该作者之前未出现，建立新的姓名列表条目
    			namelist[num] = name;
    			num++;
    		}
            tmplist[num0] = i;
            num0++;
    	}
        else if (attr == "year"){    //当前行表示论文的年份，这说明论文的作者已经罗列完毕
        	string year = s.substr(pos3+1);
        	int pos4 = s.find("/");
        	int pos5 = s.find("/",pos4+1);
        	string conf = s.substr(pos4+1,pos5-pos4-1);    //该论文所发表的会议
        	int confno;

            //将我们关注的9个会议用序号0-8表示
        	if (conf == "kdd")  confno = 0;
        	if (conf == "icdm") confno = 1;
        	if (conf == "sdm")  confno = 2;
        	if (conf == "wsdm") confno = 3;
        	if (conf == "pkdd") confno = 4;
        	if (conf == "icml") confno = 5;
        	if (conf == "nips") confno = 6;
        	if (conf == "colt") confno = 7;
        	if (conf == "ecml") confno = 8;

        	for (int i = 0; i < num0; i++)    //该论文的所有作者两两建立合作网络中的边的关系
        	    for (int j = 0; j < i; j++)
                    //边的信息包含2个顶点，会议序号，年份
                    fout<<tmplist[i]<<' '<<tmplist[j]<<' '<<confno<<' '<<year<<endl;
            if (num0 > 0){
                fp2<<num0;
                for (int i = 0; i < num0; i++)    //该论文的所有作者作为一个事务，存到事务列表中
                    fp2<<' '<<tmplist[i];
                fp2<<endl;
            }
        	num0 = 0;
        }
    }

    for (int i = 0; i < num; i++)
        fp<<namelist[i]<<endl;    //输出完整的姓名列表

    fin.close();
    fout.close();
    fp.close();
    fp2.close();
    return 0;
}
