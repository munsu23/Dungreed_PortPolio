#include "stdafx.h"
#include "inventory.h"
#include "player.h"

HRESULT Inventory::init()
{
	_InvenFrame = UIMANAGER->GetGameFrame()->GetChild("InventoryFrame");
	_shopFrame = UIMANAGER->GetGameFrame()->GetChild("DungeonShopBase");
	_swapFrame = UIMANAGER->GetGameFrame()->GetChild("swapContainer");

	_invenFullTextOn = false;
	_invenFullTextTimer = 0;

	_p = ENTITYMANAGER->getPlayer();
	return S_OK;
}

void Inventory::update()
{
	CheckInvenText();

	if (_InvenFrame->GetIsViewing())
	{
		UpdateMoney();
		EquipItem();
		UnEquipItem();
		ShowToolTip();
	}
}

void Inventory::render(HDC hdc)
{
}

void Inventory::release()
{
}

void Inventory::UpdateMoney()
{
	dynamic_cast<UIText*>(_InvenFrame->GetChild("moneyText"))->SetText(to_string(_p->GetMoney()));
}

void Inventory::EquipItem()
{
	if (INPUT->GetIsRButtonClicked())
	{
		if (!_shopFrame->GetIsViewing())
		{
			for (int i = 0; i < 15; i++)
			{
				UIFrame* curFrame = _InvenFrame->GetChild("itemFrame_" + to_string(i))->GetChild("itemImageFrame");

				if (PtInRect(&curFrame->GetRect(), _ptMouse))
				{
					if (_vInvenItems.size() > i)
					{
						SOUNDMANAGER->play("게임_아이템장착 (2)");
						Item* item = _vInvenItems[i];
						item->EquipUnEquipStatus(true);

						switch (item->GetitemType())
						{
						case ITEMTYPE::IT_WEAPON_ONEHAND:
							if (_p->GetWeapon(0) == nullptr)
							{ // 0번에 착용
								_p->SetWeapon(0, item);
								if (_p->GetSelectedWeaponIdx() != 0) item->EquipUnEquipStatus(false);
								_vInvenItems.erase(_vInvenItems.begin() + i);
							}
							else if (_p->GetWeapon(1) == nullptr)
							{ // 1번에 착용
								_p->SetWeapon(1, item);
								if (_p->GetSelectedWeaponIdx() != 1) item->EquipUnEquipStatus(false);
								_vInvenItems.erase(_vInvenItems.begin() + i);
							}
							else
							{ // 0번과 switch
								_p->GetWeapon(0)->EquipUnEquipStatus(false);
								if (_p->GetSelectedWeaponIdx() != 0) item->EquipUnEquipStatus(false);
								SwitchItem(0, item, i);
							}
							break;

						case ITEMTYPE::IT_WEAPON_TWOHAND:
							if (_p->GetWeapon(0) == nullptr)
							{ // 0번에 착용
								_p->SetWeapon(0, item);
								if (_p->GetSelectedWeaponIdx() != 0) item->EquipUnEquipStatus(false);
								_vInvenItems.erase(_vInvenItems.begin() + i);
							}

							else if (_p->GetWeapon(1) == nullptr)
							{ // 1번에 착용
								_p->SetWeapon(1, item);
								if (_p->GetSelectedWeaponIdx() != 1) item->EquipUnEquipStatus(false);
								_vInvenItems.erase(_vInvenItems.begin() + i);
							}

							else if (_vInvenItems.size() >= 15)
							{ // 미착용
								OnInvenFullText();
								item->EquipUnEquipStatus(false);
							}
							else
							{ // 0번과 switch
								if (_p->GetSelectedWeaponIdx() == 0) _p->GetWeapon(0)->EquipUnEquipStatus(false);
								if (_p->GetSelectedWeaponIdx() != 0) item->EquipUnEquipStatus(false);
								SwitchItem(0, item, i);
								if (_p->GetSubWeapon(0) != nullptr)
								{
									AddItem(_p->GetSubWeapon(0));
									if (_p->GetSelectedWeaponIdx() == 0) _p->GetSubWeapon(0)->EquipUnEquipStatus(false);
									_p->SetSubWeapon(0, nullptr);
								}
							}
							break;

						case ITEMTYPE::IT_SUBWEAPON:

							if ((_p->GetWeapon(0) == nullptr || (_p->GetWeapon(0)->GetitemType() == ITEMTYPE::IT_WEAPON_TWOHAND)) &&
								(_p->GetWeapon(1) == nullptr || (_p->GetWeapon(1)->GetitemType() == ITEMTYPE::IT_WEAPON_TWOHAND)))
							{ // 양쪽 다 서브무기를 낄 수 없는 상태라면
								OnInvenFullText();
								item->EquipUnEquipStatus(false);
							}

							else if (_p->GetWeapon(0) != nullptr && _p->GetWeapon(0)->GetitemType() != ITEMTYPE::IT_WEAPON_TWOHAND && _p->GetSubWeapon(0) == nullptr)
							{ // 무기 0이 있고 두손무기가 아니며, 서브웨폰이 없다면
								_p->SetSubWeapon(0, item);
								if (_p->GetSelectedWeaponIdx() != 0) item->EquipUnEquipStatus(false);
								_vInvenItems.erase(_vInvenItems.begin() + i);
							}

							else if (_p->GetWeapon(1) != nullptr && _p->GetWeapon(1)->GetitemType() != ITEMTYPE::IT_WEAPON_TWOHAND && _p->GetSubWeapon(1) == nullptr)
							{ // 무기 1이 있고 두손무기가 아니며, 서브웨폰이 없다면
								_p->SetSubWeapon(1, item);
								if (_p->GetSelectedWeaponIdx() != 1) item->EquipUnEquipStatus(false);
								_vInvenItems.erase(_vInvenItems.begin() + i);
							}

							else
							{
								if (_p->GetWeapon(0) != nullptr && _p->GetWeapon(0)->GetitemType() != ITEMTYPE::IT_WEAPON_TWOHAND && _p->GetSubWeapon(0) != nullptr)
								{
									_p->GetSubWeapon(0)->EquipUnEquipStatus(false);
									if (_p->GetSelectedWeaponIdx() != 0) item->EquipUnEquipStatus(false);
									SwitchItem(1, item, i);
								}
								else
								{
									OnInvenFullText();
									item->EquipUnEquipStatus(false);
								}
							}
							break;

						case ITEMTYPE::IT_ACCESORRY:
							bool equalCheck = false;
							for (int i = 0; i < _p->GetVAccessories().size(); i++)
							{
								if (_p->GetAccessory(i)->GetId() == item->GetId())
								{
									OnInvenEqualText();
									item->EquipUnEquipStatus(false);
									equalCheck = true;
									break;
								}
							}

							if (equalCheck) break;

							if (_p->GetVAccessories().size() == _p->GetAccesoryCount())
							{
								OnInvenFullText();
								item->EquipUnEquipStatus(false);
							}

							else
							{
								_p->GetVAccessories().push_back(item);
								_vInvenItems.erase(_vInvenItems.begin() + i);
							}
							break;
						}

						ReloadUIImages();
						break;
					}
				}
			}
		}
	}
}

