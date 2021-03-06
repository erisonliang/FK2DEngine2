/**
*	created:		2013-4-11   22:40
*	filename: 		FKControlFactory
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "../../Include/Util/FKControlFactory.h"
//------------------------------------------------------------------------
namespace FK2DEngine2
{
	namespace ControlFactory
	{
		const FK2DEngine2::UnicodeString PropertyBool::True	= L"true";
		const FK2DEngine2::UnicodeString PropertyBool::False	= L"false";

		void InitializeControls()
		{
			DECLARE_FK_CONTROL_FACTORY( Base_Factory );
			DECLARE_FK_CONTROL_FACTORY( Label_Factory );
			DECLARE_FK_CONTROL_FACTORY( LabelClickable_Factory );
			DECLARE_FK_CONTROL_FACTORY( Button_Factory );
			DECLARE_FK_CONTROL_FACTORY( TextBox_Factory );
			DECLARE_FK_CONTROL_FACTORY( TextBoxMultiLine_Factory );
			DECLARE_FK_CONTROL_FACTORY( CheckBox_Factory );
			DECLARE_FK_CONTROL_FACTORY( CheckBoxWithLabel_Factory );
			DECLARE_FK_CONTROL_FACTORY( MenuStrip_Factory );
			DECLARE_FK_CONTROL_FACTORY( ListBox_Factory );
			DECLARE_FK_CONTROL_FACTORY( VerticalSplitter_Factory );
			DECLARE_FK_CONTROL_FACTORY( Image_Factory );
			DECLARE_FK_CONTROL_FACTORY( FilePicker_Factory );
			DECLARE_FK_CONTROL_FACTORY( FolderPicker_Factory );
			DECLARE_FK_CONTROL_FACTORY( PageControl_Factory );
			DECLARE_FK_CONTROL_FACTORY( Rectangle_Factory );
			DECLARE_FK_CONTROL_FACTORY( ProgressBar_Factory );
			DECLARE_FK_CONTROL_FACTORY( ComboBox_Factory );
			DECLARE_FK_CONTROL_FACTORY( HorizontalSlider_Factory );
			DECLARE_FK_CONTROL_FACTORY( DesignerCanvas_Factory );
		}


		List & GetList()
		{
			static List list;
			static bool Initialized = false;

			if ( !Initialized )
			{
				Initialized = true;
				InitializeControls();
			}

			return list;
		}

		ControlFactory::Base* Find( const FK2DEngine2::String & name )
		{
			for ( ControlFactory::List::iterator it = ControlFactory::GetList().begin(); it != ControlFactory::GetList().end(); ++it )
			{
				if ( ( *it )->Name() == name )
				{
					return *it;
				}
			}

			return NULL;
		}

		Base::Base()
		{
			GetList().push_back( this );
		}

		void Base::AddProperty( Property* pProp )
		{
			m_Properties.push_back( pProp );
		}

		Base* Base::GetBaseFactory()
		{
			for ( ControlFactory::List::iterator it = ControlFactory::GetList().begin(); it != ControlFactory::GetList().end(); ++it )
			{
				if ( ( *it )->Name() == BaseName() )
				{
					return *it;
				}
			}

			return NULL;
		}

		Property* Base::GetProperty( const FK2DEngine2::String & name )
		{
			for ( ControlFactory::Property::List::const_iterator it = Properties().begin(), itEnd = Properties().end();
				it != itEnd; ++it )
			{
				if ( ( *it )->Name() != name ) { continue; }

				return *it;
			}

			Base* pBase = GetBaseFactory();

			if ( !pBase ) { return NULL; }

			return pBase->GetProperty( name );
		}

		void Base::SetControlValue( FK2DEngine2::Controls::Base* ctrl, const FK2DEngine2::String & name, const FK2DEngine2::UnicodeString & str )
		{
			Property* pProp = GetProperty( name );

			if ( !pProp )
			{
				Base* pBase = GetBaseFactory();

				if ( !pBase ) { return; }

				return pBase->SetControlValue( ctrl, name, str );
			}

			pProp->SetValue( ctrl, str );
		}

		void Base::AddChild( FK2DEngine2::Controls::Base* ctrl, FK2DEngine2::Controls::Base* child, const FK2DEngine2::Point & pos )
		{
			child->SetParent( ctrl );
		}

		void Base::AddChild( FK2DEngine2::Controls::Base* ctrl, FK2DEngine2::Controls::Base* child, int iPage )
		{
			child->SetParent( ctrl );
		}

		Controls::Base* Clone( Controls::Base* pSource, ControlFactory::Base* pFactory )
		{
			Controls::Base* pControl = pFactory->CreateInstance( pSource->GetParent() );

			while ( pFactory )
			{
				for ( ControlFactory::Property::List::const_iterator
					it = pFactory->Properties().begin(), itEnd = pFactory->Properties().end();
					it != itEnd; ++it )
				{
					( *it )->SetValue( pControl, ( *it )->GetValue( pSource ) );
				}

				pFactory = pFactory->GetBaseFactory();
			}

			return pControl;
		}

		void Base::SetParentPage( FK2DEngine2::Controls::Base* ctrl, int i )
		{
			ctrl->UserData.Set<int> ( "ParentPage", i );
		}

		int Base::GetParentPage( FK2DEngine2::Controls::Base* ctrl )
		{
			if ( !ctrl->UserData.Exists( "ParentPage" ) ) { return 0; }

			return ctrl->UserData.Get<int> ( "ParentPage" );
		}

	}
}
//------------------------------------------------------------------------