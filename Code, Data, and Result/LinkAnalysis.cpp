/*
Data Warehousing and Data Mining
Project：数据挖掘经典算法的实现——以DBLP数据集为例
Author：张彧
StudentID：1300012730
Date：2016.6 
*/

/*
LinkAnalysis.cpp 链接分析部分(包括PageRank、SimRank和Topic-Sensitive PageRank)
*/
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <cmath>
using namespace std;

#define NODE 23782    //网络的顶点数
#define EDGE 85028    //网络的边数
#define MAXN 24000
#define MAXM 180000
#define TOPK 10
#define CONFN 9

struct edge    //邻接表，用于存储合作网络
{
	int v,next;    //v表示这条边终点，next表示下一条相同起点的边的标号
};

edge E[MAXM];

int firstedge[MAXN] = {0},     //邻接表的表头，表示以该顶点为起点的第1条边的标号
    deg[MAXN] = {0},    //合作网络中每个点的出度
    topicnum,
    deg1[MAXN] = {0},    //会议-作者二部图中每个作者的度数(SimRank中使用)
    deg2[CONFN] = {0};    //会议-作者二部图中每个会议的度数(SimRank中使用)

double pr[MAXN] = {0},    //顶点的PageRank值
       pr2[MAXN] = {0},    //迭代过程中新的PageRank值，用于和pr比较判断收敛
       deltap = 1e-8,    //收敛误差
       cc = 0.8,    //PageRank的蒸发系数
       prconf[CONFN] = {0},    //会议的PageRank值(SimRank中使用)
       prconf2[CONFN] = {0};    //迭代过程中新的PageRank值(SimRank中使用)，用于和prconf比较判断收敛

bool topic[MAXN] = {0},    //每个顶点是否在需要讨论的领域中(Topic-Sensitive PageRank中使用)
     mat[CONFN][MAXN] = {0};    //会议-作者二部图

string namelist[MAXN],
       confname[CONFN] = {"KDD", "ICDM", "SDM", "WSDM", "PKDD", "ICML", "NIPS", "COLT", "ECML"};

//PageRank，找出整个合作网络中重要的顶点
void PageRank()
{
	int x,tf;
	while (1){
		for (int i = 0; i < NODE; i++){
		    pr2[i] = (1-cc)/NODE;    //(1-cc)的PageRank值均分给每个顶点
		}
		for (int i = 0; i < NODE; i++)
		    for (int j = firstedge[i]; j != 0; j = E[j].next){    //扫描邻接表中以i为起点的每一条边
			    x = E[j].v;
			    pr2[x] += cc*pr[i]/deg[i];    //i把自己的PageRank值分给邻居
		    }
		tf = 0;
		for (int i = 0; i < NODE; i++)
		    if (abs(pr[i]-pr2[i]) > deltap){    //判断PageRank值是否收敛
		    	tf = 1;
		    	break;
		    }
		if (!tf) break;
		for (int i = 0; i < NODE; i++) 
			pr[i] = pr2[i];
	}
}

//Topic-Sensitive PageRank，找出1个特定领域(例如Data Mining，Machine Learning，Learning Theory等)中的重要顶点
void TopicSensitivePageRank()
{
	int x,tf;
	while (1){
		for (int i = 0; i < NODE; i++){
		    if (topic[i])
		        pr2[i] = (1-cc)/topicnum;    //(1-cc)的PageRank值均分给每个领域中的顶点
	        else 
	        	pr2[i] = 0;
	    }
		for (int i = 0; i < NODE; i++)
		    for (int j = firstedge[i]; j != 0; j = E[j].next){    //扫描邻接表中以i为起点的每一条边
			    x = E[j].v;
			    pr2[x] += cc*pr[i]/deg[i];    //i把自己的PageRank值分给邻居
		    }
		tf = 0;
		for (int i = 0; i < NODE; i++)
		    if (abs(pr[i]-pr2[i]) > deltap){    //判断PageRank值是否收敛
		    	tf = 1;
		    	break;
		    }
		if (!tf) break;
		for (int i = 0; i < NODE; i++) 
			pr[i] = pr2[i];
	}
}

