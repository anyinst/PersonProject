#pragma once

#include "Graph.h"
#include <unordered_map>
using namespace std;

struct Result
{
	unordered_map<Vertex*, Vertex*> PathTree;
};

class GraphPathfinding
{
public:
	GraphPathfinding();
	~GraphPathfinding();

public:
	virtual void Execute(const Graph &graph, const string &VertexId) = 0;
};