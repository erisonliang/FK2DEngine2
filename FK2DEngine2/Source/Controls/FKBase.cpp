/**
*	created:		2013-4-13   21:33
*	filename: 		FKBase
*	author:			FreeKnight
*	Copyright (C): 	
*	purpose:		
*/
//------------------------------------------------------------------------
#include "../../Include/Controls/FKBase.h"
#include "../../Include/Controls/FKLabel.h"
#include "../../FK2DEngine2.h"
#include "../../Include/FKBaseRender.h"
#include "../../Include/FKSkin.h"
#include "../../Include/FKPlatform.h"
#include "../../Include/FKDragAndDrop.h"
#include "../../Include/FKToolTip.h"
#include "../../Include/FKUtility.h"
#include <list>
//------------------------------------------------------------------------
#ifndef FK_NO_ANIMATION
#include "../../Include/FKAnim.h"
#endif
//------------------------------------------------------------------------
using namespace FK2DEngine2;
using namespace Controls;
//------------------------------------------------------------------------
Base::Base( Base* pParent, const FK2DEngine2::String & Name )
{
	m_Parent = NULL;
	m_ActualParent = NULL;
	m_InnerPanel = NULL;
	m_Skin = NULL;
	SetName( Name );
	SetParent( pParent );
	m_bHidden = false;
	m_Bounds = FK2DEngine2::Rect( 0, 0, 10, 10 );
	m_Padding = Padding( 0, 0, 0, 0 );
	m_Margin = Margin( 0, 0, 0, 0 );
	m_iDock = 0;
	m_DragAndDrop_Package = NULL;
	RestrictToParent( false );
	SetMouseInputEnabled( true );
	SetKeyboardInputEnabled( false );
	Invalidate();
	SetCursor( FK2DEngine2::CursorType::Normal );
	SetToolTip( NULL );
	SetTabable( false );
	SetShouldDrawBackground( true );
	m_bDisabled = false;
	m_bCacheTextureDirty = true;
	m_bCacheToTexture = false;
	m_bIncludeInSize = true;
}

Base::~Base()
{
	{
		Canvas* canvas = GetCanvas();

		if ( canvas )
		{ canvas->PreDeleteCanvas( this ); }
	}
	Base::List::iterator iter = Children.begin();

	while ( iter != Children.end() )
	{
		Base* pChild = *iter;
		iter = Children.erase( iter );
		delete pChild;
	}

	for ( AccelMap::iterator accelIt = m_Accelerators.begin(); accelIt != m_Accelerators.end(); ++accelIt )
	{
		delete accelIt->second;
	}

	m_Accelerators.clear();
	SetParent( NULL );

	if ( FK2DEngine2::HoveredControl == this ) { FK2DEngine2::HoveredControl = NULL; }

	if ( FK2DEngine2::KeyboardFocus == this ) { FK2DEngine2::KeyboardFocus = NULL; }

	if ( FK2DEngine2::MouseFocus == this ) { FK2DEngine2::MouseFocus = NULL; }

	DragAndDrop::ControlDeleted( this );
	ToolTip::ControlDeleted( this );
#ifndef FK_NO_ANIMATION
	Anim::Cancel( this );
#endif

	if ( m_DragAndDrop_Package )
	{
		delete m_DragAndDrop_Package;
		m_DragAndDrop_Package = NULL;
	}
}

void Base::Invalidate()
{
	m_bNeedsLayout = true;
	m_bCacheTextureDirty = true;
}

void Base::DelayedDelete()
{
	Canvas* canvas = GetCanvas();
	canvas->AddDelayedDelete( this );
}

Canvas* Base::GetCanvas()
{
	Base* pCanvas = m_Parent;

	if ( !pCanvas ) { return NULL; }

	return pCanvas->GetCanvas();
}

void Base::SetParent( Base* pParent )
{
	if ( m_Parent == pParent ) { return; }

	if ( m_Parent )
	{
		m_Parent->RemoveChild( this );
	}

	m_Parent = pParent;
	m_ActualParent = NULL;

	if ( m_Parent )
	{
		m_Parent->AddChild( this );
	}
}

