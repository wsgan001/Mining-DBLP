# -*- coding:utf-8 -*-

# Data Warehousing and Data Mining
# Project：数据挖掘经典算法的实现——以DBLP数据集为例
# Author：张彧
# StudentID：1300012730
# Date：2016.10 

# BigCLAM.py 社区发现部分(本部分是2016.10修改时新增的算法，并未出现在实验报告中)

from __future__ import division
import numpy as np
import scipy as sp
import networkx as nx
import sys
old = sys.stdout

class BigCLAM(object):

	def __init__(self, A = None, K = None):
		np.random.seed(1125582)
		self.A = A.copy()
		self.K = K
		self.N = self.A.shape[0]
		self.not_A = 1.0 * (self.A == 0)
		np.fill_diagonal(self.not_A, 0)
		self.sparsity_coef = 0
		self.initFmode = 'Rand'
		self.eps = 1e-4
		self.epsCommForce = 1e-8
		self.stepSizeMod = 'Backtracking_Line_Search'
		self.max_iter = 1000000
		self.last_step = None
		self.lF = [[], []]
		self.noImprCount = 0

	def initF_Rand(self):
		F = 0.75 + 0.5 * np.random.rand(self.N, self.K)
		return F

	def initF(self):
		if self.initFmode == 'Rand':
			F = self.initF_Rand() 
		# elif self.initFmode == 'LocMinNeighborhood':	#原文中提到了Local Minimal Neighborhood方法，还未实现
		# 	F = self.initF_LMN()

		self.maxlF = self.loglikelihood(F)
		self.lF[0].append(0)
		self.lF[1].append(self.maxlF)
		self.maxF = F.copy()
		self.sumF = np.sum(F, axis = 0)
		return F

	def calc_penalty(self, F, u=None, newFu=None, real=False):
		if newFu is not None:
			Fu = newFu[:, None]
		else:
			Fu = F[u, None]

		pen = 0
		if self.sparsity_coef != 0 or real:
			if u is None:
				pen = F.T.dot(F)
				np.fill_diagonal(pen, 0)
				pen = np.sum(pen)
			else:
				pen = Fu.T.dot(Fu)
				np.fill_diagonal(pen, 0)
				pen = np.sum(pen)
		return pen

	def calc_penalty_grad(self, F, u):
		pen_grad = 0
		if self.sparsity_coef != 0:
			pen_grad = np.sum(F[u]) - F[u]
		return pen_grad

	def loglikelihood_u(self, F, u=None, newFu=None):
		if newFu is not None:
			Fu = newFu
 			sumF = self.sumF - F[u] + newFu
		else:
			Fu = F[u]
			sumF = self.sumF

		indx = self.A[u, :] != 0
		neigF = F[indx]
		pen = self.calc_penalty(F, u, newFu)
		S1 = np.sum(np.log(1 - np.exp(-Fu.dot(neigF.T) - self.epsCommForce)))
		S2 = - Fu.dot((sumF - Fu - np.sum(neigF, axis=0).T))

		return S1 + S2 - self.sparsity_coef * pen

	def loglikelihood(self, F):
		FF = F.dot(F.T)
		P = np.log(1 - np.exp(-FF - self.epsCommForce))
		pen = self.calc_penalty(F)
 		lF = np.sum(P * self.A) - np.sum(self.not_A * FF)
		return lF - self.sparsity_coef * pen

	def gradient(self, F, u):
		indx = np.where(self.A[u, :] != 0)
		neigF = F[indx] / self.A[u, indx].T
 		PP = 1 / (1 - np.exp(-F[u].dot(neigF.T) - self.epsCommForce)) - 1
		pen_grad = self.calc_penalty_grad(F, u)

		grad = np.sum(neigF * PP[:, None], axis=0) - (self.sumF - F[u] - np.sum(neigF, axis=0)) - 2 * self.sparsity_coef * pen_grad

		return grad

	def stop(self, F, iter):
		newlF = self.loglikelihood(F)
		if newlF > self.maxlF:
			self.maxlF = newlF
			self.noImprCount = 0
			self.maxF = F.copy()
		else:
			self.noImprCount += 1
		self.lF[0].append(iter)
		self.lF[1].append(newlF)
		if len(self.lF[0]) <= 1:
			return False
		elif (abs(self.lF[1][-1] / self.lF[1][-2] - 1) < self.eps) or (self.noImprCount > 3) or (iter > self.max_iter):
		    return True
		else:
			return False

	def nextNodeToOptimize(self, F):
		iter = 0
		while True:
			order = np.random.permutation(self.A.shape[0])
			for i in order:
				iter += 1
				yield iter, i

	def optimize(self, F, u, iter=1, step=None):
		grad = self.gradient(F, u)
		m = max(np.abs(grad))
		if m > 100:
			grad = grad * 100.0 / m
		step = self.stepSize(u, F, grad, grad, iter)
		if step != 0.0:
			newFu = self.step(F[u], step, grad)
			self.sumF = self.sumF - F[u] + newFu
			F[u] = newFu
		return F

	def step(self, Fu, stepSize, direction):
		return np.minimum(np.maximum(0, Fu + stepSize * direction), 10000)

	def stepSize(self, u, F, deltaV, gradV, iter, alpha=0.1, beta=0.3, MaxIter=15):
		if self.stepSizeMod == 'Backtracking_Line_Search':
			return self.backtrackingLineSearch(u, F, deltaV, gradV, alpha, beta, MaxIter)
		else:
			return 0.01 / iter ** 0.25

	def backtrackingLineSearch(self, u, F, deltaV, gradV, alpha=0.1, beta=0.3, MaxIter=15):
		stepSize = 0.1
		stepSize = stepSize if self.last_step == None else min(self.last_step / beta, stepSize)
		lF = self.loglikelihood_u(F, u)
		for i in xrange(MaxIter):
			D = self.step(F[u], stepSize, deltaV)
			newlF = self.loglikelihood_u(F, u, newFu=D)
			update = alpha * stepSize * gradV.dot(deltaV)
			if newlF < lF + update or np.isnan(newlF):
				stepSize *= beta
			else:
				self.last_step = stepSize
				break
		else:
			stepSize = 0
		return stepSize

	def communityDetection(self):
		F = self.initF()

		for iter, u in self.nextNodeToOptimize(F):
			F = self.optimize(F, u, iter)
			if iter % self.N == 0:
				if self.stop(F, iter):
					print 'Iterations stopped!'
					break
				else:
					lF = self.loglikelihood(F)	#未终止，输出目标函数(对数似然)
					print 'Iteration: {}, Loglikelihood: {}'.format(iter, lF)

		return self.maxF, self.maxlF

