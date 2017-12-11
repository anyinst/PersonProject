#pragma once

#include "cocos2d.h"
USING_NS_CC;

#include <unordered_map>
using namespace std;

class RoleKaito : public Node
{
public:
	RoleKaito();
	~RoleKaito();

public:
	CREATE_FUNC(RoleKaito);
	bool init();

public:
	void CreateActions();

public:
	enum class State
	{
		None,
		WalkLeft,  // 向左走
		WalkRight, // 向右走
		WalkTop,   // 向上走
		WalkBottom // 向下走
	};

	void SetState(State type);

public:
	unordered_map<string, void*> UserData;
	Sprite *m_pSprMain;
	Vector<Action*> m_Actions;
	Size m_size;

	Action *m_pActWalkRight;
	Action *m_pActWalkLeft;
	Action *m_pActWalkTop;
	Action *m_pActWalkBottom;

	State m_State;
};