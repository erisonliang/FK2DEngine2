/**
*	created:		2013-4-13   20:49
*	filename: 		FKMenuStrip
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "../FK2DEngine2/Include/UnitTest/FKUnitTest.h"
#include "../FK2DEngine2/Include/Controls/FKMenuStrip.h"
//------------------------------------------------------------------------
using namespace FK2DEngine2;
//------------------------------------------------------------------------
class MenuStrip : public GUnit
{
public:

	FK_CONTROL_INLINE( MenuStrip, GUnit )
	{
		Dock( Pos::Fill );
		FK2DEngine2::Controls::MenuStrip* menu = new FK2DEngine2::Controls::MenuStrip( this );
		{
			FK2DEngine2::Controls::MenuItem* pRoot = menu->AddItem( L"File" );
			pRoot->GetMenu()->AddItem( L"New", L"test16.png", "Ctrl + N" )->SetAction( this, &ThisClass::MenuItemSelect );
			pRoot->GetMenu()->AddItem( L"Load", L"test16.png", "Ctrl+L" )->SetAction( this, &ThisClass::MenuItemSelect );
			pRoot->GetMenu()->AddItem( L"Save", "", "CTRL+S" )->SetAction( this, &ThisClass::MenuItemSelect );
			pRoot->GetMenu()->AddItem( L"Save As..", "", "Ctrl+Shift+S" )->SetAction( this, &ThisClass::MenuItemSelect );
			pRoot->GetMenu()->AddItem( L"Quit", "", "Ctrl+Q" )->SetAction( this, &ThisClass::MenuItemSelect );
		}
		{
			FK2DEngine2::Controls::MenuItem* pRoot = menu->AddItem( L"\u043F\u0438\u0440\u0430\u0442\u0441\u0442\u0432\u043E" );
			pRoot->GetMenu()->AddItem( L"\u5355\u5143\u6D4B\u8BD5" )->SetAction( this, &ThisClass::MenuItemSelect );
			pRoot->GetMenu()->AddItem( L"\u0111\u01A1n v\u1ECB th\u1EED nghi\u1EC7m" )->SetAction( this, &ThisClass::MenuItemSelect );
		}
		{
			FK2DEngine2::Controls::MenuItem* pRoot = menu->AddItem( L"Submenu" );
			FK2DEngine2::Controls::MenuItem* pCheckable = pRoot->GetMenu()->AddItem( "Checkable" );
			pCheckable->SetCheckable( true );
			pCheckable->SetChecked( true );
			{
				FK2DEngine2::Controls::MenuItem* pRootB = pRoot->GetMenu()->AddItem( "Two" );
				pRootB->GetMenu()->AddItem( "Two.One" );
				pRootB->GetMenu()->AddItem( "Two.Two" );
				pRootB->GetMenu()->AddItem( "Two.Three" );
				pRootB->GetMenu()->AddItem( "Two.Four" );
				pRootB->GetMenu()->AddItem( "Two.Five" );
				pRootB->GetMenu()->AddItem( "Two.Six" );
				pRootB->GetMenu()->AddItem( "Two.Seven" );
				pRootB->GetMenu()->AddItem( "Two.Eight" );
				pRootB->GetMenu()->AddItem( "Two.Nine", "test16.png" );
			}
			pRoot->GetMenu()->AddItem( "Three" );
			pRoot->GetMenu()->AddItem( "Four" );
			pRoot->GetMenu()->AddItem( "Five" );
			{
				FK2DEngine2::Controls::MenuItem* pRootB = pRoot->GetMenu()->AddItem( "Six" );
				pRootB->GetMenu()->AddItem( "Six.One" );
				pRootB->GetMenu()->AddItem( "Six.Two" );
				pRootB->GetMenu()->AddItem( "Six.Three" );
				pRootB->GetMenu()->AddItem( "Six.Four" );
				pRootB->GetMenu()->AddItem( "Six.Five", "test16.png" );
				{
					FK2DEngine2::Controls::MenuItem* pRootC = pRootB->GetMenu()->AddItem( "Six.Six" );
					pRootC->GetMenu()->AddItem( "Sheep" );
					pRootC->GetMenu()->AddItem( "Goose" );
					{
						FK2DEngine2::Controls::MenuItem* pRootD = pRootC->GetMenu()->AddItem( "Camel" );
						pRootD->GetMenu()->AddItem( "Eyes" );
						pRootD->GetMenu()->AddItem( "Nose" );
						{
							FK2DEngine2::Controls::MenuItem* pRootE = pRootD->GetMenu()->AddItem( "Hair" );
							pRootE->GetMenu()->AddItem( "Blonde" );
							pRootE->GetMenu()->AddItem( "Black" );
							{
								FK2DEngine2::Controls::MenuItem* pRootF = pRootE->GetMenu()->AddItem( "Red" );
								pRootF->GetMenu()->AddItem( "Light" );
								pRootF->GetMenu()->AddItem( "Medium" );
								pRootF->GetMenu()->AddItem( "Dark" );
							}
							pRootE->GetMenu()->AddItem( "Brown" );
						}
						pRootD->GetMenu()->AddItem( "Ears" );
					}
					pRootC->GetMenu()->AddItem( "Duck" );
				}
				pRootB->GetMenu()->AddItem( "Six.Seven" );
				pRootB->GetMenu()->AddItem( "Six.Eight" );
				pRootB->GetMenu()->AddItem( "Six.Nine" );
			}
			pRoot->GetMenu()->AddItem( "Seven" );
		}
	}


	void MenuItemSelect( Base* pControl )
	{
		FK2DEngine2::Controls::MenuItem* pMenuItem = ( FK2DEngine2::Controls::MenuItem* ) pControl;
		UnitPrint( Utility::Format( L"Menu Selected: %ls", pMenuItem->GetText().GetUnicode().c_str() ) );
	}

};
//------------------------------------------------------------------------
DEFINE_UNIT_TEST( MenuStrip, L"MenuStrip" );
//------------------------------------------------------------------------