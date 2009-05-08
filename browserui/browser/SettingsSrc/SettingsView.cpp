/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*     View of the information about the active settings
*     
*
*/


// INCLUDE FILES

#include <AKNLISTS.H>
#include <CALSLBS.H>
#include <AKNVIEWAPPUI.H>
#include <BrowserNG.rsg>
#include <EikMenuP.H>

#include "BrowserAppUi.h"
#include "BrowserPreferences.h"
#include "BrowserDialogs.h"
#include "SettingsView.h"
#include "SettingsContainer.h"
#include "CommonConstants.h"

#include <eikapp.h>
// CONSTS

// ========================= MEMBER FUNCTIONS ================================

// ---------------------------------------------------------
// CBrowserSettingsView::ConstructL
// ---------------------------------------------------------
//
void CBrowserSettingsView::ConstructL()
	{
	BaseConstructL( R_BROWSER_SETTINGS_SETTINGS_VIEW );
	}

// ---------------------------------------------------------
// CBrowserSettingsView::NewLC
// ---------------------------------------------------------
//
CBrowserSettingsView* CBrowserSettingsView::NewLC( MApiProvider& aApiProvider )
    {
	CBrowserSettingsView* result;
	result = new( ELeave ) CBrowserSettingsView( aApiProvider );
	CleanupStack::PushL( result );
	result->ConstructL();
	return result;
	}

// ---------------------------------------------------------
// CBrowserSettingsView::NewL
// ---------------------------------------------------------
//
CBrowserSettingsView* CBrowserSettingsView::NewL( MApiProvider& aApiProvider )
	{
	CBrowserSettingsView* result;
	result = CBrowserSettingsView::NewLC( aApiProvider );
	CleanupStack::Pop();
	return result;
	}

// ---------------------------------------------------------
// CBrowserSettingsView::CBrowserSettingsView
// ---------------------------------------------------------
//
CBrowserSettingsView::CBrowserSettingsView( MApiProvider& aApiProvider )
: CBrowserViewBase( aApiProvider )
	{
	}

// ---------------------------------------------------------
// CBrowserSettingsView::~CBrowserSettingsView
// ---------------------------------------------------------
//
CBrowserSettingsView::~CBrowserSettingsView()
	{
	delete iContainer;
	}

// ---------------------------------------------------------------------------
// CBrowserSettingsView::CommandSetResourceIdL
// ---------------------------------------------------------------------------
TInt CBrowserSettingsView::CommandSetResourceIdL()
    {
    // It is never called, but this function have to be implemented
    return KWmlEmptyResourceId;
    }

// ---------------------------------------------------------
// CBrowserSettingsView::HandleCommsModelChangeL
// ---------------------------------------------------------
//
void CBrowserSettingsView::HandleCommsModelChangeL()
	{
	MPreferences& preferences = ApiProvider().Preferences();
    TUint defaultAp = preferences.DefaultAccessPoint();

	// re-check default access point in case it was deleted.
	preferences.SetDefaultAccessPointL(defaultAp);
	
	if ( iContainer )
		{
		iContainer->DisplayCorrectSettingCategoryListL();
		}
	}

// ---------------------------------------------------------
// CBrowserSettingsView::HandleCommandL
// ---------------------------------------------------------
//
void CBrowserSettingsView::HandleCommandL( TInt aCommandId )
    {
    switch ( aCommandId )
        {
        case EWmlCmdSettingsOpen:
            {
            UpdateCbaL(R_BROWSER_SETTINGS_CBA_OPTIONS_BACK_CHANGE);
            iContainer->DisplayCorrectSettingCategoryListL();
            break;
            }
        case EWmlCmdSettingsChange:
            {
            UpdateCbaL(R_BROWSER_SETTINGS_CBA_OPTIONS_BACK_CHANGE);
            iContainer->ChangeItemL( ETrue );
            break;
            }            
        case EAknSoftkeyBack:
            {
            if ( iContainer->SettingsError() )
                {
                TBrowserDialogs::ErrorNoteL( R_BROWSER_SETTING_TOOLBAR_SAVE_DIALOG );
                break;
                }
                
            UpdateCbaL(R_BROWSER_SETTINGS_CBA_OPTIONS_BACK_OPEN);

            // Close the settings
            if ( iContainer->CloseSettingsListL() )
                {
                if ( iLastViewId != KNullViewId )
                    {
                    ActivateViewL( iLastViewId );
                    }
                }
            break;
            }
        case EWmlNoCmd:
            {
            // Fake MSK command for settings -- update cba to label MSK as Change
            UpdateCbaL(R_BROWSER_SETTINGS_CBA_OPTIONS_BACK_CHANGE);
            break;
            }    
            
    	case EWmlCmdUserExit:
	    	{
	    	if ( iContainer->SettingsError() )
                {
                TBrowserDialogs::ErrorNoteL( R_BROWSER_SETTING_TOOLBAR_SAVE_DIALOG );
                break;
                }
            
            // Save the settings
            iContainer->SaveChangesL();
	    	}
	    	// intentional fall through to default case below.

        default:
            {
            AppUi()->HandleCommandL( aCommandId );
            break;
            }
        }
    }

