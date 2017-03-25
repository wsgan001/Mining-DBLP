/*
Data Warehousing and Data Mining
Project：数据挖掘经典算法的实现——以DBLP数据集为例
Author：张彧
StudentID：1300012730
Date：2016.6 
*/

/*
AssocRule.cpp 关联规则挖掘部分
*/
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

#define NODE 23782    //项的个数
#define FREQN 1335    //2阶频繁项集的个数
#define MINCONF 0.9    //最小置信度
#define TRAN 22998    //事务的个数
#define RATIO 0.33    //导师与学生的支持度比值(发表论文数比值，在挖掘导师-学生关系中用到)

int transn[TRAN] = {0},    //每个事务所含项的个数
    trans[TRAN][45] = {0},    //每个事务含的所有项(用变长数组vector写可能更好)
    freq[FREQN][2] = {0},    //所有的2阶频繁项集
    totSA = 0,    //挖掘出的导师-学生关系总数
    totAR = 0;    //挖掘出的关联规则总数

string namelist[NODE];

//判断项x是否在事务y中
bool Found(int x, int y)
{
	for (int i = 0; i < transn[y]; i++)
		if (trans[y][i] == x) 
			return 1;
	return 0;
}

int main()
{
	ifstream fin("FreqItem (FPGrowth).txt",ios::in);    //输入：挖掘出的所有频繁项集
    ofstream fout("AssocRule.txt",ios::out);    //输出：挖掘出的关联规则和导师-学生关系
    ifstream fp("Namelist.txt",ios::in);    //输入：作者的姓名列表
    ifstream fp2("Translist.txt",ios::in);    //输入：事务列表

    string s;
    int x,y,
        cnt = 0;
    while (fin>>s){    //找出所有2阶频繁项集
        if (s == "(2)"){
            fin>>x;
            freq[cnt][0] = x;
            getline(fin,s);
            fin>>y;
            freq[cnt][1] = y;
            getline(fin,s);
            cnt++;
        }
    }

    int num, tot;
    for (int i = 0; i < TRAN; i++){    //读入所有事务及包含的项
    	fp2>>num;
    	transn[i] = num;
        for (int j = 0; j < num; j++)
        	fp2>>trans[i][j];
    }

    for (int i = 0; i < NODE; i++){    //读入姓名列表
    	getline(fp,namelist[i]);
    }

    for (int i = 0; i < FREQN; i++){    //对于所有2阶频繁项集{A,B}
        int nA = 0,    //A的支持度   
            nB = 0,    //B的支持度
            nAB = 0;    //{A,B}的支持度
        bool fA,fB;
        for (int j = 0; j < TRAN; j++){    //计算上述3个支持度
        	fA = 0;
        	fB = 0;
        	if (Found(freq[i][0],j)){
        		fA = 1;
                nA++;
        	}
            if (Found(freq[i][1],j)){
        		fB = 1;
                nB++;
        	}
        	if (fA && fB) nAB++;
        }
        if ((float)nAB/nA > MINCONF){    //如果A的置信度达到阈值，发现关联规则 A===>B
        	fout<<namelist[freq[i][0]]<<" ===> "<<namelist[freq[i][1]];
            if ((float)nA/nB < RATIO){    //如果支持度之比也达到阈值，发现可能的导师-学生关系(B师A生)
                fout<<"  (Student-Advisor)";
                totSA++;
            }
            fout<<endl;
            totAR++;
        }
        if ((float)nAB/nB > MINCONF){    //如果B的置信度达到阈值，发现关联规则 B===>A
        	fout<<namelist[freq[i][1]]<<" ===> "<<namelist[freq[i][0]];
            if ((float)nB/nA < RATIO){    //如果支持度之比也达到阈值，发现可能的导师-学生关系(A师B生)
                fout<<"  (Student-Advisor)";
                totSA++;
            }
            fout<<endl;
            totAR++;
        }
    }

    fout<<endl<<"Association Rules: "<<totAR<<endl
        <<"Student-Advisor Relationships: "<<totSA<<endl;    //输出关联规则总数和导师-学生关系总数

    fin.close();
    fout.close();
    fp.close();
    fp2.close();
    return 0;
}