void Base::Dock( int iDock )
{
	if ( m_iDock == iDock ) { return; }

	m_iDock = iDock;
	Invalidate();
	InvalidateParent();
}

int Base::GetDock()
{
	return m_iDock;
}

bool Base::Hidden() const
{
	return m_bHidden;
}

bool Base::Visible() const
{
	if ( Hidden() ) { return false; }

	if ( GetParent() )
	{
		return GetParent()->Visible();
	}

	return true;
}

void Base::InvalidateChildren( bool bRecursive )
{
	for ( Base::List::iterator it = Children.begin(); it != Children.end(); ++it )
	{
		( *it )->Invalidate();

		if ( bRecursive )
		{ ( *it )->InvalidateChildren( bRecursive ); }
	}

	if ( m_InnerPanel )
	{
		for ( Base::List::iterator it = m_InnerPanel->Children.begin(); it != m_InnerPanel->Children.end(); ++it )
		{
			( *it )->Invalidate();

			if ( bRecursive )
			{ ( *it )->InvalidateChildren( bRecursive ); }
		}
	}
}

void Base::Position( int pos, int xpadding, int ypadding )
{
	const Rect & bounds = GetParent()->GetInnerBounds();
	const Margin & margin = GetMargin();
	int x = X();
	int y = Y();

	if ( pos & Pos::Left ) { x = bounds.x + xpadding + margin.left; }

	if ( pos & Pos::Right ) { x = bounds.x + ( bounds.w - Width() - xpadding - margin.right ); }

	if ( pos & Pos::CenterH ) { x = bounds.x + ( bounds.w - Width() )  * 0.5; }

	if ( pos & Pos::Top ) { y = bounds.y + ypadding; }

	if ( pos & Pos::Bottom ) { y = bounds.y + ( bounds.h - Height() - ypadding ); }

	if ( pos & Pos::CenterV ) { y = bounds.y + ( bounds.h - Height() )  * 0.5; }

	SetPos( x, y );
}

void Base::SendToBack()
{
	if ( !m_ActualParent ) { return; }

	if ( m_ActualParent->Children.front() == this ) { return; }

	m_ActualParent->Children.remove( this );
	m_ActualParent->Children.push_front( this );
	InvalidateParent();
}

void Base::BringToFront()
{
	if ( !m_ActualParent ) { return; }

	if ( m_ActualParent->Children.back() == this ) { return; }

	m_ActualParent->Children.remove( this );
	m_ActualParent->Children.push_back( this );
	InvalidateParent();
	Redraw();
}

Controls::Base* Base::FindChildByName( const FK2DEngine2::String & name, bool bRecursive )
{
	Base::List::iterator iter;

	for ( iter = Children.begin(); iter != Children.end(); ++iter )
	{
		Base* pChild = *iter;

		if ( !pChild->GetName().empty() && pChild->GetName() == name )
		{ return pChild; }

		if ( bRecursive )
		{
			Controls::Base* pSubChild = pChild->FindChildByName( name, true );

			if ( pSubChild )
			{ return pSubChild; }
		}
	}

	return NULL;
}

void Base::BringNextToControl( Controls::Base* pChild, bool bBehind )
{
	if ( !m_ActualParent ) { return; }

	m_ActualParent->Children.remove( this );
	Base::List::iterator it = std::find( m_ActualParent->Children.begin(), m_ActualParent->Children.end(), pChild );

	if ( it == m_ActualParent->Children.end() )
	{ return BringToFront(); }

	if ( bBehind )
	{
		++it;

		if ( it == m_ActualParent->Children.end() )
		{ return BringToFront(); }
	}

	m_ActualParent->Children.insert( it, this );
	InvalidateParent();
}

void Base::AddChild( Base* pChild )
{
	if ( m_InnerPanel )
	{
		m_InnerPanel->AddChild( pChild );
		return;
	}

	Children.push_back( pChild );
	OnChildAdded( pChild );
	pChild->m_ActualParent = this;
}
void Base::RemoveChild( Base* pChild )
{
	if ( m_InnerPanel == pChild )
	{
		m_InnerPanel = NULL;
	}

	if ( m_InnerPanel )
	{
		m_InnerPanel->RemoveChild( pChild );
	}

	Children.remove( pChild );
	OnChildRemoved( pChild );
}

