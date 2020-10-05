#include "stdafx.h"
#include "FieldMap.h"

HRESULT FieldMap::init(string fileName)
{
	_fileName = fileName;
	_spawnTimer = 0;

	_nextMapIndex[0] = -1;
	_nextMapIndex[1] = -1;
	_nextMapIndex[2] = -1;
	_nextMapIndex[3] = -1;

	return S_OK;
}

/// <summary>
/// 사전에 맵을 로드하여 타일과 오브젝트를 배치해넣는다.
/// </summary>
void FieldMap::LoadMap()
{
	vector<vector<string>> stringData = CSVMANAGER->csvLoad("Data/MapData/" + _fileName + ".mapData");
	vector<vector<string>> stringData2 = CSVMANAGER->csvLoad("Data/MapData/" + _fileName + "2.mapData");

	if (stringData.size() == 0 || stringData2.size() == 0)
	{
		return;
	} // 해당 맵 불러오기에 실패한 경우

	_vMapData.clear();
	_backImageEtc = nullptr;
	_backImageMain = nullptr;
	
	for (int i = 0; i < stringData.size(); i++)
	{
		vector<Tile*> tileLine;
		for (int j = 0; j < stringData[i].size(); j++)
		{
			Tile* tile = new Tile();
			if (stringData[i][j] == "-1") tile->_img = nullptr;
			else
			{
				tile->_img = IMAGEMANAGER->findImage(stringData[i][j]);
				RECT rc;
				rc = RectMake(10 + j * 5, 10 + i * 5, 5, 5); // 한칸당 5픽셀이기 때문에 늘리고싶으면 이미지도 같이 바꿔야합니다
				_vMiniRc.push_back(rc);
			}
			if (stringData2[i][j] == "-1") tile->_img2 = nullptr;
			else tile->_img2 = IMAGEMANAGER->findImage(stringData2[i][j]);
			tile->_x = j * 48;
			tile->_y = i * 48;

			if (stringData[i][j] == "-1") tile->_collisionImage = nullptr;	// 타일이 없다면 충돌체도 없다.
			else tile->_collisionImage = DATAMANAGER->GetGridDataByName(stringData[i][j])->_colImage; // 충돌체를 적당한 이미지로 넣는다.

			tileLine.push_back(tile);
		}
		_vMapData.push_back(tileLine);
	}

	if (_stage == 1 || _stage == 2)
	{
		_backImageEtc = IMAGEMANAGER->findImage("BackFloorBack1");
		_backImageMain = IMAGEMANAGER->findImage("SubBGStage1");
	}
}

