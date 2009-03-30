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
*      Declaration of class CBrowserAppViewBase.
*
*
*/


#include "BrowserAppViewBase.h"
#include "BrowserAppUi.h"
#include "ApiProvider.h"
#include <BrowserNG.rsg>
#include "CommonConstants.h"
#include <aknviewappui.h>
#include <eikbtgpc.h>
#include <avkon.rsg>
#include <LayoutMetaData.cdl.h> // check landscape/portrait modes
#include <aknenv.h>
#include <AknSgcc.h>
#include <StringLoader.h>


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// TSKPair::TSKPair
// ---------------------------------------------------------
//
TSKPair::TSKPair():
	iCmdId(EWmlNoCmd),iQtnRsc(R_BROWSER_DYN_SK_NULL)
{
}	

// ---------------------------------------------------------
// TSKPair::TSKPair
// ---------------------------------------------------------
//
TSKPair::TSKPair(TInt aId,TInt aRsc):
	iCmdId(aId),iQtnRsc(aRsc)
{
}	

// ---------------------------------------------------------
// TSKPair::IsNotDefault
// ---------------------------------------------------------
//
TBool TSKPair::IsAssigned()
{
	return ( (iCmdId != EWmlNoCmd) && (iQtnRsc != R_BROWSER_DYN_SK_NULL) );
}

// ---------------------------------------------------------
// TSKPair::operator==
// ---------------------------------------------------------
//
TBool TSKPair::operator==(TSKPair aSkPair)
{
	return( (iCmdId == aSkPair.id()) && (iQtnRsc == aSkPair.qtn()) );
}

// ---------------------------------------------------------
// TSKPair::operator=
// ---------------------------------------------------------
//
void TSKPair::operator=(TSKPair aSKPair)
{
	iCmdId = aSKPair.id();
	iQtnRsc = aSKPair.qtn();
}

// ---------------------------------------------------------
// TSKPair::set
// ---------------------------------------------------------
//
void TSKPair::setPair(TInt aId, TInt aRsc)
{
	iCmdId = aId;
	iQtnRsc = aRsc;
}


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserViewBase::CBrowserViewBase
// ---------------------------------------------------------
//
CBrowserViewBase::CBrowserViewBase( MApiProvider& aApiProvider ):
    iApiProvider( aApiProvider ),
    iLastCommandSet( R_BROWSER_BUTTONS_CBA_NULL )
    {
    }

// ---------------------------------------------------------
// CBrowserViewBase::ApiProvider
// ---------------------------------------------------------
//
MApiProvider& CBrowserViewBase::ApiProvider()
    {
    return iApiProvider;
    }