void Base::RemoveAllChildren()
{
	while ( Children.size() > 0 )
	{
		RemoveChild( *Children.begin() );
	}
}

unsigned int Base::NumChildren()
{
	return Children.size();
}

Controls::Base* Base::GetChild( unsigned int i )
{
	if ( i >= NumChildren() ) { return NULL; }

	for ( Base::List::iterator iter = Children.begin(); iter != Children.end(); ++iter )
	{
		if ( i == 0 )
		{ return *iter; }

		i--;
	}

	return NULL;
}

void Base::OnChildAdded( Base* /*pChild*/ )
{
	Invalidate();
}

void Base::OnChildRemoved( Base* /*pChild*/ )
{
	Invalidate();
}

Skin::Base* Base::GetSkin( void )
{
	if ( m_Skin ) { return m_Skin; }

	if ( m_Parent ) { return m_Parent->GetSkin(); }

	Debug::AssertCheck( 0, "Base::GetSkin Returning NULL!\n" );
	return NULL;
}

void Base::MoveBy( int x, int y )
{
	MoveTo( X() + x, Y() + y );
}

void Base::MoveTo( int x, int y )
{
	if ( m_bRestrictToParent && GetParent() )
	{
		Base* pParent = GetParent();

		if ( x - GetPadding().left < pParent->GetMargin().left )	{ x = pParent->GetMargin().left + GetPadding().left; }

		if ( y - GetPadding().top < pParent->GetMargin().top ) { y = pParent->GetMargin().top + GetPadding().top; }

		if ( x + Width() + GetPadding().right > pParent->Width() - pParent->GetMargin().right ) { x = pParent->Width() - pParent->GetMargin().right - Width() - GetPadding().right; }

		if ( y + Height() + GetPadding().bottom > pParent->Height() - pParent->GetMargin().bottom ) { y = pParent->Height() - pParent->GetMargin().bottom - Height() - GetPadding().bottom; }
	}

	SetBounds( x, y, Width(), Height() );
}

void Base::SetPos( int x, int y )
{
	SetBounds( x, y, Width(), Height() );
}

bool Base::SetSize( int w, int h )
{
	return SetBounds( X(), Y(), w, h );
}

bool Base::SetSize( const Point & p )
{
	return SetSize( p.x, p.y );
}

bool Base::SetBounds( const FK2DEngine2::Rect & bounds )
{
	return SetBounds( bounds.x, bounds.y, bounds.w, bounds.h );
}

bool Base::SetBounds( int x, int y, int w, int h )
{
	if ( m_Bounds.x == x &&
			m_Bounds.y == y &&
			m_Bounds.w == w &&
			m_Bounds.h == h )
	{ return false; }

	FK2DEngine2::Rect oldBounds = GetBounds();
	m_Bounds.x = x;
	m_Bounds.y = y;
	m_Bounds.w = w;
	m_Bounds.h = h;
	OnBoundsChanged( oldBounds );
	return true;
}

void Base::OnBoundsChanged( FK2DEngine2::Rect oldBounds )
{
	if ( GetParent() )
	{ GetParent()->OnChildBoundsChanged( oldBounds, this ); }

	if ( m_Bounds.w != oldBounds.w || m_Bounds.h != oldBounds.h )
	{
		Invalidate();
	}

	Redraw();
	UpdateRenderBounds();
}

void Base::OnScaleChanged()
{
	for ( Base::List::iterator iter = Children.begin(); iter != Children.end(); ++iter )
	{
		( *iter )->OnScaleChanged();
	}
}

void Base::OnChildBoundsChanged( FK2DEngine2::Rect /*oldChildBounds*/, Base* /*pChild*/ )
{
}

void Base::Render( FK2DEngine2::Skin::Base* /*skin*/ )
{
}