void FieldMap::LoadObject()
{
	_vObjs.clear();
	vector<vector<string>> objData = CSVMANAGER->csvLoad("Data/MapData/" + _fileName + "_Objs.mapData");
	for (int i = 0; i < objData.size(); i++)
	{
		Object* obj;
		// BEFORE OBJECT LOAD
		switch (stoi(objData[i][0]))
		{
		case 1500:// 큰 해골
			obj = new BigWhiteSkel(*dynamic_cast<BigWhiteSkel*>(DATAMANAGER->GetObjectById(stoi(objData[i][0]))));
			break;

		case 1501:// 미노타우르스
			obj = new Minotaurs(*dynamic_cast<Minotaurs*>(DATAMANAGER->GetObjectById(stoi(objData[i][0]))));
			break;

		case 702: // 붉은 박쥐
			obj = new RedBat(*dynamic_cast<RedBat*>(DATAMANAGER->GetObjectById(stoi(objData[i][0]))));
			break;

		case 2000: // 벨리알
			obj = new Belial(*dynamic_cast<Belial*>(DATAMANAGER->GetObjectById(stoi(objData[i][0]))));
			break;

		case 1505:// 작은 유령
			obj = new LittleGhost(*dynamic_cast<LittleGhost*>(DATAMANAGER->GetObjectById(stoi(objData[i][0]))));
			break;

		case 1507:// 해골
			obj = new Skel(*dynamic_cast<Skel*>(DATAMANAGER->GetObjectById(stoi(objData[i][0]))));
			break;

		case 514: // 문 왼쪽
			obj = new Door(*dynamic_cast<Door*>(DATAMANAGER->GetObjectById(stoi(objData[i][0]))));
			break;

		case 515: // 문 오른쪽
			obj = new Door(*dynamic_cast<Door*>(DATAMANAGER->GetObjectById(stoi(objData[i][0]))));
			break;

		case 516: // 문 위쪽
			obj = new Door(*dynamic_cast<Door*>(DATAMANAGER->GetObjectById(stoi(objData[i][0]))));
			break;

		case 517: // 문 아래쪽
			obj = new Door(*dynamic_cast<Door*>(DATAMANAGER->GetObjectById(stoi(objData[i][0]))));
			break;

		case 2500: // 몬스터 스포너
			obj = new MonsterSpawner(*dynamic_cast<MonsterSpawner*>(DATAMANAGER->GetObjectById(stoi(objData[i][0]))));
			break;

		default:
			obj = new Object(*DATAMANAGER->GetObjectById(stoi(objData[i][0])));
			break;
		}
		obj->SetX(stof(objData[i][1]) * 48);
		obj->SetY(stof(objData[i][2]) * 48);
		obj->SetSpawnTime(stoi(objData[i][3]));
		obj->SetBelongMap(this);
		obj->SetBodyPos();

		bool CheckRollBack = false;
		// AFTER OBJECT LOAD
		switch (stoi(objData[i][0]))
		{
		case 514: // 문 왼쪽
			if (_nextMapIndex[DIRECTION::DIR_LEFT] != -1) 
				MakeDoor(dynamic_cast<Door*>(obj));
			else CheckRollBack = true;
			break;
		case 515: // 문 오른쪽
			if (_nextMapIndex[DIRECTION::DIR_RIGHT] != -1) 
				MakeDoor(dynamic_cast<Door*>(obj));
			else CheckRollBack = true;
			break;
		case 516: // 문 위쪽
			if (_nextMapIndex[DIRECTION::DIR_UP] != -1) 
				MakeDoor(dynamic_cast<Door*>(obj));
			else CheckRollBack = true;
			break;
		case 517: // 문 아래쪽
			if (_nextMapIndex[DIRECTION::DIR_DOWN] != -1) 
				MakeDoor(dynamic_cast<Door*>(obj));
			else CheckRollBack = true;
			break;
		}

		_vObjs.push_back(obj);
		if (CheckRollBack)
		{
			_vObjs.erase(_vObjs.end() - 1);
		}
	}
}