void Inventory::SwitchItem(int num, Item* item, int index)
{
	Item* getItem;
	if (num == 0)
	{
		getItem = _p->GetWeapon(0);
		_p->SetWeapon(0, item);
		_vInvenItems[index] = getItem;
	}

	else if (num == 1)
	{
		getItem = _p->GetSubWeapon(0);
		_p->SetSubWeapon(0, item);
		_vInvenItems[index] = getItem;
	}

	else if (num == 2)
	{
		getItem = _p->GetWeapon(1);
		_p->SetWeapon(1, item);
		_vInvenItems[index] = getItem;
	}

	else if (num == 3)
	{
		getItem = _p->GetSubWeapon(1);
		_p->SetSubWeapon(1, item);
		_vInvenItems[index] = getItem;
	}

	else
	{
		getItem = _p->GetAccessory(num - 4);
		_p->SetVAccessory(num - 4, item);
		_vInvenItems[index] = getItem;
	}
}

void Inventory::UnEquipItem()
{
	if (INPUT->GetIsRButtonClicked())
	{
		if (!_shopFrame->GetIsViewing())
		{
			if (PtInRect(&_InvenFrame->GetChild("curWeapon_1")->GetRect(), _ptMouse) && _p->GetWeapon(0) != nullptr)
			{
				SOUNDMANAGER->play("게임_아이템장착 (2)");

				if ((_vInvenItems.size() > 13 && _p->GetWeapon(0)->GetitemType() == ITEMTYPE::IT_WEAPON_TWOHAND && _p->GetSubWeapon(0) != nullptr) || _vInvenItems.size() > 14)
				{
					OnInvenFullText();
				}

				else
				{
					AddItem(_p->GetWeapon(0));
					if (_p->GetSelectedWeaponIdx() == 0)	_p->GetWeapon(0)->EquipUnEquipStatus(false);
					_p->SetWeapon(0, nullptr);
					if (_p->GetSubWeapon(0) != nullptr)
					{
						AddItem(_p->GetSubWeapon(0));
						if (_p->GetSelectedWeaponIdx() == 0) _p->GetSubWeapon(0)->EquipUnEquipStatus(false);
						_p->SetSubWeapon(0, nullptr);
					}
				}
			}

			else if (PtInRect(&_InvenFrame->GetChild("curWeapon_2")->GetRect(), _ptMouse) && _p->GetWeapon(1) != nullptr)
			{
				if ((_vInvenItems.size() > 13 && _p->GetWeapon(1)->GetitemType() == ITEMTYPE::IT_WEAPON_TWOHAND && _p->GetSubWeapon(1) != nullptr) || _vInvenItems.size() > 14)
				{
					OnInvenFullText();
				}

				else
				{
					AddItem(_p->GetWeapon(1));
					if (_p->GetSelectedWeaponIdx() == 1) _p->GetWeapon(1)->EquipUnEquipStatus(false);
					_p->SetWeapon(1, nullptr);
					if (_p->GetSubWeapon(1) != nullptr)
					{
						AddItem(_p->GetSubWeapon(1));
						if (_p->GetSelectedWeaponIdx() == 1) _p->GetSubWeapon(1)->EquipUnEquipStatus(false);
						_p->SetSubWeapon(1, nullptr);
					}
				}
			}

			else if (PtInRect(&_InvenFrame->GetChild("curWeaponSub_1")->GetRect(), _ptMouse) && _p->GetSubWeapon(0) != nullptr)
			{
				if (_vInvenItems.size() > 14)
				{
					OnInvenFullText();
				}

				else
				{
					AddItem(_p->GetSubWeapon(0));
					if (_p->GetSelectedWeaponIdx() == 0) _p->GetSubWeapon(0)->EquipUnEquipStatus(false);
					_p->SetSubWeapon(0, nullptr);
				}
			}

			else if (PtInRect(&_InvenFrame->GetChild("curWeaponSub_2")->GetRect(), _ptMouse) && _p->GetSubWeapon(1) != nullptr)
			{
				if (_vInvenItems.size() > 14)
				{
					OnInvenFullText();
				}

				else
				{
					AddItem(_p->GetSubWeapon(1));
					if (_p->GetSelectedWeaponIdx() == 1) _p->GetSubWeapon(1)->EquipUnEquipStatus(false);
					_p->SetSubWeapon(1, nullptr);
				}
			}

			else
			{
				for (int i = 0; i < _p->GetAccesoryCount(); i++)
				{
					if (PtInRect(&_InvenFrame->GetChild("accesoryFrame_" + to_string(i))->GetRect(), _ptMouse) && _p->GetVAccessories().size() > i)
					{
						if (_vInvenItems.size() > 14)
						{
							OnInvenFullText();
						}

						else
						{
							AddItem(_p->GetAccessory(i));
							_p->GetAccessory(i)->EquipUnEquipStatus(false);
							_p->GetVAccessories().erase(_p->GetVAccessories().begin() + i);
						}
						break;
					}
				}
			}

			ReloadUIImages();
		}
	}
}