void Base::DoCacheRender( FK2DEngine2::Skin::Base* skin, FK2DEngine2::Controls::Base* pMaster )
{
	FK2DEngine2::Renderer::Base* render = skin->GetRender();
	FK2DEngine2::Renderer::ICacheToTexture* cache = render->GetCTT();

	if ( !cache ) { return; }

	FK2DEngine2::Point pOldRenderOffset = render->GetRenderOffset();
	FK2DEngine2::Rect rOldRegion = render->ClipRegion();

	if ( this != pMaster )
	{
		render->AddRenderOffset( GetBounds() );
		render->AddClipRegion( GetBounds() );
	}
	else
	{
		render->SetRenderOffset( FK2DEngine2::Point( 0, 0 ) );
		render->SetClipRegion( GetBounds() );
	}

	if ( m_bCacheTextureDirty && render->ClipRegionVisible() )
	{
		render->StartClip();
		{
			if ( ShouldCacheToTexture() )
			{ cache->SetupCacheTexture( this ); }

			Render( skin );

			if ( !Children.empty() )
			{
				for ( Base::List::iterator iter = Children.begin(); iter != Children.end(); ++iter )
				{
					Base* pChild = *iter;

					if ( pChild->Hidden() ) { continue; }

					pChild->DoCacheRender( skin, pMaster );
				}
			}

			if ( ShouldCacheToTexture() )
			{
				cache->FinishCacheTexture( this );
				m_bCacheTextureDirty = false;
			}
		}
		render->EndClip();
	}

	render->SetClipRegion( rOldRegion );
	render->StartClip();
	{
		render->SetRenderOffset( pOldRenderOffset );
		cache->DrawCachedControlTexture( this );
	}
	render->EndClip();
}

void Base::DoRender( FK2DEngine2::Skin::Base* skin )
{
	if ( m_Skin )
	{ skin = m_Skin; }

	Think();
	FK2DEngine2::Renderer::Base* render = skin->GetRender();

	if ( render->GetCTT() && ShouldCacheToTexture() )
	{
		DoCacheRender( skin, this );
		return;
	}

	RenderRecursive( skin, GetBounds() );
}

void Base::RenderRecursive( FK2DEngine2::Skin::Base* skin, const FK2DEngine2::Rect & cliprect )
{
	FK2DEngine2::Renderer::Base* render = skin->GetRender();
	FK2DEngine2::Point pOldRenderOffset = render->GetRenderOffset();
	render->AddRenderOffset( cliprect );
	RenderUnder( skin );
	FK2DEngine2::Rect rOldRegion = render->ClipRegion();

	if ( ShouldClip() )
	{
		render->AddClipRegion( cliprect );

		if ( !render->ClipRegionVisible() )
		{
			render->SetRenderOffset( pOldRenderOffset );
			render->SetClipRegion( rOldRegion );
			return;
		}
	}

	render->StartClip();
	{
		Render( skin );

		if ( !Children.empty() )
		{
			for ( Base::List::iterator iter = Children.begin(); iter != Children.end(); ++iter )
			{
				Base* pChild = *iter;

				if ( pChild->Hidden() ) { continue; }

				pChild->DoRender( skin );
			}
		}
	}
	render->EndClip();

	{
		render->SetClipRegion( rOldRegion );
		render->StartClip();
		{
			RenderOver( skin );
			RenderFocus( skin );
		}
		render->EndClip();
		render->SetRenderOffset( pOldRenderOffset );
	}
}

void Base::SetSkin( Skin::Base* skin, bool doChildren )
{
	if ( m_Skin == skin ) { return; }

	m_Skin = skin;
	Invalidate();
	Redraw();
	OnSkinChanged( skin );

	if ( doChildren )
	{
		for ( Base::List::iterator it = Children.begin(); it != Children.end(); ++it )
		{
			( *it )->SetSkin( skin, true );
		}
	}
}

void Base::OnSkinChanged( Skin::Base* /*skin*/ )
{
	// TODO
}

bool Base::OnMouseWheeled( int iDelta )
{
	if ( m_ActualParent )
	{ return m_ActualParent->OnMouseWheeled( iDelta ); }

	return false;
}

void Base::OnMouseMoved( int /*x*/, int /*y*/, int /*deltaX*/, int /*deltaY*/ )
{
}

