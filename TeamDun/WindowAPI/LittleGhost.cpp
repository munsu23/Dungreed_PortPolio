#include "stdafx.h"
#include "LittleGhost.h"

HRESULT LittleGhost::init(int id, string name, OBJECTTYPE type, vector<string> imgNames)
{
	Enemy::init(id, name, type, imgNames);
	return S_OK;
}

void LittleGhost::update()
{
	Enemy::update();

	if (_isSpawned)
	{
		this->Animation();
	}
}

void LittleGhost::release()
{
}

void LittleGhost::render(HDC hdc)
{
	if (_isSpawned)
	{
		Enemy::render(hdc);
	}
}

void LittleGhost::Move()
{
	Enemy::Move();
}

void LittleGhost::Attack()
{
	Enemy::Attack();
}

void LittleGhost::Animation()
{
}