bool Inventory::AddItem(Item* item)
{
	if (_vInvenItems.size() < 15)
	{
		_vInvenItems.push_back(item);
		ReloadUIImages();
		return true;
	}

	return false;
}

void Inventory::ReloadUIImages()
{
	Player* p = ENTITYMANAGER->getPlayer();
	_swapFrame->GetChild("weapon1")->GetVChildFrames().clear();
	_swapFrame->GetChild("weapon2")->GetVChildFrames().clear();

	UIFrame* weapon1Swap = new UIFrame();
	_InvenFrame->GetChild("curWeapon_1")->SetImage(nullptr);
	if (p->GetWeapon(0) != nullptr)
	{
		_InvenFrame->GetChild("curWeapon_1")->SetImage(p->GetWeapon(0)->GetInvenImage());
		weapon1Swap->init("image",
			25 - p->GetWeapon(0)->GetDropImage()->getFrameWidth() / 2 * (p->GetWeapon(0)->GetRenderScale() - 1),
			15 - p->GetWeapon(0)->GetDropImage()->getFrameHeight() / 2 * (p->GetWeapon(0)->GetRenderScale() - 1),
			0,0,
			p->GetWeapon(0)->GetDropImageName(),
			p->GetWeapon(0)->GetRenderScale(),
			p->GetWeapon(0)->GetRenderScale()
		);
	}
	else
	{
		weapon1Swap->init("image", 0, 0, 0, 0, "");
	}
	_swapFrame->GetChild("weapon1")->AddFrame(weapon1Swap);
	UIFrame* weapon2Swap = new UIFrame();
	_InvenFrame->GetChild("curWeapon_2")->SetImage(nullptr);
	if (p->GetWeapon(1) != nullptr)
	{
		_InvenFrame->GetChild("curWeapon_2")->SetImage(p->GetWeapon(1)->GetInvenImage());
		weapon2Swap->init("image",
			25 - p->GetWeapon(1)->GetDropImage()->getFrameWidth() / 2 * (p->GetWeapon(1)->GetRenderScale() - 1),
			15 - p->GetWeapon(1)->GetDropImage()->getFrameHeight() / 2 * (p->GetWeapon(1)->GetRenderScale() - 1),
			p->GetWeapon(1)->GetDropImage()->getFrameWidth() * p->GetWeapon(1)->GetRenderScale(),
			p->GetWeapon(1)->GetDropImage()->getFrameHeight() * p->GetWeapon(1)->GetRenderScale(),
			p->GetWeapon(1)->GetDropImageName(),
			p->GetWeapon(1)->GetRenderScale(),
			p->GetWeapon(1)->GetRenderScale()
		);
	}
	else
	{
		weapon2Swap->init("image", 0, 0, 0, 0, "");
	}
	_swapFrame->GetChild("weapon2")->AddFrame(weapon2Swap);

	_InvenFrame->GetChild("curWeaponSub_1")->SetImage(nullptr);
	if (p->GetSubWeapon(0) != nullptr) _InvenFrame->GetChild("curWeaponSub_1")->SetImage(p->GetSubWeapon(0)->GetInvenImage());
	_InvenFrame->GetChild("curWeaponSub_2")->SetImage(nullptr);
	if (p->GetSubWeapon(1) != nullptr) _InvenFrame->GetChild("curWeaponSub_2")->SetImage(p->GetSubWeapon(1)->GetInvenImage());
	for (int i = 0; i < p->GetAccesoryCount(); i++)
	{
		_InvenFrame->GetChild("accesoryFrame_" + to_string(i))->GetChild("itemImageFrame")->SetImage(nullptr);
		if (p->GetVAccessories().size() > i && p->GetAccessory(i) != nullptr)
			_InvenFrame->GetChild("accesoryFrame_" + to_string(i))->GetChild("itemImageFrame")->SetImage(p->GetAccessory(i)->GetInvenImage());
	}

	for (int i = 0; i < 15; i++)
	{
		_InvenFrame->GetChild("itemFrame_" + to_string(i))->GetChild("itemImageFrame")->SetImage(nullptr);
		if (i < _vInvenItems.size())
			_InvenFrame->GetChild("itemFrame_" + to_string(i))->GetChild("itemImageFrame")->SetImage(_vInvenItems[i]->GetInvenImage());
	}
}