void Base::OnMouseEnter()
{
	onHoverEnter.Call( this );

	if ( GetToolTip() )
	{ ToolTip::Enable( this ); }
	else if ( GetParent() && GetParent()->GetToolTip() )
	{ ToolTip::Enable( GetParent() ); }

	Redraw();
}

void Base::OnMouseLeave()
{
	onHoverLeave.Call( this );

	if ( GetToolTip() )
	{ ToolTip::Disable( this ); }

	Redraw();
}


bool Base::IsHovered()
{
	return FK2DEngine2::HoveredControl == this;
}

bool Base::ShouldDrawHover()
{
	return FK2DEngine2::MouseFocus == this || FK2DEngine2::MouseFocus == NULL;
}

bool Base::HasFocus()
{
	return FK2DEngine2::KeyboardFocus == this;
}

void Base::Focus()
{
	if ( FK2DEngine2::KeyboardFocus == this ) { return; }

	if ( FK2DEngine2::KeyboardFocus )
	{ FK2DEngine2::KeyboardFocus->OnLostKeyboardFocus(); }

	FK2DEngine2::KeyboardFocus = this;
	OnKeyboardFocus();
	Redraw();
}

void Base::Blur()
{
	if ( FK2DEngine2::KeyboardFocus != this ) { return; }

	FK2DEngine2::KeyboardFocus = NULL;
	OnLostKeyboardFocus();
	Redraw();
}

bool Base::IsOnTop()
{
	if ( !GetParent() )
	{ return false; }

	Base::List::iterator iter = GetParent()->Children.begin();
	Base* pChild = *iter;

	if ( pChild == this )
	{ return true; }

	return false;
}


void Base::Touch()
{
	if ( GetParent() )
	{ GetParent()->OnChildTouched( this ); }
}

void Base::OnChildTouched( Controls::Base* /*pChild*/ )
{
	Touch();
}

Base* Base::GetControlAt( int x, int y, bool bOnlyIfMouseEnabled )
{
	if ( Hidden() )
	{ return NULL; }

	if ( x < 0 || y < 0 || x >= Width() || y >= Height() )
	{ return NULL; }

	Base::List::reverse_iterator iter;

	for ( iter = Children.rbegin(); iter != Children.rend(); ++iter )
	{
		Base* pChild = *iter;
		Base* pFound = NULL;
		pFound = pChild->GetControlAt( x - pChild->X(), y - pChild->Y(), bOnlyIfMouseEnabled );

		if ( pFound ) { return pFound; }
	}

	if ( bOnlyIfMouseEnabled && !GetMouseInputEnabled() )
	{ return NULL; }

	return this;
}


void Base::Layout( Skin::Base* skin )
{
	if ( skin->GetRender()->GetCTT() && ShouldCacheToTexture() )
	{ skin->GetRender()->GetCTT()->CreateControlCacheTexture( this ); }
}

