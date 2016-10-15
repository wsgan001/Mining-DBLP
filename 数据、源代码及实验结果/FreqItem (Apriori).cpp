/*
Data Warehousing and Data Mining
Project：数据挖掘经典算法的实现——以DBLP数据集为例
Author：张彧
StudentID：1300012730
Date：2016.6 
*/

/*
FreqItem (Apriori).cpp 频繁模式挖掘部分(Apriori算法)
*/
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
using namespace std;

#define NODE 23782    //项的个数
#define TRAN 22998    //事务的个数
#define MAXN 50000
#define MINSUP 5    //最小支持度

int transn[TRAN] = {0},    //每个事务所含项的个数
    trans[TRAN][45] = {0},    //每个事务含的所有项(用变长数组vector写可能更好)
    freq[2][MAXN][10] = {0},    //频繁项集，由于Apriori计算k阶频繁项集时只需要k-1阶频繁项集，所以只保存前1阶的结果
    freqn[2] = {0};    //每一阶频繁项集的个数，同样只保存前1阶的结果

string namelist[NODE];

//判断项x是否在事务y中
bool Found(int x, int y)
{
	for (int i = 0; i < transn[y]; i++)
		if (trans[y][i] == x) 
			return 1;
	return 0;
}

//已知2个频繁项集，x代表{x_0,x_1,...,x_k-3,x_k-2}，y代表{x_0,x_1,...,x_k-3,y_k-2}
//判断项集{x_0,x_1,...,x_k-3,x_k-2,y_k-2}是否在事务z中
bool FoundSet(int ht, int x, int y, int z, int k)
{
	for (int i = 0; i < k-1; i++)
		if (!Found(freq[ht][x][i],z)) return 0; 
	if (!Found(freq[ht][y][k-2],z)) return 0; 
	return 1;
}

int main()
{
	ifstream fin("Translist.txt",ios::in);    //输入：事务列表
    ofstream fout("FreqItem (Apriori).txt",ios::out);    //输出：挖掘出的所有频繁项集
    ifstream fp("Namelist.txt",ios::out);    //输入：作者的姓名列表

    time_t start,end;
	start = clock();
    
    int num, tot;
    for (int i = 0; i < TRAN; i++){    //读入所有事务及包含的项
    	fin>>num;
    	transn[i] = num;
        for (int j = 0; j < num; j++)
        	fin>>trans[i][j];
    }

    for (int i = 0; i < NODE; i++){    //读入姓名列表
    	getline(fp,namelist[i]);
    }

    for (int i = 0; i < NODE; i++){    //找出所有1阶频繁项集
    	tot = 0;
    	for (int j = 0; j < TRAN; j++)
    		if (Found(i,j)) tot++;
    	if (tot >= MINSUP){
    		freq[1][freqn[1]][0] = i;
    		freqn[1]++;
    	}
    }
    for (int i = 0; i < freqn[1]; i++)
    	fout<<"(1) "<<freq[1][i][0]<<' '<<namelist[freq[1][i][0]]<<endl<<endl;    //输出所有1阶频繁项集
    
    int u,v,
        k = 2;
    //找出所有k阶频繁项集(k >= 2)
    while (1){
    	int ht = k%2;
    	freqn[ht] = 0;
    	for (int i = 0; i < freqn[1-ht]-1; i++)    //枚举所有(k-1)阶频繁项集
    	    for (int j = i+1; j < freqn[1-ht]; j++){
    	    	for (u = 0; u < k-2; u++)
    	    		if (freq[1-ht][i][u] != freq[1-ht][j][u]) break;
                //如果有2个(k-1)阶频繁项集{x_0,x_1,...,x_k-3,x_k-2}和{x_0,x_1,...,x_k-3,y_k-2}
                //并且x_k-2 < y_k-2，检验{x_0,x_1,...,x_k-3,x_k-2,y_k-2}是否频繁
    	    	if (u == k-2){
    	            tot = 0;
    	            for (v = 0; v < TRAN; v++)
    		            if (FoundSet(1-ht,i,j,v,k)) tot++;
    	            if (tot >= MINSUP){    //候选项集频繁，找到一个新的k阶频繁项集
    		            for (v = 0; v < k-1; v++) 
    		            	freq[ht][freqn[ht]][v] = freq[1-ht][i][v];
    		            freq[ht][freqn[ht]][k-1] = freq[1-ht][j][k-2]; 
    		            freqn[ht]++;
    		        }   
                }         
            }
        if (freqn[ht] == 0) break;    //不存在k阶频繁项集，当然也不存在更高阶的，算法结束

        for (int i = 0; i < freqn[ht]; i++){    //输出所有k阶频繁项集
            fout<<'('<<k<<") ";
        	for (int j = 0; j < k; j++)
    	        fout<<freq[ht][i][j]<<' '<<namelist[freq[ht][i][j]]<<"\n    ";
    	    fout<<endl;
    	}
    	k++;
    }

    end = clock();
    fout<<double(end-start)/CLOCKS_PER_SEC<<'s'<<endl;    //输出运行时间 

    fin.close();
    fout.close();
    fp.close();
    return 0;
}
