/*
Data Warehousing and Data Mining
Project：数据挖掘经典算法的实现——以DBLP数据集为例
Author：张彧
StudentID：1300012730
Date：2016.6 
*/

/*
FreqItem (FPGrowth).cpp 频繁模式挖掘部分(FPGrowth算法)
*/
#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <algorithm>
#include <map>
using namespace std;

#define NODE 23782    //项的个数
#define TRAN 22998    //事务的个数
#define MAXN 50000
#define MINSUP 5    //最小支持度
#define STACK 100000    //栈空间的大小

ifstream fin("Translist.txt",ios::in);    //输入：事务列表
ofstream fout("FreqItem (FPGrowth).txt",ios::out);    //输出：挖掘出的所有频繁项集
ifstream fp("Namelist.txt",ios::out);    //输入：作者的姓名列表

//FP树节点，采用左子-右兄存储方法
struct TreeNode{
	int id,cnt;    //id表示节点对应的项，cnt表示节点在FP树上的计数
	TreeNode *father,*next,*leftchild,*sibling;    //father，leftchild，sibling分别表示父，左子，右兄
                                                   //next指向FP树中表示同一个项的下一个节点
    void clear()    //节点初始化
	{
		id = cnt = 0;
		father = next = leftchild = sibling = NULL;
	}
	TreeNode* down(int x, map<int,TreeNode*> &first);    //在FP树中向“下”走一步，用于将事务转化为FP树上的路径
                                                         //first表示每个项在FP树中第1次出现的位置
};

TreeNode buf[STACK];    //直接申请一段固定大小的空间存放FP树(防止malloc之后忘记free...)
int TreeNodenum = 0;    //FP树当前的节点个数，也相当于标识当前buf空间用到了哪里

//从当前节点向“下”走一步，使得子结点为x
TreeNode* TreeNode::down(int x, map<int,TreeNode*> &first)
{
	for (TreeNode* next = leftchild; next != NULL; next = next->sibling)
	    if (next->id == x)    //当前节点已有子节点x，直接向下走 
	    	return next;

    //当前节点无子节点x，新建一个节点，作为当前节点的新左儿子
    TreeNode* tmp = buf+TreeNodenum;
    TreeNodenum++;
    tmp->father = this;
    tmp->id = x;
    tmp->sibling = leftchild;    //原来的左子变为右兄(第2个儿子)
    tmp->cnt = 0;
    tmp->next = first[x];     //原来第1个出现的x变为next(第2个出现)
    tmp->leftchild = NULL;
    first[x] = tmp;    //新节点成为第1个出现的x
    leftchild = tmp;    //新节点成为左子
    return tmp;
}

//1阶频繁项集的表，按支持度排序
struct itemTable{
    int sup,id;    //sup表示支持度，id表示项
};

itemTable IT[NODE];

//FP树
struct FPTree{
	TreeNode* root;    //根结点
	map<int,TreeNode*> first;    //每个(1阶频繁)项在FP树中第1次出现的位置    
};

FPTree FPT;

int transn[TRAN] = {0},    //每个事务所含项的个数
    trans[TRAN][45] = {0},    //每个事务含的所有项(用变长数组vector写可能更好)
    permute[NODE] = {0},    //每个(1阶频繁)项在itemTable中的位置
    item[45] = {0},    //Search()中搜索到的频繁项集
    path[45] = {0};    //Search()中自底向上搜索到的路径   

string namelist[NODE];

//用于将所有项按支持度排序
bool myless(itemTable x, itemTable y)
{
	return x.sup > y.sup;
}

//用于将每个事务中的项按itemTable中的位置排序
bool myless2(int x, int y)
{
	return permute[x] < permute[y];
}

//判断项x是否在事务y中
bool Found(int x, int y)
{
	for (int i = 0; i < transn[y]; i++)
		if (trans[y][i] == x) 
			return 1;
	return 0;
}

//建立FP树
void Maketree()
{
	FPT.root = buf+TreeNodenum;    //新建根节点
	TreeNodenum++;
	FPT.root->clear();
	for (int i = 0; i < TRAN; i++){    //扫描所有事务
		TreeNode* cur = FPT.root;
		for (int j = 0; j < transn[i]; j++){    //按排好的新顺序扫描事务中的项
			cur = cur->down(trans[i][j],FPT.first);    //根据项建树，向“下”走一步
			cur->cnt++;
		}
	}
}

//搜索FP树中的频繁项集，PrevTree表示当前的FP树，depth表示递归的深度
void Search(int x, FPTree* PrevTree, int depth)
{
	int y = IT[x].id,    //当前排在itemTable最后的节点
	    total = 0;
	for (TreeNode* tmp = PrevTree->first[y]; tmp != NULL; tmp = tmp->next)
		total += tmp->cnt;    //将FP树中所有y的支持度相加
	if (total < MINSUP)    //支持度不够，不是频繁项集
		return;

	item[depth] = y;    //支持度足够，找到一个新的频繁项集

    fout<<'('<<depth+1<<") ";     //输出新的频繁项集
    for (int i = 0; i <= depth; i++)
    	fout<<item[i]<<' '<<namelist[item[i]]<<"\n    ";
    fout<<endl;

    //用从y自底向上找到的所有路径(y的条件模式基)构造新的FP树
	int newTreeNodenum = TreeNodenum;
	FPTree NewTree;
	NewTree.root = buf+TreeNodenum;    //新FP树的根  
	TreeNodenum++;
	NewTree.root->clear();

	for (TreeNode* tmp = PrevTree->first[y]; tmp != NULL; tmp = tmp->next){    //扫描FP树中所有的y
		int len = 0;
		for (TreeNode* cur = tmp; cur != PrevTree->root; cur = cur->father){    //从y自底向上找路径
			len++;
			path[len] = cur->id;
		}
		TreeNode* cur = NewTree.root;
        while (len > 0){
			cur = cur->down(path[len],NewTree.first);    //用这些路径构造新的FP树
			cur->cnt += tmp->cnt;    //计算新FP树上所有节点的支持度计数
			len--;
		}
	}

	for (int i = x-1; i >= 0; i--) 
		if (NewTree.first.find(IT[i].id) != NewTree.first.end())
		    Search(i,&NewTree,depth+1);    //在新的FP树上搜索

	TreeNodenum = newTreeNodenum;    //回溯
}

int main()
{
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

    int freqn = 0;
    for (int i = 0; i < NODE; i++){    //找出所有1阶频繁项
    	tot = 0;
    	for (int j = 0; j < TRAN; j++)
    		if (Found(i,j)) tot++;
    	if (tot >= MINSUP){
    		IT[freqn].sup = tot;
    		IT[freqn].id = i;
    		freqn++;
    	}
    }

    sort(IT,IT+freqn,myless);    //按照支持度对所有1阶频繁项排序
    for (int i = 0; i < freqn; i++)
    	permute[IT[i].id] = i;
    for (int i = 0; i < TRAN; i++)    //按照1阶频繁项的排序对每个事务中的项排序
    	sort(trans[i],trans[i]+transn[i],myless2);
  
    Maketree();    //建初始FP树

    for (int i = freqn-1; i >= 0; i--)
    	if (FPT.first.find(IT[i].id) != FPT.first.end())
    	    Search(i,&FPT,0);    //从排在itemTable最后的节点开始，在FP树上搜索

    end = clock();
    fout<<double(end-start)/CLOCKS_PER_SEC<<'s'<<endl;    //输出运行时间 

    fin.close();
    fout.close();
    fp.close();
    return 0;
}