void Base::RecurseLayout( Skin::Base* skin )
{
	if ( m_Skin ) { skin = m_Skin; }

	if ( Hidden() ) { return; }

	if ( NeedsLayout() )
	{
		m_bNeedsLayout = false;
		Layout( skin );
	}

	FK2DEngine2::Rect rBounds = GetRenderBounds();

	rBounds.x += m_Padding.left;
	rBounds.w -= m_Padding.left + m_Padding.right;
	rBounds.y += m_Padding.top;
	rBounds.h -= m_Padding.top + m_Padding.bottom;

	for ( Base::List::iterator iter = Children.begin(); iter != Children.end(); ++iter )
	{
		Base* pChild = *iter;

		if ( pChild->Hidden() )
		{ continue; }

		int iDock = pChild->GetDock();

		if ( iDock & Pos::Fill )
		{ continue; }

		if ( iDock & Pos::Top )
		{
			const Margin & margin = pChild->GetMargin();
			pChild->SetBounds( rBounds.x + margin.left, rBounds.y + margin.top, rBounds.w - margin.left - margin.right, pChild->Height() );
			int iHeight = margin.top + margin.bottom + pChild->Height();
			rBounds.y += iHeight;
			rBounds.h -= iHeight;
		}

		if ( iDock & Pos::Left )
		{
			const Margin & margin = pChild->GetMargin();
			pChild->SetBounds( rBounds.x + margin.left, rBounds.y + margin.top, pChild->Width(), rBounds.h - margin.top - margin.bottom );
			int iWidth = margin.left + margin.right + pChild->Width();
			rBounds.x += iWidth;
			rBounds.w -= iWidth;
		}

		if ( iDock & Pos::Right )
		{
			// TODO: 
			const Margin & margin = pChild->GetMargin();
			pChild->SetBounds( ( rBounds.x + rBounds.w ) - pChild->Width() - margin.right, rBounds.y + margin.top, pChild->Width(), rBounds.h - margin.top - margin.bottom );
			int iWidth = margin.left + margin.right + pChild->Width();
			rBounds.w -= iWidth;
		}

		if ( iDock & Pos::Bottom )
		{
			// TODO: 
			const Margin & margin = pChild->GetMargin();
			pChild->SetBounds( rBounds.x + margin.left, ( rBounds.y + rBounds.h ) - pChild->Height() - margin.bottom, rBounds.w - margin.left - margin.right, pChild->Height() );
			rBounds.h -= pChild->Height() + margin.bottom + margin.top;
		}

		pChild->RecurseLayout( skin );
	}

	m_InnerBounds = rBounds;

	for ( Base::List::iterator iter = Children.begin(); iter != Children.end(); ++iter )
	{
		Base* pChild = *iter;
		int iDock = pChild->GetDock();

		if ( !( iDock & Pos::Fill ) )
		{ continue; }

		const Margin & margin = pChild->GetMargin();
		pChild->SetBounds( rBounds.x + margin.left, rBounds.y + margin.top, rBounds.w - margin.left - margin.right, rBounds.h - margin.top - margin.bottom );
		pChild->RecurseLayout( skin );
	}

	PostLayout( skin );

	if ( IsTabable() && !IsDisabled() )
	{
		if ( !GetCanvas()->FirstTab ) { GetCanvas()->FirstTab = this; }

		if ( !GetCanvas()->NextTab ) { GetCanvas()->NextTab = this; }
	}

	if ( FK2DEngine2::KeyboardFocus == this )
	{
		GetCanvas()->NextTab = NULL;
	}
}

bool Base::IsChild( Controls::Base* pChild )
{
	for ( Base::List::iterator iter = Children.begin(); iter != Children.end(); ++iter )
	{
		if ( pChild == ( *iter ) ) { return true; }
	}

	return false;
}

FK2DEngine2::Point Base::LocalPosToCanvas( const FK2DEngine2::Point & pnt )
{
	if ( m_Parent )
	{
		int x = pnt.x + X();
		int y = pnt.y + Y();

		if ( m_Parent->m_InnerPanel && m_Parent->m_InnerPanel->IsChild( this ) )
		{
			x += m_Parent->m_InnerPanel->X();
			y += m_Parent->m_InnerPanel->Y();
		}

		return m_Parent->LocalPosToCanvas( FK2DEngine2::Point( x, y ) );
	}

	return pnt;
}

FK2DEngine2::Point Base::CanvasPosToLocal( const FK2DEngine2::Point & pnt )
{
	if ( m_Parent )
	{
		int x = pnt.x - X();
		int y = pnt.y - Y();

		if ( m_Parent->m_InnerPanel && m_Parent->m_InnerPanel->IsChild( this ) )
		{
			x -= m_Parent->m_InnerPanel->X();
			y -= m_Parent->m_InnerPanel->Y();
		}

		return m_Parent->CanvasPosToLocal( FK2DEngine2::Point( x, y ) );
	}

	return pnt;
}

bool Base::IsMenuComponent()
{
	if ( !m_Parent ) { return false; }

	return m_Parent->IsMenuComponent();
}

void Base::CloseMenus()
{
	for ( Base::List::iterator it = Children.begin(); it != Children.end(); ++it )
	{
		( *it )->CloseMenus();
	}
}

void Base::UpdateRenderBounds()
{
	m_RenderBounds.x = 0;
	m_RenderBounds.y = 0;
	m_RenderBounds.w = m_Bounds.w;
	m_RenderBounds.h = m_Bounds.h;
}

