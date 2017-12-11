#pragma once
#include "GraphPathfinding.h"

/*
 *	迪杰斯特拉单源最短路径算法
*/
class Dijkstra : public GraphPathfinding
{
public:
	Dijkstra();
	~Dijkstra();

public:
	void Execute(const Graph& Graph, const string& VetexId) override;

private:
	// 抽出最小路径估值的顶点
	Vertex* ExtractMin(vector< Vertex* >& Q);

	// 松弛
	void Relax(Vertex* v1, Vertex* v2, int Weight);

public:
	Result& GetResult() { return m_Ret; }

private:
	Result m_Ret;
};