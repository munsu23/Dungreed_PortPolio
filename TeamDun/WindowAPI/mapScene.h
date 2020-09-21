#pragma once
#include "uibrushTool.h"
#include "MapTool.h"
class mapScene : public gameNode
{
private:
	// UI BRUSH 관련 //
	uibrushTool*	_uiBrushTool;
	
	// MAP TOOL 관련 //
	MapTool*		_mapTool;
	image*			_targetImage;
	

	// FILL RANGE 관련 //
	bool			_isFillClicked;
	POINT			_clickedPointOne;
	POINT			_clickedPointTwo;

	// 초기 사이즈 설정 관련 //
	int				_heightNum;
	int				_widthNum;
	bool			_isSettingPage;

	// 회전 관련 (테스트용) //
	int				_rotateTester;
	int				_rotTimer;

	POINT			_pivot;

	// 세이브 로드용 //
	bool			_isEditerViewing;
	char			_fileName[128];
	bool			_isLoad;

	// 클릭 관련//
	bool			_isLeftClicked;
	bool			_isRightClicked;

	image * _shorcutKeyIg;
	
public :
	HRESULT init();
	void UIInit();
	void release();
	void update();
	void InputCheck();
	void SetMapSize();
	void RemovePaint();
	void FillSquareRange();
	void Paint();
	void FloodFill();
	void FillAll();
	void GetUiBrush();
	void CallSaveLoadEditor();
	void AddMapLine();
	void Undo();
	void render();
	void CameraMove();
	void SaveLoadMap();
	

	// GETSET //
	image* GetTargetImage() { return _targetImage; }
	void SetTargetImage(image* img) { _targetImage = img; }
	void SaveShortcutKey();
	void LoadShortcutKey();
};