// ---------------------------------------------------------
// CBrowserSettingsView::Id
// ---------------------------------------------------------
//
TUid CBrowserSettingsView::Id() const
	{
	return KUidBrowserSettingsViewId;
	}

// ---------------------------------------------------------
// CBrowserSettingsView::DoActivateL
// ---------------------------------------------------------
//
void CBrowserSettingsView::DoActivateL( const TVwsViewId& aPrevViewId,
                                        TUid aCustomMessageId,
                                        const TDesC8& /*aCustomMessage*/ )
	{
  	StatusPane()->SwitchLayoutL( R_AVKON_STATUS_PANE_LAYOUT_USUAL );
  	StatusPane()->MakeVisible( ETrue );
  		
	if (aPrevViewId.iAppUid == AppUi()->Application()->AppDllUid())
	    { // store this only if our own...
    	    iLastViewId = aPrevViewId;    
	    }
	else 
	    {
	    iLastViewId = TVwsViewId(AppUi()->Application()->AppDllUid(),  ApiProvider().LastActiveViewId());  
	    }    

    ApiProvider().SetLastActiveViewId( Id() );

	// restoring container
	iContainer = CSettingsContainer::NewL
        ( ClientRect(), ApiProvider(), *this );
        

	AppUi()->AddToViewStackL( *this, iContainer );
    ApiProvider().CommsModel().AddObserverL( *this );
    
    if (iContainer && ( aCustomMessageId == KUidSettingsGotoToolbarGroup ) )
        {
        iContainer->DisplayToolbarSettingsL();
        UpdateCbaL(R_BROWSER_SETTINGS_CBA_OPTIONS_BACK_CHANGE);
        }
    else if (iContainer && ( aCustomMessageId == KUidSettingsGotoShortcutsGroup ) )
        {
        iContainer->DisplayShortcutsSettingsL();
        UpdateCbaL(R_BROWSER_SETTINGS_CBA_OPTIONS_BACK_CHANGE);
        }
    else if (iContainer)
        {
        UpdateCbaL(R_BROWSER_SETTINGS_CBA_OPTIONS_BACK_OPEN);
        }
	}

// ---------------------------------------------------------
// CBrowserSettingsView::DoDeactivate
// ---------------------------------------------------------
//
void CBrowserSettingsView::DoDeactivate()
	{
	if ( !ApiProvider().ExitInProgress() )
        {
        ApiProvider().CommsModel().RemoveObserver( *this );
        }
    // removing container
    AppUi()->RemoveFromViewStack( *this, iContainer );
	delete iContainer;
	iContainer = NULL;
	}

// ---------------------------------------------------------
// CBrowserSettingsView::HandleClientRectChange
// ---------------------------------------------------------
//
void CBrowserSettingsView::HandleClientRectChange()
	{
	if( iContainer )
	    {
        iContainer->SetRect( ClientRect() );
        }
	}

// ---------------------------------------------------------
// CBrowserSettingsView::HandleForegroundEventL
// ---------------------------------------------------------
//
void CBrowserSettingsView::HandleForegroundEventL (TBool aForeground)
	{
	if(aForeground)
		{
		iContainer->HandleGainingForegroundL();
		}
	}

// ---------------------------------------------------------------------------
// CBrowserSettingsView::DynInitMenuPaneL
// ---------------------------------------------------------------------------
void CBrowserSettingsView::DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane )
    {
    if ( aResourceId == R_BROWSER_SETTINGS_SETTINGS_MENU )    
        {
        // Show the appropriate menu items, depending on whether
        // we are in the main setting list, or sub category e.g. Page
        if ( iContainer->IsSettingModifiable() )
            {
            aMenuPane->SetItemDimmed( EWmlCmdSettingsOpen, ETrue );
            }
        else
            {
            aMenuPane->SetItemDimmed( EWmlCmdSettingsChange, ETrue );
            }

        }
        
    }

// ---------------------------------------------------------------------------
// CBrowserSettingsView::UpdateCba()
// ---------------------------------------------------------------------------
void CBrowserSettingsView::UpdateCbaL(TInt aCommandSet)
    {
    if (Cba())
        {
        Cba()->SetCommandSetL(aCommandSet);
        Cba()->DrawDeferred();
        }
    }
// End of File