void Inventory::OnInvenFullText()
{
	_invenFullTextOn = true;
	_invenFullTextTimer = 0;
	UIMANAGER->GetGameFrame()->GetChild("isFullText")->SetIsViewing(true);
}

void Inventory::OnInvenEqualText()
{
	_invenEqualTextOn = true;
	_invenEqualTextTimer = 0;
	UIMANAGER->GetGameFrame()->GetChild("isEqualText")->SetIsViewing(true);
}

void Inventory::CheckInvenText()
{
	if (_invenFullTextOn)
	{
		_invenFullTextTimer++;
		if (_invenFullTextTimer > 25)
		{
			UIMANAGER->GetGameFrame()->GetChild("isFullText")->SetIsViewing(false);
			_invenFullTextTimer = 0;
			_invenFullTextOn = false;
		}
	}

	if (_invenEqualTextOn)
	{
		_invenEqualTextTimer++;
		if (_invenEqualTextTimer > 25)
		{
			UIMANAGER->GetGameFrame()->GetChild("isEqualText")->SetIsViewing(false);
			_invenEqualTextTimer = 0;
			_invenEqualTextOn = false;
		}
	}
}

void Inventory::SwitchWeapon(int selectedWeapon)
{
	if (selectedWeapon == 0)	//선택무기가 0번일때
	{
		if (_p->GetWeapon(0) != nullptr)	//공격무기가 0번이 아닐
			_p->GetWeapon(0)->EquipUnEquipStatus(true);
		if (_p->GetSubWeapon(0) != nullptr)
			_p->GetSubWeapon(0)->EquipUnEquipStatus(true);
		if (_p->GetWeapon(1) != nullptr)
			_p->GetWeapon(1)->EquipUnEquipStatus(false);
		if (_p->GetSubWeapon(1) != nullptr)
			_p->GetSubWeapon(1)->EquipUnEquipStatus(false);
	}

	else
	{
		if (_p->GetWeapon(0) != nullptr)
			_p->GetWeapon(0)->EquipUnEquipStatus(false);
		if (_p->GetSubWeapon(0) != nullptr)
			_p->GetSubWeapon(0)->EquipUnEquipStatus(false);
		if (_p->GetWeapon(1) != nullptr)
			_p->GetWeapon(1)->EquipUnEquipStatus(true);
		if (_p->GetSubWeapon(1) != nullptr)
			_p->GetSubWeapon(1)->EquipUnEquipStatus(true);
	}
}

