/*
Data Warehousing and Data Mining
Project：数据挖掘经典算法的实现——以DBLP数据集为例
Author：张彧
StudentID：1300012730
Date：2016.6 
*/

/*
InfluMax.cpp 影响力最大化分析部分
*/
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <queue>
#include <time.h>
using namespace std;

#define NODE 23872    //网络的顶点数
#define EDGE 85028    //网络的边数
#define MAXN 24000
#define MAXM 180000
#define TOPK 10
#define SNAP 100    //计算目标函数所用的snapshot数量

struct edge    //邻接表，用于存储合作网络
{
	int v,next;    //v表示这条边终点，next表示下一条相同起点的边的标号
};

edge E[MAXM];

int firstedge[MAXN] = {0},     //邻接表的表头，表示以该顶点为起点的第1条边的标号
    deg[MAXN] = {0},    //合作网络中每个点的出度
    seed[TOPK] = {0},    //选出的边际影响力值前TOPK的顶点，即种子结点
    nb[NODE] = {0};    //每个顶点已经被影响的顶点个数

bool visit[MAXN] = {0},    //Simulate()的BFS中记录是否顶点是否已被影响
     cur[MAXN] = {0};    //利用优先队列找边际影响力最大的点时，每个点的值是不是本轮刚刚计算出来的

float delta[MAXN][SNAP] = {0};    //SNAP个随机生成的snapshot中，每个点的阈值
float marg[NODE] = {0};    //当前轮次中每个点的边际影响力

struct myless
{
    bool operator()(int x, int y){
	    return marg[x] < marg[y];
    }
};
priority_queue <int, vector<int>, myless> PQ;    //Lazy Forward策略中用到的优先队列

string namelist[MAXN];

//构建SNAP个snapshot
void GenerateThreshold()
{
    for (int i = 0; i < NODE; i++)
	    for (int j = 0; j < SNAP; j++)
            delta[i][j] = (float)rand()/RAND_MAX;    //根据线性阈值模型，每个点的阈值是[0,1]的均匀分布
}

//模拟线性阈值模型的传播过程
int Simulate(int snapno, int topk)
{
    queue <int> Q;    //BFS用的队列
	int x,y,i,
	    tot = 0;
	float thrs;
	
	memset(visit,0,sizeof(visit));
	memset(nb,0,sizeof(nb));
	for (i = 0; i < topk; i++){
	    Q.push(seed[i]);    //所有种子节点已经被影响
	    visit[seed[i]] = 1;
	    tot++;
	}

    //利用BFS找出所有被影响的顶点
	while (!Q.empty()){
		x = Q.front();
		Q.pop();
		for (i = firstedge[x]; i != 0; i = E[i].next){
			y = E[i].v;
			nb[y]++;
			thrs = (float)nb[y]/deg[y];
			if (thrs >= delta[y][snapno] && !visit[y]){
				Q.push(y);   //如果当前顶点(被影响的邻居个数)/(总邻居个数)超过阈值，他也被影响
				visit[y] = 1;
				tot++;
			}
		}
	}
	return tot;
}

int main()
{
	ifstream fin("Network.txt",ios::in);    //输入：作者们的合作网络
    ofstream fout("InfluMax.txt",ios::out);    //输出：影响力最大化分析的结果
    ifstream fp("Namelist.txt",ios::in);    //输入：作者的姓名列表

    fout<<setiosflags(ios::fixed)<<setprecision(2);

	int x,y,conf,year,
	    tot = 0;
    
    srand(time(NULL));
    time_t start,end;
	start = clock();

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
	}

	for (int i = 0; i < NODE; i++){    //读入姓名列表
    	getline(fp,namelist[i]);
    }
	
	GenerateThreshold();		
	for (int i = 0; i < NODE; i++){
	    marg[i] = NODE+1;
	    PQ.push(i);
	}
		
    float maxd,
          totnum = 0;
	int maxj;

	/*找出边际影响力值前TOPK的顶点，共进行k轮，每轮找出边际影响力值最高的点
	  此贪心算法的近似比不会低于(1-1/e)=0.63 (Kempe et al. KDD'03)
	  利用Lazy Forward策略，用优先队列轮转可以很大程度上加速 (Leskovec et al. KDD'07)
	  利用snapshot近似随机，可以更快地计算出目标函数 (Cheng et al. CIKM'13) */
	for (int i = 1; i <= TOPK; i++){
	    memset(cur,0,sizeof(cur));
	    while (1){
	    	maxj = PQ.top();    //优先对列队首的元素maxj
	    	PQ.pop();
	    	maxd = marg[maxj];
	    	if (cur[maxj]){    //如果maxj的边际影响力值是本轮刚刚计算出来的，它一定是所有顶点中最大的
	    		seed[i-1] = maxj;
		        fout<<i<<' '<<namelist[maxj]<<'\t'<<maxd<<endl;    //输出顶点的姓名和PageRank值
		        break;
	    	}
	    	else {
	    		seed[i-1] = maxj;
		        totnum = 0;
		        for (int u = 1; u <= SNAP; u++){
		            totnum += Simulate(u,i)-Simulate(u,i-1);
	            }
	            marg[maxj] = totnum/SNAP;    //如果maxj的边际影响力值不是本轮刚刚计算出来的，重新计算
	            cur[maxj] = 1;
	            PQ.push(maxj);    //重新压回单调队列
	    	}
	    }
	} 
    
    end = clock();
    fout<<endl<<double(end-start)/CLOCKS_PER_SEC<<'s'<<endl;    //输出运行时间

    fin.close();
    fout.close();
    fp.close();
	return 0;
}