// ---------------------------------------------------------
// CBrowserViewBase::UpdateCbaL
// ---------------------------------------------------------
//
void CBrowserViewBase::UpdateCbaL()
{
    if ( !iApiProvider.ExitInProgress() )
        {
        TInt commandSet = CommandSetResourceIdL();
        TUid currViewId = Id();

		// Ask BrowserAppUi which view we were in when we last updated the CBA
		// this will help decide if we should do it again
        TUid previousCBAUpdateViewId = iApiProvider.GetLastCBAUpdateView();

        if ( commandSet )
        	{
        	// Don't perform updates to the CBA when
        	//
        	// - We're still in the same view we were just in AND
        	// - We're not dynamically setting CBA (do more checking below) AND
        	// - The command set is the same as the one we just set last time
        	//   this was called
        	// - We're currently in the content view (the update-cba is more complex here
        	//   and we should just do it when asked by ContentView class)
        	//
        	// NOTE that we rely on CommandSetResource to set the commandSet to
        	//  R_BROWSER_BUTTONS_CBA_DYNAMIC to signal a desire to update the keys
        	//  dynamically. 
			if(	currViewId == iPreviousViewID &&
				commandSet != R_BROWSER_BUTTONS_CBA_DYNAMIC &&
				currViewId == previousCBAUpdateViewId &&
				currViewId != KUidBrowserContentViewId)
				{
				return;
				}
        	
        	// If command set is our special dynamic cba set,
        	// now ask derived class for the resources dynamically.        	
        	if(commandSet == R_BROWSER_BUTTONS_CBA_DYNAMIC)
            	{
				TSKPair lsk,rsk,msk;					
				CommandSetResourceDynL(lsk,rsk,msk);

				// Similar exclusion as above:
				// Don't update the LSK, RSK, MSK dynamic assignments if:
				//
				// - We're still in the same view we were just in
				// - The lsk, rsk, msk are all the same settings we set last time
				if(currViewId == previousCBAUpdateViewId &&
					lsk == iLastLSK && 
					rsk == iLastRSK &&
					msk == iLastMSK)
				{
					return;
				}				
				
				// Save the settings
				iLastLSK = lsk;
				iLastRSK = rsk;
				iLastMSK = msk;
				
				//
				// Update each softkey in turn
				// 
				// If the CommandSetResourceDynL command assigned something to the key
				// (in other words, it changed the key assignment from its default - it does have the option not to do so), 
				// 
				// Update each key now.
				//
				// Each derived-class view should update them as a general rule, even if it is to some default value
				//	
				if( lsk.IsAssigned() )
					{
					HBufC* txt = StringLoader::LoadLC( lsk.qtn() );
            		Cba()->SetCommandL(CEikButtonGroupContainer::ELeftSoftkeyPosition, lsk.id(), *txt);
					CleanupStack::PopAndDestroy();  // txt
					}
				if( rsk.IsAssigned() )
					{
					HBufC* txt = StringLoader::LoadLC( rsk.qtn() );
            		Cba()->SetCommandL(CEikButtonGroupContainer::ERightSoftkeyPosition, rsk.id(), *txt);
					CleanupStack::PopAndDestroy();  // txt
					}
				if( msk.IsAssigned() )
					{
					HBufC* txt = StringLoader::LoadLC( msk.qtn() );
            		Cba()->SetCommandL(CEikButtonGroupContainer::EMiddleSoftkeyPosition, msk.id(), *txt);
					CleanupStack::PopAndDestroy();  // txt
					}
				}
			else // Else - the exisiting case: set the softkeys as a block by assigning a pre-defined resource id
				{
	        	// The usual case is to set the command set to 
    	    	// whatever CommandSetResourceIdL has said
	    	  	Cba()->SetCommandSetL( commandSet );				
				}				
 			
 			// Save the last command set and set view id
 			iLastCommandSet = commandSet;
 			iApiProvider.SetLastCBAUpdateView(currViewId);
 			
	   		// DrawNow() doesn't refresh MSK properly
           	Cba()->DrawDeferred();
			}
        }    
}








// ---------------------------------------------------------
// CBrowserViewBase::MakeCbaVisible
// ---------------------------------------------------------
//
void CBrowserViewBase::MakeCbaVisible(TBool aVisible)
    {
    // don't turn off the cba in landscape mode since it will still be visible and
    // calling MakeVisible(EFalse) will just make it not update correctly

    TBool vis = aVisible;
    if (!vis)
        {
        CEikStatusPaneBase* statusPane = CEikStatusPaneBase::Current();
        TInt currentStatusPaneLayoutResId = 0;
        if (statusPane)
            {
            currentStatusPaneLayoutResId = statusPane->CurrentLayoutResId();
            }
        else
            {
            // If this CBA:s owner does not have statuspane, then we ask the layout from AknCapServer.
            currentStatusPaneLayoutResId = AVKONENV->StatusPaneResIdForCurrentLayout(CAknSgcClient::CurrentStatuspaneResource());
            }

        TBool staconPane = ((currentStatusPaneLayoutResId == R_AVKON_STACON_PANE_LAYOUT_USUAL_SOFTKEYS_RIGHT) ||
                      (currentStatusPaneLayoutResId == R_AVKON_STACON_PANE_LAYOUT_USUAL_SOFTKEYS_LEFT)  ||
                      (currentStatusPaneLayoutResId == R_AVKON_STACON_PANE_LAYOUT_EMPTY_SOFTKEYS_RIGHT) ||
                      (currentStatusPaneLayoutResId == R_AVKON_STACON_PANE_LAYOUT_EMPTY_SOFTKEYS_LEFT)  ||
                      (currentStatusPaneLayoutResId == R_AVKON_STACON_PANE_LAYOUT_IDLE_SOFTKEYS_RIGHT) ||
                      (currentStatusPaneLayoutResId == R_AVKON_STACON_PANE_LAYOUT_IDLE_SOFTKEYS_LEFT));
        vis = staconPane;
        }
    Cba()->MakeVisible( vis );
    }

// ---------------------------------------------------------
// CBrowserViewBase::UpdateGotoPaneL
// ---------------------------------------------------------
//
void CBrowserViewBase::UpdateGotoPaneL()
    {
    }

// ---------------------------------------------------------
// CBrowserViewBase::ClearCbaL
// ---------------------------------------------------------
//
void CBrowserViewBase::ClearCbaL()
    {
    TInt commandSet = R_AVKON_SOFTKEYS_EMPTY;
    Cba()->SetCommandSetL( commandSet );
    Cba()->DrawNow();
    }
// End of File