void Inventory::InitToolTipItem(Item* item)
{
	_curToolTipItem = item;

	UIImage* uiToolTip = dynamic_cast<UIImage*>(UIMANAGER->GetGameFrame()->GetChild("InventoryFrame")->GetChild("itemToolTip"));
	uiToolTip->GetVChildFrames().clear();

	// 이름 색
	int nameR = 255, nameG = 255, nameB = 255;
	switch (item->GetItemClass())
	{
	case ITEMCLASS::IC_NORMAL: nameR = 255, nameG = 255, nameB = 255; break;
	case ITEMCLASS::IC_ADVANCED: nameR = 112, nameG = 146, nameB = 190; break;
	case ITEMCLASS::IC_RARE: nameR = 232, nameG = 239, nameB = 90; break;
	case ITEMCLASS::IC_LEGENDARY: nameR = 237, nameG = 9, nameB = 138; break;
	}

	// 이름
	UIText* itemName = new UIText();
	itemName->init("itemName", 0, 10, 400, 200, item->GetName(), FONT::PIX, WORDSIZE::WS_MIDDLE, WORDSORT::WSORT_MIDDLE, RGB(nameR, nameG, nameB));
	uiToolTip->AddFrame(itemName);

	// 이미지 프레임
	UIFrame* imgFrame = new UIFrame();
	imgFrame->init("imgFrame", 10, 50, IMAGEMANAGER->findImage("IconWhite")->getWidth(), IMAGEMANAGER->findImage("IconWhite")->getHeight(), "IconWhite");
	uiToolTip->AddFrame(imgFrame);

	// 아이템 이미지
	UIFrame* itemImage = new UIFrame();
	itemImage->init("itemImage", 0, 0, item->GetInvenImage()->getWidth(), item->GetInvenImage()->getHeight(), item->GetInvenImageName());
	imgFrame->AddFrame(itemImage);

	// 공격력
	if (item->GetMinAtk() != 0 && item->GetMaxAtk() != 0)
	{
		UIText* attack = new UIText();
		attack->init("attack", 75, 50, 80, 80, "공격력 : ", FONT::PIX, WORDSIZE::WS_SMALL, WORDSORT::WSORT_LEFT, RGB(255, 255, 255));
		uiToolTip->AddFrame(attack);

		UIText* attackValue = new UIText();
		attackValue->init("attackValue", 155, 50, 200, 80, to_string_with_precision(item->GetMinAtk(), 0) + " ~ " + to_string_with_precision(item->GetMaxAtk(), 0), FONT::PIX, WORDSIZE::WS_SMALL, WORDSORT::WSORT_LEFT, RGB(230, 230, 0));
		uiToolTip->AddFrame(attackValue);
	}

	// 공격속도
	if (item->GetAtkSpeed() != 0)
	{
		UIText* attackSpd = new UIText();
		attackSpd->init("attackSpd", 75, 70, 200, 80, "초당 공격 횟수 : ", FONT::PIX, WORDSIZE::WS_SMALL, WORDSORT::WSORT_LEFT, RGB(255, 255, 255));
		uiToolTip->AddFrame(attackSpd);

		UIText* attackSpdValue = new UIText();
		attackSpdValue->init("attackSpdValue", 225, 70, 200, 80, to_string_with_precision(item->GetAtkSpeed(), 2), FONT::PIX, WORDSIZE::WS_SMALL, WORDSORT::WSORT_LEFT, RGB(230, 230, 0));
		uiToolTip->AddFrame(attackSpdValue);
	}

	// 방어력
	if (item->GetDefence() != 0)
	{
		UIText* def = new UIText();
		def->init("def", 75, 90, 200, 80, "방어력 : ", FONT::PIX, WORDSIZE::WS_SMALL, WORDSORT::WSORT_LEFT, RGB(255, 255, 255));
		uiToolTip->AddFrame(def);

		UIText* defValue = new UIText();
		defValue->init("defValue", 155, 90, 200, 80, to_string_with_precision(item->GetDefence(), 0), FONT::PIX, WORDSIZE::WS_SMALL, WORDSORT::WSORT_LEFT, RGB(230, 230, 0));
		uiToolTip->AddFrame(defValue);
	}

	// 추가 옵션
	for (int i = 0; i < item->GetSubOptions().size(); i++)
	{
		SubOption* option = item->GetSubOptions()[i];

		int optionR = 255, optionG = 255, optionB = 255;
		if (option->_optionPower < 0) optionR = 255, optionG = 0, optionB = 0;
		if (option->_optionPower > 0) optionR = 0, optionG = 255, optionB = 0;

		UIText* startText = new UIText();
		startText->init("▶" + to_string(i), 15, 120 + i * 15, 30, 30, "▶", FONT::PIX, WORDSIZE::WS_SMALLEST);
		uiToolTip->AddFrame(startText);

		UIText* descript = new UIText();
		descript->init("descript" + to_string(i), 30, 122 + i * 15, 370, 30, OptionString(option), FONT::PIX, WORDSIZE::WS_SMALLEST, WORDSORT::WSORT_LEFT,
			RGB(optionR, optionG, optionB));
		uiToolTip->AddFrame(descript);
	}

	_toolTipFinalY = item->GetSubOptions().size() * 20 + 122;

	// 아이템 클래스
	string itemClassString = "일반 아이템";
	switch (item->GetItemClass())
	{
	case ITEMCLASS::IC_NORMAL: itemClassString = "일반 아이템"; break;
	case ITEMCLASS::IC_ADVANCED: itemClassString = "고급 아이템";  break;
	case ITEMCLASS::IC_RARE: itemClassString = "희귀 아이템"; break;
	case ITEMCLASS::IC_LEGENDARY: itemClassString = "전설 아이템"; break;
	}

	UIText* itemClass = new UIText();
	itemClass->init("itemClass", 10, _toolTipFinalY + 5, 150, 30, itemClassString, FONT::PIX, WORDSIZE::WS_SMALLEST, WORDSORT::WSORT_LEFT, RGB(120, 120, 120));
	uiToolTip->AddFrame(itemClass);

	// 아이템 종류
	string itemKindString = "양손무기";
	switch (item->GetitemType())
	{
	case ITEMTYPE::IT_WEAPON_ONEHAND: itemKindString = "한손무기"; break;
	case ITEMTYPE::IT_WEAPON_TWOHAND: itemKindString = "양손무기"; break;
	case ITEMTYPE::IT_SUBWEAPON: itemKindString = "보조장비"; break;
	case ITEMTYPE::IT_ACCESORRY: itemKindString = "악세서리"; break;
	}

	UIText* itemKind = new UIText();
	itemKind->init("itemKind", 10, _toolTipFinalY + 20, 150, 30, itemKindString, FONT::PIX, WORDSIZE::WS_SMALLEST, WORDSORT::WSORT_LEFT, RGB(120, 120, 120));
	uiToolTip->AddFrame(itemKind);
	_toolTipFinalY += 15;

	// 비고
	string weaponkindString = "";
	switch (item->GetWeaponType())
	{
	case WEAPONTYPE::WT_CHARGE: weaponkindString = "#충전형"; break;
	case WEAPONTYPE::WT_KATANA: weaponkindString = "#카타나"; break;
	case WEAPONTYPE::WT_PISTOL: weaponkindString = "#권총"; break;
	case WEAPONTYPE::WT_SPEAR: weaponkindString = "#창"; break;
	}

	if (weaponkindString != "")
	{
		UIText* tag = new UIText();
		tag->init("tag", 10, _toolTipFinalY + 25, 100, 30, weaponkindString, FONT::PIX, WORDSIZE::WS_SMALLEST, WORDSORT::WSORT_LEFT, RGB(162, 210, 148));
		uiToolTip->AddFrame(tag);
		_toolTipFinalY += 20;
	}

	// 스킬
	if (item->GetSkill() != nullptr)
	{
		UIImage* skillFrame = new UIImage();
		skillFrame->init("skillFrame", 10, _toolTipFinalY + 25, 280, 55, "ToolTipSkillCover", false, 0, 0, 280 / 30.f, 55 / 30.f, 80);
		uiToolTip->AddFrame(skillFrame);
		UIFrame* skillImage = new UIFrame();
		skillImage->init("skillImage", 2, 2, 51, 51, item->GetSkill()->GetImageName(), 51.f / 57, 51.f / 57);
		skillFrame->AddFrame(skillImage);
		UIText* skillName = new UIText();
		skillName->init("skillName", 55, 2, 300, 80, item->GetSkill()->GetName(), FONT::PIX, WORDSIZE::WS_SMALL, WORDSORT::WSORT_LEFT, RGB(230, 230, 0));
		skillFrame->AddFrame(skillName);
		UIText* skillDescription = new UIText();
		skillDescription->init("skillDescription", 55, 22, 220, 120, item->GetSkill()->GetDescription(), FONT::PIX, WORDSIZE::WS_SMALLEST, WORDSORT::WSORT_LEFT);
		skillFrame->AddFrame(skillDescription);
		UIText* skillCoolTime = new UIText();
		skillCoolTime->init("skillCoolTime", 257, 39, 20, 30, to_string_with_precision(item->GetSkill()->GetCoolTime(), 1), FONT::PIX, WORDSIZE::WS_SMALLEST, WORDSORT::WSORT_RIGHT, RGB(221, 173, 103));
		skillFrame->AddFrame(skillCoolTime);
		_toolTipFinalY += 45;
	}

	// 설명
	UIText* description = new UIText();
	description->init("description", 10, _toolTipFinalY + 45, 350, 200, item->GetDescription(), FONT::PIX, WORDSIZE::WS_SMALLEST, WORDSORT::WSORT_LEFT, RGB(208, 247, 252));
	uiToolTip->AddFrame(description);
	_toolTipFinalY += (item->GetDescription().length() / 22) * 10 + 70;

	if (_shopFrame->GetIsViewing())
	{
		UIFrame* moneyImg = new UIFrame();
		moneyImg->init("moneyImg", 352, _toolTipFinalY - 2, 19, 19, "moneyUI");
		uiToolTip->AddFrame(moneyImg);

		UIText* sellMoney = new UIText();
		sellMoney->init("sellMoney", 250, _toolTipFinalY, 100, 50, to_string_with_precision(item->GetSellPrice(), 0), FONT::PIX, WORDSIZE::WS_SMALL, WORDSORT::WSORT_RIGHT);
		uiToolTip->AddFrame(sellMoney);
	}

	_toolTipFinalY += 30;
	uiToolTip->SetScaleY(_toolTipFinalY / 100.f);
}