void Base::UpdateCursor()
{
	Platform::SetCursor( m_Cursor );
}

DragAndDrop::Package* Base::DragAndDrop_GetPackage( int /*x*/, int /*y*/ )
{
	return m_DragAndDrop_Package;
}

bool Base::DragAndDrop_HandleDrop( FK2DEngine2::DragAndDrop::Package* /*pPackage*/, int /*x*/, int /*y*/ )
{
	DragAndDrop::SourceControl->SetParent( this );
	return true;
}

bool Base::DragAndDrop_Draggable()
{
	if ( !m_DragAndDrop_Package ) { return false; }

	return m_DragAndDrop_Package->draggable;
}

void Base::DragAndDrop_SetPackage( bool bDraggable, const String & strName, void* pUserData )
{
	if ( !m_DragAndDrop_Package )
	{
		m_DragAndDrop_Package = new FK2DEngine2::DragAndDrop::Package();
	}

	m_DragAndDrop_Package->draggable = bDraggable;
	m_DragAndDrop_Package->name = strName;
	m_DragAndDrop_Package->userdata = pUserData;
}

void Base::DragAndDrop_StartDragging( FK2DEngine2::DragAndDrop::Package* pPackage, int x, int y )
{
	pPackage->holdoffset = CanvasPosToLocal( FK2DEngine2::Point( x, y ) );
	pPackage->drawcontrol = this;
}

bool Base::SizeToChildren( bool w, bool h )
{
	FK2DEngine2::Point size = ChildrenSize();
	size.y += GetPadding().bottom;
	size.x += GetPadding().right;
	return SetSize( w ? size.x : Width(), h ? size.y : Height() );
}

FK2DEngine2::Point Base::ChildrenSize()
{
	FK2DEngine2::Point size;

	for ( Base::List::iterator iter = Children.begin(); iter != Children.end(); ++iter )
	{
		Base* pChild = *iter;

		if ( pChild->Hidden() ) { continue; }

		if ( !pChild->ShouldIncludeInSize() ) { continue; }

		size.x = FK2DEngine2::Max( size.x, pChild->Right() );
		size.y = FK2DEngine2::Max( size.y, pChild->Bottom() );
	}

	return size;
}

void Base::SetPadding( const Padding & padding )
{
	if ( m_Padding.left == padding.left &&
			m_Padding.top == padding.top &&
			m_Padding.right == padding.right &&
			m_Padding.bottom == padding.bottom )
	{ return; }

	m_Padding = padding;
	Invalidate();
	InvalidateParent();
}

void Base::SetMargin( const Margin & margin )
{
	if ( m_Margin.top == margin.top &&
			m_Margin.left == margin.left &&
			m_Margin.bottom == margin.bottom &&
			m_Margin.right == margin.right )
	{ return; }

	m_Margin = margin;
	Invalidate();
	InvalidateParent();
}

bool Base::HandleAccelerator( FK2DEngine2::UnicodeString & accelerator )
{
	if ( FK2DEngine2::KeyboardFocus == this || !AccelOnlyFocus() )
	{
		AccelMap::iterator iter = m_Accelerators.find( accelerator );

		if ( iter != m_Accelerators.end() )
		{
			iter->second->Call( this );
			return true;
		}
	}

	for ( Base::List::iterator it = Children.begin(); it != Children.end(); ++it )
	{
		if ( ( *it )->HandleAccelerator( accelerator ) )
		{ return true; }
	}

	return false;
}