/// <summary>
/// 문을 만들고 해당 문의 주변 타일을 처리한다 
/// </summary>
void FieldMap::MakeDoor(Door* door)
{
	
	int x = (door->GetX() + door->GetImage(0)->getWidth() / 2) / 48;	// 문의 중점 X
	int y = (door->GetY() + door->GetImage(0)->getHeight() / 2) / 48;	// 문의 중점 Y

	int pos[4] = { 0,0,0,0 };
	switch (door->GetDirection()) // 방향에 따라 변경할 타일의 범위가 달라진다.
	{
	case DIRECTION::DIR_LEFT:	pos[0] = -3;  pos[1] = 0, pos[2] = -2, pos[3] = 2; break;
	case DIRECTION::DIR_RIGHT:	pos[0] = 0;  pos[1] = 3, pos[2] = -2, pos[3] = 2; break;
	case DIRECTION::DIR_UP:		pos[0] = -2;  pos[1] = 1, pos[2] = -2, pos[3] = 0; break;
	case DIRECTION::DIR_DOWN:	pos[0] = -2;  pos[1] = 1, pos[2] = 0, pos[3] = 2; break;
	}

	for (int i = y + pos[2]; i <= y + pos[3]; i++)
	{
		if (i < 0 || i >= _vMapData.size()) continue;
		for (int j = x + pos[0]; j <= x + pos[1]; j++)
		{
			if (j < 0 || j >= _vMapData[i].size()) continue;

			// 주변 타일의 변경
			_vMapData[i][j]->_img = nullptr;
			_vMapData[i][j]->_img2 = DATAMANAGER->GetGridDataByName("Stage1_Tile43")->_image;
			switch (door->GetDirection()) // 방향에 따른 충돌픽셀 설정
			{
			case DIRECTION::DIR_LEFT:
				_vMapData[i][j]->_collisionImage = IMAGEMANAGER->findImage("Green_CollisionAll");
				break;
			case DIRECTION::DIR_RIGHT:
				_vMapData[i][j]->_collisionImage = IMAGEMANAGER->findImage("Green_CollisionAll2");
				break;
			case DIRECTION::DIR_UP:
				_vMapData[i][j]->_collisionImage = IMAGEMANAGER->findImage("Green_CollisionAll3");
				break;
			case DIRECTION::DIR_DOWN:
				_vMapData[i][j]->_collisionImage = IMAGEMANAGER->findImage("Green_CollisionAll4");
				break;
			}

			// 문의 방향에 따른 주변 타일 변경
			switch (door->GetDirection())
			{
			case DIRECTION::DIR_LEFT: case DIRECTION::DIR_RIGHT:
				if (i == y + pos[2] && i - 1 >= 0)
				{
					_vMapData[i - 1][j]->_img = DATAMANAGER->GetGridDataByName("Stage1_Tile19")->_image;
					_vMapData[i - 1][j]->_collisionImage = DATAMANAGER->GetGridDataByName("Stage1_Tile19")->_colImage;
				}

				if (i == y + pos[3] && i + 1 < _vMapData.size())
				{
					_vMapData[i + 1][j]->_img = DATAMANAGER->GetGridDataByName("Stage1_Tile1")->_image;
					_vMapData[i + 1][j]->_collisionImage = DATAMANAGER->GetGridDataByName("Stage1_Tile1")->_colImage;
				}
				break;

			case DIRECTION::DIR_UP:
				if (j == x + pos[0] && j - 1 >= 0)
				{
					_vMapData[i][j - 1]->_img = DATAMANAGER->GetGridDataByName("Stage1_Tile5")->_image;
					_vMapData[i][j - 1]->_collisionImage = DATAMANAGER->GetGridDataByName("Stage1_Tile5")->_colImage;
				}

				if (j == x + pos[1] && j + 1 <= _vMapData[i].size() - 1)
				{
					_vMapData[i][j + 1]->_img = DATAMANAGER->GetGridDataByName("Stage1_Tile3")->_image;
					_vMapData[i][j + 1]->_collisionImage = DATAMANAGER->GetGridDataByName("Stage1_Tile3")->_colImage;
				}

				if (i == y + pos[2] && i - 1 >= 0)
				{
					_vMapData[i - 1][j]->_img = DATAMANAGER->GetGridDataByName("Stage1_Tile19")->_image;
					_vMapData[i - 1][j]->_collisionImage = DATAMANAGER->GetGridDataByName("Stage1_Tile19")->_colImage;
				}
				break;

			case DIRECTION::DIR_DOWN:
				if (j == x + pos[0] && j - 1 >= 0)
				{
					_vMapData[i][j - 1]->_img = DATAMANAGER->GetGridDataByName("Stage1_Tile5")->_image;
					_vMapData[i][j - 1]->_collisionImage = DATAMANAGER->GetGridDataByName("Stage1_Tile5")->_colImage;
				}

				if (j == x + pos[1] && j + 1 <= _vMapData[i].size() - 1)
				{
					_vMapData[i][j + 1]->_img = DATAMANAGER->GetGridDataByName("Stage1_Tile3")->_image;
					_vMapData[i][j + 1]->_collisionImage = DATAMANAGER->GetGridDataByName("Stage1_Tile3")->_colImage;
				}

				if (i == y + pos[3] && i + 1 < _vMapData.size())
				{
					_vMapData[i + 1][j]->_img = DATAMANAGER->GetGridDataByName("Stage1_Tile1")->_image;
					_vMapData[i + 1][j]->_collisionImage = DATAMANAGER->GetGridDataByName("Stage1_Tile1")->_colImage;
				}
				break;
			}
		}
	}
}

