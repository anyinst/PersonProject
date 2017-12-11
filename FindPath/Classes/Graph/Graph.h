#pragma once
#include <vector>
// map是按照operator<比较判断元素是否相同，以及比较元素的大小，然后选择合适的位置插入到树中。
// unordered_map是计算元素的Hash值，根据Hash值判断元素是否相同。
#include <unordered_map>
using namespace std;

class Edge;
class Vertex;
class Graph;
class GraphPathfinding;

/*
    图顶点
*/

class Vertex
{
	friend class Graph;
	friend class GraphPathfinding;
	friend class Dijkstra;

public:
	Vertex(const string& Name)
	{
		m_strId = Name;
		m_Cost = 0;
		m_pGraph = 0;
	}

	~Vertex(){}

public:
	// 附加属性
	unordered_map<string, void*> UserData;

public:
	const unordered_map<string, Edge*> & GetEdgesOut() const
	{
		return m_EdgesOut;
	}

	const unordered_map<string, Edge*> & GetEdgesIn() const
	{
		return m_EdgesIn;
	}

	const string & GetId() const
	{
		return m_strId;
	}

	const string & GetText() const
	{
		return m_Text;
	}

	void SetText(const string & Text)
	{
		m_Text = Text;
	}

	Graph * GetGraph()
	{
		return m_pGraph;
	}

public:
	// AStar寻路算法需要的数据
	struct Pathfinding
	{
		// 顶点的前驱顶点
		Vertex *pParent;
		// 路径代价估计  
		int Cost;
		// 标识符
		int Flag;
		// 启发式函数的计算出来的值
		int Heuristic;

		Pathfinding()
		{
			pParent = 0;
			Cost = 0;
			Flag = 0;
			Heuristic = 0;
		}
	}PathfindingData;

protected:
	// 出度集合
	unordered_map<string, Edge*> m_EdgesOut;

	// 入度集合
	unordered_map<string, Edge*> m_EdgesIn;

	// 节点表示的字符串
	string m_Text;

	// 节点的ID
	string m_strId;

	// 用于寻路算法。路径代价估计
	int m_Cost;

	// 所属的图
	Graph *m_pGraph;
};

/*
	图顶点的边
	有向边
*/
class Edge
{
	friend class Graph;

public:
	Edge()
	{
		m_Weight = 0;
		m_pStartVertex = m_pEndVertex = 0;
	}

	Edge(Vertex* pStartVertex, Vertex* pEndVertex, int Weight = 0)
	{
		m_Weight = Weight;

		m_pStartVertex = pStartVertex;
		m_pEndVertex = pEndVertex;
	}

public:
	int GetWeight() const 
	{ 
		return m_Weight; 
	}

	void SetWeight(int var)
	{
		m_Weight = var;
	}

	Vertex* GetStartVertex() const 
	{ 
		return m_pStartVertex; 
	}

	Vertex* GetEndVertex() const 
	{ 
		return m_pEndVertex; 
	}

protected:
	// 边的权值
	int m_Weight;

	// 起点的顶点
	Vertex *m_pStartVertex;

	// 终点的顶点
	Vertex *m_pEndVertex;
};

/*
    图.
    图会负责释放顶点和边的内存
*/
class Graph
{
public:
	Graph();
	~Graph();

public:
	// 添加一个顶点
	void AddVertex(Vertex* pV);

	// 删除一个顶点
	void DeleleVertex(const string& VertexName);

	// 添加一条边。返回边对象
	Edge* AddEdge(const string& Vertex1Name, const string& Vertex2Name, int Weight = 0);

	// 删除一条边
	void DeleteEdge(const string& StartVertexName, const string& EndVertexName);

public:
	const unordered_map<string, Vertex *> & GetVertes() const
	{
		return m_Vertexes;
	}

protected:
	// 顶点的集合
	unordered_map<string, Vertex*> m_Vertexes;

	// 边的集合。Key的格式“顶点1name->顶点2name"
	unordered_map<string, Edge*> m_Edges;

protected:
	#define GetEdgeKey( pV1 , pV2 )(  pV1->m_strId + "->" + pV2->m_strId );
};