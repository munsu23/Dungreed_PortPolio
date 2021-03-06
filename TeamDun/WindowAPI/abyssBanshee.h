#pragma once
#include "enemy.h"
class AbyssBanshee : public Enemy
{
private:
	int		_stateTimer;
	int		_rndAngle;
	bool	_isAtk;
	bool	_leftAtk;
	float	_probeBottom;

public:
	virtual HRESULT init(int id, string name, OBJECTTYPE type, vector<string> imgNames);
	virtual void	update();
	virtual	void	release();
	virtual void	render(HDC hdc);

	virtual void	Attack();
	void CheckNewPos();
	virtual void	Animation();

};