string Inventory::OptionString(SubOption* option)
{
	string optionResult = "";

	if (option->_optionPower != 0)
	{
		optionResult += (option->_optionPower >= 0 ? "+" : "") + to_string_with_precision(option->_optionPower, 0);

		switch (option->_optionId)
		{
		case POWER:
			optionResult += " 위력";
			break;
		case ATKSPEED:
			optionResult += "% 공격속도";
			break;
		case DASHATK:
			optionResult += "% 대쉬 공격력";
			break;
		case DEFENCE:
			optionResult += " 방어력";
			break;
		case BLOCK:
			optionResult += " 막기";
			break;
		case CRITICALPERCENT:
			optionResult += " 크리티컬";
			break;
		case CRITICALDAMAGE:
			optionResult += " 크리티컬 데미지";
			break;
		case MINDAMAGE:
			optionResult += " 최소 데미지";
			break;
		case MAXDAMAGE:
			optionResult += " 최대 데미지";
			break;
		case FINALDAMAGEPERCENT:
			optionResult += "% 최종 데미지";
			break;
		case FINALDAMAGE:
			optionResult += " 최종 데미지";
			break;
		case TOUGHNESS:
			optionResult += " 강인함";
			break;
		case TRUEDAMAGE:
			optionResult += " 고정 데미지";
			break;
		case MAXHP:
			optionResult += " 최대 체력";
			break;
		case MAXHPPERCENT:
			optionResult += "% 최대 체력";
			break;
		case EVADE:
			optionResult += " 회피";
			break;
		case MOVESPEED:
			optionResult += "% 이동속도";
			break;
		case JUMPPOWER:
			optionResult += " 점프력";
			break;
		case GOLDDROP:
			optionResult += "% 골드 드랍";
			break;
		case RELOADSPEED:
			optionResult += "% 재장전 속도";
			break;
		case DASHCOUNT:
			optionResult += " 대쉬 횟수";
			break;
		case ACCURACY:
			optionResult += " 조준 정확도";
			break;
		case FIREDAMAGE:
			optionResult += " 화상 피해";
			break;
		case ICEDAMAGE:
			optionResult += " 냉기 피해";
			break;
		case ELECDAMAGE:
			optionResult += " 감전 피해";
			break;
		case POSIONDAMAGE:
			optionResult += " 중독 피해";
			break;
		case STUNDAMAGE:
			optionResult += " 기절 피해";
		default:
			break;
		}
	}

	switch (option->_optionId)
	{
	case IMMUNEFIRE:
		optionResult += "화상 면역";
		break;
	case IMMUNEICE:
		optionResult += "냉기 면역";
		break;
	case IMMUNEELEC:
		optionResult += "감전 면역";
		break;
	case IMMUNEPOSION:
		optionResult += "중독 면역";
		break;
	case IMMUNESTUN:
		optionResult += "기절 면역";
		break;
	case TOFIRE:
		optionResult += "화상 부여";
		break;
	case TOICE:
		optionResult += "냉기 부여";
		break;
	case TOELEC:
		optionResult += "감전 부여";
		break;
	case TOPOSION:
		optionResult += "중독 부여";
		break;
	case TOSTUN:
		optionResult += "기절 부여";
	}

	if (option->_description != ".")
		optionResult += option->_description;

	return optionResult;
}