//SimRank，衡量会议-作者二部图中各个会议与会议v的相似度
void SimRank(int v)
{
	int tf;
	while (1){
		for (int i = 0; i < NODE; i++)
		    pr2[i] = 0;
		for (int i = 0; i < CONFN; i++)
		    prconf2[i] = 0;
		prconf2[v] = 1-cc;    //(1-cc)的PageRank值集中到会议节点v
		for (int i = 0; i < CONFN; i++)
		    for (int j = 0; j < NODE; j++)
		        if (mat[i][j]){    //扫描会议-作者二部图mat中的每一条边
			        pr2[j] += cc*prconf[i]/deg2[i];    //作者节点j把自己的PageRank值分给会议节点邻居
			        prconf2[i] += cc*pr[j]/deg1[j];    //会议节点i把自己的PageRank值分给作者节点邻居
		        }
		tf = 0;
		for (int i = 0; i < NODE; i++)
		    if (abs(pr[i]-pr2[i]) > deltap){    //判断PageRank值是否收敛
		    	tf = 1;
		    	break;
		    }
		for (int i = 0; i < CONFN; i++)
		    if (abs(prconf[i]-prconf2[i]) > deltap){
		    	tf = 1;
		    	break;
		    }
		if (!tf) break;
		for (int i = 0; i < NODE; i++) 
			pr[i] = pr2[i];
		for (int i = 0; i < CONFN; i++) 
			prconf[i] = prconf2[i];
	}
}

int main()
{
	ifstream fin("Network.txt",ios::in);    //输入：作者们的合作网络
    ofstream fout("LinkAnalysis.txt",ios::out);    //输出：链接分析的结果
    ifstream fp("Namelist.txt",ios::in);    //输入：作者的姓名列表

	int x,y,conf,year,
	    tot = 0;
	   
	for (int i = 0; i < EDGE; i++){
	    fin>>x>>y>>conf>>year;
	    //根据边的的信息建立邻接表
	    tot++; 
		E[tot].v = y;
		E[tot].next = firstedge[x];    //将这条边插入到x的边链表的表头
		firstedge[x] = tot;
		deg[x]++;
		tot++;
		E[tot].v = x;
		E[tot].next = firstedge[y];    //将这条边插入到y的边链表的表头
		firstedge[y] = tot;
		deg[y]++;

		//if (year >= 2011){    //只考虑近5年的论文
	    //if (conf <= 4){    //只考虑Data Mining领域的论文
	    if (conf >= 5){    //只考虑Machine Learning领域的论文
		//if (conf == 7){    //只考虑COLT会议(Learning Theory领域)的论文
			topic[x] = 1;    //标记该作者是否属于这一领域
			topic[y] = 1;
		}

		mat[conf][x] = 1;    //建立会议-作者二部图
		mat[conf][y] = 1;
	}

    for (int i = 0; i < NODE; i++){    //读入姓名列表
    	getline(fp,namelist[i]);
    }
	
	fout<<"PageRank:\n";
	for (int i = 0; i < NODE; i++)
		pr[i] = (double)1/NODE;    //初始所有顶点PageRank值均分
	PageRank();
	
	int maxj;
	double maxd = 0;
	for (int i = 1; i <= TOPK; i++){    //找出PageRank值前TOPK的顶点
		maxd = 0; 
		for (int j = 0; j < NODE; j++)
		    if (pr[j] > maxd){
		        maxd = pr[j];
		        maxj = j;
		    }
		pr[maxj] = 0;
		fout<<i<<'\t'<<namelist[maxj]<<'\t'<<maxd<<endl;     //输出这些顶点的姓名和PageRank值
	}

    fout<<"\nTopic-Sensitive PageRank:\n";
	for (int i = 0; i < NODE; i++) 
		pr[i] = (double)1/NODE;    //初始所有顶点PageRank值均分
	for (int i = 0; i < NODE; i++)
		if (topic[i]) topicnum++;
	TopicSensitivePageRank();

	for (int i = 1; i <= TOPK; i++){    //找出Topic-Sensitive PageRank值前TOPK的顶点
		maxd = 0; 
		for (int j = 0; j < NODE; j++)
		    if (pr[j] > maxd && topic[j]){
		        maxd = pr[j];
		        maxj = j;
		    }
		pr[maxj] = 0;
		fout<<i<<'\t'<<namelist[maxj]<<'\t'<<maxd<<endl;     //输出这些顶点的姓名和PageRank值
	}

	fout<<"\nSimRank:\n";
	for (int i = 0; i < CONFN; i++)
		for (int j = 0; j < NODE; j++)
			if (mat[i][j]){    //计算会议-作者二部图中定点的度数
				deg1[j]++;
				deg2[i]++;
			}
	for (int i = 0; i < NODE; i++) 
		pr[i] = (double)1/(NODE+CONFN);    //初始所有顶点PageRank值均分    
	for (int i = 0; i < CONFN; i++)
		prconf[i] = (double)1/(NODE+CONFN);
	SimRank(0);    //计算所有会议与KDD(0号会议)的SimRank值，即相似度

	for (int i = 1; i <= CONFN; i++){
		maxd = 0; 
		for (int j = 0; j < CONFN; j++)
		    if (prconf[j] > maxd){
		        maxd = prconf[j];
		        maxj = j;
		    }
		prconf[maxj] = 0;
		fout<<i<<'\t'<<confname[maxj]<<'\t'<<maxd<<endl;     //输出所有会议与KDD的相似度
	}

    fin.close();
    fout.close();
    fp.close();
	return 0;
}