bool Base::OnKeyPress( int iKey, bool bPress )
{
	bool bHandled = false;

	switch ( iKey )
	{
		case Key::Tab:
			bHandled = OnKeyTab( bPress );
			break;

		case Key::Space:
			bHandled = OnKeySpace( bPress );
			break;

		case Key::Home:
			bHandled = OnKeyHome( bPress );
			break;

		case Key::End:
			bHandled = OnKeyEnd( bPress );
			break;

		case Key::Return:
			bHandled = OnKeyReturn( bPress );
			break;

		case Key::Backspace:
			bHandled = OnKeyBackspace( bPress );
			break;

		case Key::Delete:
			bHandled = OnKeyDelete( bPress );
			break;

		case Key::Right:
			bHandled = OnKeyRight( bPress );
			break;

		case Key::Left:
			bHandled = OnKeyLeft( bPress );
			break;

		case Key::Up:
			bHandled = OnKeyUp( bPress );
			break;

		case Key::Down:
			bHandled = OnKeyDown( bPress );
			break;

		case Key::Escape:
			bHandled = OnKeyEscape( bPress );
			break;

		default:
			break;
	}

	if ( !bHandled && GetParent() )
	{ GetParent()->OnKeyPress( iKey, bPress ); }

	return bHandled;
}

bool Base::OnKeyRelease( int iKey )
{
	return OnKeyPress( iKey, false );
}

bool Base::OnKeyTab( bool bDown )
{
	if ( !bDown ) { return true; }

	if ( GetCanvas()->NextTab )
	{
		GetCanvas()->NextTab->Focus();
		Redraw();
	}

	return true;
}

void Base::RenderFocus( FK2DEngine2::Skin::Base* skin )
{
	if ( FK2DEngine2::KeyboardFocus != this ) { return; }

	if ( !IsTabable() ) { return; }

	skin->DrawKeyboardHighlight( this, GetRenderBounds(), 3 );
}

void Base::SetToolTip( const TextObject & strText )
{
	Label* tooltip = new Label( this );
	tooltip->SetText( strText );
	tooltip->SetTextColorOverride( GetSkin()->Colors.TooltipText );
	tooltip->SetPadding( Padding( 5, 3, 5, 3 ) );
	tooltip->SizeToContents();
	SetToolTip( tooltip );
}

TextObject Base::GetChildValue( const FK2DEngine2::String & strName )
{
	Base* pChild = FindChildByName( strName, true );

	if ( !pChild ) { return ""; }

	return pChild->GetValue();
}

TextObject Base::GetValue()
{
	return "";
}

void Base::SetValue( const TextObject & strValue )
{
}

int Base::GetNamedChildren( FK2DEngine2::ControlList & list, const FK2DEngine2::String & strName, bool bDeep )
{
	int iFound = 0;
	Base::List::iterator iter;

	for ( iter = Children.begin(); iter != Children.end(); ++iter )
	{
		Base* pChild = *iter;

		if ( !pChild->GetName().empty() && pChild->GetName() == strName )
		{
			list.Add( pChild );
			iFound++;
		}

		if ( !bDeep ) { continue; }

		iFound += pChild->GetNamedChildren( list, strName, bDeep );
	}

	return iFound;
}

FK2DEngine2::ControlList Base::GetNamedChildren( const FK2DEngine2::String & strName, bool bDeep )
{
	FK2DEngine2::ControlList list;
	GetNamedChildren( list, strName, bDeep );
	return list;
}
//------------------------------------------------------------------------
#ifndef FK_NO_ANIMATION

void Base::Anim_WidthIn( float fLength, float fDelay, float fEase )
{
	FK2DEngine2::Anim::Add( this, new FK2DEngine2::Anim::Size::Width( 0, Width(), fLength, false, fDelay, fEase ) );
	SetWidth( 0 );
}

void Base::Anim_HeightIn( float fLength, float fDelay, float fEase )
{
	FK2DEngine2::Anim::Add( this, new FK2DEngine2::Anim::Size::Height( 0, Height(), fLength, false, fDelay, fEase ) );
	SetHeight( 0 );
}

void Base::Anim_WidthOut( float fLength, bool bHide, float fDelay, float fEase )
{
	FK2DEngine2::Anim::Add( this, new FK2DEngine2::Anim::Size::Width( Width(), 0, fLength, bHide, fDelay, fEase ) );
}

void Base::Anim_HeightOut( float fLength, bool bHide, float fDelay, float fEase )
{
	FK2DEngine2::Anim::Add( this, new FK2DEngine2::Anim::Size::Height( Height(), 0, fLength, bHide, fDelay, fEase ) );
}

#endif
//------------------------------------------------------------------------