if __name__ == "__main__":

	def splitStr(initStr, splitFlag = ' '):
		tmpList = initStr.split(splitFlag)
		tmpList = list(filter(lambda x : x != '', tmpList))
		return tmpList

	# A = np.array([[0, 1, 1, 0, 0, 0], [1, 0, 1, 0, 0, 0], [1, 1, 0, 1, 0, 0], [0, 0, 1, 0, 1, 1], [0, 0, 0, 1, 0, 1],
	# 			  [0, 0, 0, 1, 1, 0]])

	Namelist = []
	fin0 = open('Namelist.txt')
	for line in fin0:
		Namelist.append(line)
	fin0.close()

	PRToplist = []
	PRToplistInv = np.zeros(25000)
	fin1 = open('BigCLAMNodes.txt')	#Nodes with Top-500 PageRank Values
	cnt = 0
	for line in fin1:
		tmpList = splitStr(line, '\t')
		x = int(tmpList[0])
		PRToplist.append(x)
		PRToplistInv[x] = cnt
		cnt += 1
	fin1.close()

	G = nx.Graph()
	fin2 = open('Network.txt')
	for line in fin2:
		tmpList = splitStr(line)
		x = int(tmpList[0])
		y = int(tmpList[1])
		if (x in PRToplist) and (y in PRToplist):
			G.add_edge(PRToplistInv[x], PRToplistInv[y])
	for x in PRToplist:
		if not PRToplistInv[x] in G.nodes():
			G.add_node(PRToplistInv[x])
	fin2.close()

	A = np.array(nx.to_numpy_matrix(G))
	bc = BigCLAM(A, 4)
	[maxF, maxlF] = bc.communityDetection()

	fout = open('BigCLAM.txt', 'w')
	sys.stdout = fout
	for i in PRToplist:
		print Namelist[i], maxF[int(PRToplistInv[i])]

	fout.close()
	sys.stdout = old