void FieldMap::release()
{
	for (int i = 0; i < _vMapData.size(); i++)
	{
		for (int j = 0; j < _vMapData[i].size(); j++)
		{
			SAFE_DELETE(_vMapData[i][j]);
		}
	}

	for (int i = 0; i < _vObjs.size(); i++)
	{
		_vObjs[i]->release();
		SAFE_DELETE(_vObjs[i]);
	}
}

/// <summary>
/// 문 생성/해제 시 주변 타일의 픽셀충돌 온오프를 한다.
/// </summary>
void FieldMap::MakeNearTileCollision(Door* door, bool isActivate)
{
	int x = (door->GetX() + door->GetImage(0)->getWidth() / 2) / 48;	// 문의 중점 X
	int y = (door->GetY() + door->GetImage(0)->getHeight() / 2) / 48;	// 문의 중점 Y

	int pos[4] = { 0,0,0,0 };
	switch (door->GetDirection()) // 방향에 따라 변경할 타일의 범위가 달라진다.
	{
	case DIRECTION::DIR_LEFT:	pos[0] = -3;  pos[1] = 0, pos[2] = -2, pos[3] = 2; break;
	case DIRECTION::DIR_RIGHT:	pos[0] = 0;  pos[1] = 3, pos[2] = -2, pos[3] = 2; break;
	case DIRECTION::DIR_UP:		pos[0] = -2;  pos[1] = 1, pos[2] = -2, pos[3] = 0; break;
	case DIRECTION::DIR_DOWN:	pos[0] = -2;  pos[1] = 1, pos[2] = 0, pos[3] = 2; break;
	}

	// 주변 타일들을 돌며 해당 타일들의 충돌 정보를 바꾼다.
	for (int i = y + pos[2]; i <= y + pos[3]; i++)
	{
		if (i < 0 || i >= _vMapData.size()) continue;
		for (int j = x + pos[0]; j <= x + pos[1]; j++)
		{
			if (j < 0 || j >= _vMapData[i].size()) continue;

			if (isActivate)
			{
				_vMapData[i][j]->_collisionImage = IMAGEMANAGER->findImage("Red_CollisionAll");
			}
			else
			{
				switch (door->GetDirection()) // 방향에 따른 충돌픽셀 설정
				{
				case DIRECTION::DIR_LEFT:
					_vMapData[i][j]->_collisionImage = IMAGEMANAGER->findImage("Green_CollisionAll");
					break;
				case DIRECTION::DIR_RIGHT:
					_vMapData[i][j]->_collisionImage = IMAGEMANAGER->findImage("Green_CollisionAll2");
					break;
				case DIRECTION::DIR_UP:
					_vMapData[i][j]->_collisionImage = IMAGEMANAGER->findImage("Green_CollisionAll3");
					break;
				case DIRECTION::DIR_DOWN:
					_vMapData[i][j]->_collisionImage = IMAGEMANAGER->findImage("Green_CollisionAll4");
					break;
				}
			}
		}
	}

	PixelCollisionMapGenerate();
}

/// <summary>
/// 픽셀 콜리젼 이미지를 그려 픽셀충돌용 맵을 만드는 함수
/// </summary>
void FieldMap::PixelCollisionMapGenerate()
{
	Rectangle(IMAGEMANAGER->findImage("PixelMapIg")->getMemDC(), -10, -10, 10000, 10000); // 픽셀충돌 이미지 도화지에 커다란 흰색 RECT를 끼얹는다

	for (int i = 0; i < _vMapData.size(); i++)
	{
		for (int j = 0; j < _vMapData[i].size(); j++)
		{
			if (_vMapData[i][j]->_collisionImage != nullptr)
			{
				_vMapData[i][j]->_collisionImage->render(IMAGEMANAGER->findImage("PixelMapIg")->getMemDC(), _vMapData[i][j]->_x, _vMapData[i][j]->_y); // 충돌용 배경에 충돌용 타일 배치
			}
		}
	}
}

