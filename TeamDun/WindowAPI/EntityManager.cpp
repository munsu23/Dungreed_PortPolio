#include "stdafx.h"
#include "EntityManager.h"

HRESULT EntityManager::init()
{
	for (int i = 0; i < _vBullets.size(); i++)
	{
		_vBullets[i]->init();
	}
	
	_wormVillage = new WormVillage(*dynamic_cast<WormVillage*>(DATAMANAGER->GetObjectById(2501)));

	return S_OK;
}

void EntityManager::update()
{
	for (int i = 0; i < _vObjs.size(); i++) _vObjs[i]->update();
	for (int i = 0; i < _vBullets.size(); i++) _vBullets[i]->update();
	eraseBullet();
	
	_wormVillage->update();
	_p->update();
	HitBullet();
}

void EntityManager::render(HDC hdc)
{
	_wormVillage->render(hdc);
	for (int i = 0; i < _vObjs.size(); i++) _vObjs[i]->render(hdc);
	for (int i = 0; i < _vBullets.size(); i++) _vBullets[i]->render(hdc);
	_p->render(hdc);
}

void EntityManager::release()
{
	for (int i = 0; i < _vObjs.size(); i++)	_vObjs[i]->release();
	for (int i = 0; i < _vBullets.size(); i++) _vBullets[i]->release();
	_wormVillage->release();
	_p->release();
}

Bullet* EntityManager::makeBullet(const char * imageName, string effectIgName, BULLETTYPE type, float x, float y, float angle,float damage, float speed, float maxDis, bool isFrame ,float igAngle)
{
	Bullet* _bullet = new Bullet;
	_bullet->makeBullet(imageName, effectIgName,type, x, y, angle,damage, speed, maxDis, isFrame , igAngle);
	_vBullets.push_back(_bullet);
	return _bullet;
}

void EntityManager::eraseBullet()
{

	image* batBulletIg = IMAGEMANAGER->findImage("BatBullet");
	image* BansheeBulletIg = IMAGEMANAGER->findImage("BansheeBulletIdle");



	for (int i = 0; i < _vBullets.size(); i++)
	{
		if (_vBullets[i]->getDis() >= _vBullets[i]->getMaxDis())
		{
			EFFECTMANAGER->AddEffect(_vBullets[i]->getX(), _vBullets[i]->getY(), _vBullets[i]->getEffectIgName(), 4, 0, 0, false, 255);
			_vBullets[i]->SetIsDead(true);
			
		}
	}
	
	//총알 레이어 1벽과 충돌체크
	for (int i = 0; i < _vBullets.size(); i++)
	{
		if (_vBullets[i]->getType() == BT_NOMAL|| _vBullets[i]->getType() == BT_PLAYER)
		{
			COLORREF color = GetFastPixel(MAPMANAGER->GetPixelGetter(), _vBullets[i]->getX(), _vBullets[i]->getY());
			int r = GetRValue(color);
			int g = GetGValue(color);
			int b = GetBValue(color);

			if ((r == 255 && g == 0 && b == 0))
			{
				
				EFFECTMANAGER->AddEffect(_vBullets[i]->getX(), _vBullets[i]->getY(), _vBullets[i]->getEffectIgName(), 4, 0, 0, false, 255);
				_vBullets[i]->SetIsDead(true);
				
			}
		}
	}
	


	for (int i = 0; i < _vBullets.size(); i++)
	{
		if (_vBullets[i]->getIsDead() == true)
		{
			_vBullets.erase(_vBullets.begin() + i);
		}
	}
}

void EntityManager::HitBullet()
{
	RECT temp;
	for (int i = 0; i < _vBullets.size(); i++)
	{
		if (_vBullets[i]->getType() == BT_NOCOL || _vBullets[i]->getType() == BT_NOMAL)
		{

			if (IntersectRect(&temp, &_p->GetBody(), &_vBullets[i]->getRc()))
			{
				_vBullets[i]->SetIsDead(true);

				if (_p->GetIsHit() == false)
				{
					float damage;
					float block;
					float evasion;
					
					damage = _vBullets[i]->getDamage() * _p->GetRealDefence()/100;
					evasion = RANDOM->range(100);
					block = RANDOM->range(100);
					if (_p->GetRealEvasion() <= evasion)
					{
						if (_p->GetBlock() <= block)
						{
							_p->SetIsHit(true);
							_p->SetHitCount(0);
							_p->SetHp(_p->GetHP() - damage);
						}
					}
				}
			}
		}
	}
}