void Inventory::ShowToolTip()
{
	Item* item = nullptr;
	bool isEquipped = true;

	UIImage* uiToolTip = dynamic_cast<UIImage*>(UIMANAGER->GetGameFrame()->GetChild("InventoryFrame")->GetChild("itemToolTip"));

	for (int i = 0; i < _vInvenItems.size(); i++)
	{
		UIFrame* curFrame = _InvenFrame->GetChild("itemFrame_" + to_string(i));
		if (PtInRect(&curFrame->GetRect(), _ptMouse))
		{
			item = _vInvenItems[i];
			isEquipped = false;
			break;
		}
	}

	for (int i = 0; i < _p->GetVAccessories().size(); i++)
	{
		UIFrame* curFrame = _InvenFrame->GetChild("accesoryFrame_" + to_string(i));
		if (PtInRect(&curFrame->GetRect(), _ptMouse))
		{
			item = _p->GetAccessory(i);
			break;
		}
	}
	if (PtInRect(&_InvenFrame->GetChild("curWeapon_1")->GetRect(), _ptMouse))
	{
		if (_p->GetWeapon(0) != nullptr)
			item = _p->GetWeapon(0);
	}
	else if (PtInRect(&_InvenFrame->GetChild("curWeapon_2")->GetRect(), _ptMouse))
	{
		if (_p->GetWeapon(1) != nullptr)
			item = _p->GetWeapon(1);
	}
	else if (PtInRect(&_InvenFrame->GetChild("curWeaponSub_1")->GetRect(), _ptMouse))
	{
		if (_p->GetSubWeapon(0) != nullptr)
			item = _p->GetSubWeapon(0);
	}
	else if (PtInRect(&_InvenFrame->GetChild("curWeaponSub_2")->GetRect(), _ptMouse))
	{
		if (_p->GetSubWeapon(1) != nullptr)
			item = _p->GetSubWeapon(1);
	}

	if (item != nullptr)
	{
		if (item != _curToolTipItem)
		{
			InitToolTipItem(item);
		}

		uiToolTip->MoveFrameChild((_ptMouse.x - 300) - uiToolTip->GetX(), (_ptMouse.y - (isEquipped ? 0 : _toolTipFinalY)) - uiToolTip->GetY());
		uiToolTip->SetIsViewing(true);
	}

	else
	{
		uiToolTip->SetIsViewing(false);
	}
}