/// <summary>
/// 맵 전체 타일 이미지에 그려 맵을 만드는 함수
/// </summary>
void FieldMap::GridMapGenerate()
{
	Rectangle(IMAGEMANAGER->findImage("Layer1MapIg")->getMemDC(), -10, -10, 10000, 10000); // 픽셀충돌 이미지 도화지에 커다란 흰색 RECT를 끼얹는다
	Rectangle(IMAGEMANAGER->findImage("Layer2MapIg")->getMemDC(), -10, -10, 10000, 10000); // 픽셀충돌 이미지 도화지에 커다란 흰색 RECT를 끼얹는다
	Rectangle(IMAGEMANAGER->findImage("MiniMapGroundIg")->getMemDC(), -10, -10, 10000, 10000); // 미니맵 이미지 도화지에 커다란 흰색 RECT를 끼얹는다

	for (int i = 0; i < _vMapData.size(); i++)
	{
		for (int j = 0; j < _vMapData[i].size(); j++)
		{
			if (_vMapData[i][j]->_img != nullptr)
			{
				_vMapData[i][j]->_img->render(IMAGEMANAGER->findImage("Layer1MapIg")->getMemDC(), _vMapData[i][j]->_x, _vMapData[i][j]->_y); // 충돌용 배경에 충돌용 타일 배치
			}

			if (_vMapData[i][j]->_img2 != nullptr)
			{
				_vMapData[i][j]->_img2->render(IMAGEMANAGER->findImage("Layer2MapIg")->getMemDC(), _vMapData[i][j]->_x, _vMapData[i][j]->_y); // 충돌용 배경에 충돌용 타일 배치
			}
		}
	}
	for (int i = 0; i < _vMiniRc.size(); i++)
	{
		IMAGEMANAGER->findImage("MiniMapPixel")->render(IMAGEMANAGER->findImage("MiniMapGroundIg")->getMemDC(), _vMiniRc[i].left, _vMiniRc[i].top);
	} // 미니맵 렌더
}

void FieldMap::update()
{
	for (int i = 0; i < _vObjs.size(); i++)
	{
		_vObjs[i]->update();
	}

	SpawnMonsters();
	EraseDeathObject();
	ShotObject();
	CheckNoMonsterInMap();
	EFFECTMANAGER->update();
}

/// <summary>
/// 문을 활성화 상태로 만든다
/// </summary>
void FieldMap::SetDoorSpawning()
{
	for (int i = 0; i < _vObjs.size(); i++)
	{
		switch (_vObjs[i]->GetId())
		{
		case 514: case 515: case 516: case 517:
			dynamic_cast<Door*>(_vObjs[i])->SetIsSpawning(true);
			_vObjs[i]->SetUseImage(1);	// 활성화 이미지로 변경
			MakeNearTileCollision(dynamic_cast<Door*>(_vObjs[i]), true);
			break;
		}
	}
}

/// <summary>
/// 몬스터 타입인 오브젝트들을 시간에 맞추어 스폰한다.
/// </summary>
void FieldMap::SpawnMonsters()
{
	if (_isSpawning && !_isCleared)
	{
		_spawnTimer++;
		for (int i = 0; i < _vObjs.size(); i++)
		{
			if (_vObjs[i]->GetType() == OBJECTTYPE::OT_MONSTER)
			{
				Enemy* enemy = dynamic_cast<Enemy*>(_vObjs[i]);

				if (!enemy->GetIsSpawned() && enemy->GetSpawnTime() <= _spawnTimer)
				{
					enemy->SpawnEnemy();
				}
			}
		}
	}
}

/// <summary>
/// 맵에 몬스터가 더 없으면 문을 연다
/// </summary>
void FieldMap::CheckNoMonsterInMap()
{
	if (_isSpawning && !_isCleared)
	{
		bool isRemainMonster = false;
		for (int i = 0; i < _vObjs.size(); i++)
		{
			if (_vObjs[i]->GetType() == OBJECTTYPE::OT_MONSTER)
			{
				isRemainMonster = true;
				break;
			}
		}

		if (!isRemainMonster) // 남은 몬스터가 없을때
		{
			_isCleared = true; // 몬스터를 모두 정리했다 알림
			for (int i = 0; i < _vObjs.size(); i++)
			{
				switch (_vObjs[i]->GetId())
				{
				case 514: case 515: case 516: case 517: // 문 Case
					dynamic_cast<Door*>(_vObjs[i])->SetIsActivated(false);
					break;
				}
			}
		}
	}
}

/// <summary>
/// 사망한 오브젝트를 없앤다.
/// </summary>
void FieldMap::EraseDeathObject()
{
	for (int i = 0; i < _vObjs.size(); i++)
	{
		if (_vObjs[i]->GetIsDead())
		{
			_vObjs.erase(_vObjs.begin() + i);
			i--;
		}
	}
}

/// <summary>
/// 테스트용, 쏴서 오브젝트를 파괴함.
/// </summary>
void FieldMap::ShotObject()
{
	if (INPUT->GetKeyDown(VK_LBUTTON))
	{
		for (int i = 0; i < _vObjs.size(); i++)
		{
			if (PtInRect(&_vObjs[i]->GetBody(), CAMERAMANAGER->GetAbsolutePoint(_ptMouse.x, _ptMouse.y)))
			{
				_vObjs[i]->SetIsDead(true);
				break;
			}
		}
	}
}

void FieldMap::ChangePlayerByDirection(DIRECTION dir)
{
	ENTITYMANAGER->getPlayer()->SetX(_mapMovePos[dir].x);
	ENTITYMANAGER->getPlayer()->SetY(_mapMovePos[dir].y);
	ENTITYMANAGER->getPlayer()->SetBodyPos();
}

void FieldMap::render(HDC hdc)
{
	if (_backImageMain != nullptr) _backImageMain->loopRender(hdc, &RectMake(0, 0, WINSIZEX, WINSIZEY), CAMERAMANAGER->GetRect().left / 4, CAMERAMANAGER->GetRect().top / 4);
	
	int mapSizeX = _vMapData[0].size() * 48;
	int mapSizeY = _vMapData.size() * 48;
	
	if (_backImageEtc != nullptr)
	{
		CAMERAMANAGER->StretchRender(hdc, _backImageEtc, -WINSIZEX / 2, -WINSIZEY / 2, WINSIZEX + mapSizeX, WINSIZEY / 2);
		CAMERAMANAGER->StretchRender(hdc, _backImageEtc, -WINSIZEX / 2, 0, WINSIZEX / 2, mapSizeY);
		CAMERAMANAGER->StretchRender(hdc, _backImageEtc, -WINSIZEX / 2, mapSizeY, WINSIZEX + mapSizeX, WINSIZEY / 2);
		CAMERAMANAGER->StretchRender(hdc, _backImageEtc, mapSizeX, 0, WINSIZEX / 2, mapSizeY);
	}

	CAMERAMANAGER->Render(hdc, IMAGEMANAGER->findImage("Layer2MapIg"), 0, 0);
	for (int i = 0; i < _vObjs.size(); i++)
	{
		_vObjs[i]->render(hdc);
	} // 오브젝트 렌더

	CAMERAMANAGER->Render(hdc, IMAGEMANAGER->findImage("Layer1MapIg"), 0, 0);

	IMAGEMANAGER->findImage("MiniMapGroundIg")->render(hdc, 0, 0);
	if (INPUT->GetKey(VK_F1))
	{
		CAMERAMANAGER->Render(hdc, IMAGEMANAGER->findImage("PixelMapIg"), 0, 0);
	} // 픽셀충돌 렌더

	/*
	for (int i = 0; i < _vObjs.size(); i++)
	{
		_vObjs[i]->render(hdc);
	} // 오브젝트 렌더
	*/


	ENTITYMANAGER->render(hdc);
	EFFECTMANAGER->render(hdc);
	PARTICLEMANAGER->render(hdc);
	// 플레이어 및 불릿 등 렌더
}


