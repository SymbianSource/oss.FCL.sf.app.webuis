/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
*      Implementation of CBrowserBookmarksGotoPane.
*      
*
*/


// INCLUDE FILES

#include <akninfrm.h>
#include <eikedwin.h>
#include <FavouritesLimits.h>

#include <aknconsts.h>
#include <akneditstateindicator.h>
#include <AknsListBoxBackgroundControlContext.h>
#include <AknsFrameBackgroundControlContext.h>

#include <BrowserNG.rsg>
#include <Avkon.mbg>
#include <AknsUtils.h>
#include <AppApacLayout.cdl.h>
#include <AppLayout.cdl.h>
#include <AknLayout.cdl.h>

#include <centralrepository.h> 
#include <AknFepInternalCRKeys.h>
#include <PtiDefs.h>

#include "BrowserBookmarksGotoPane.h"
#include "BrowserUtil.h"
#include "commonconstants.h"
#include "BrowserAdaptiveListPopup.h"
#include "WmlBrowserBuild.h" //feature manager
#include "browser.hrh"
#include "BrowserAppUi.h"
#include "BrowserFavouritesView.h"
#include "Preferences.h"
#include <Featmgr.h>

#include <eikpriv.rsg>
#include <BARSREAD.H>

#include <skinlayout.cdl.h>
using namespace SkinLayout;

const TInt EUseSkinContext = 0x01;
const TInt EParentAbsolute = 0x02;
const TInt KRightSpace = 10; //space in pixels left at the end of text editor.
// CONSTANTS

_LIT(KAddressText,"http://www.");

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::NewL
// ---------------------------------------------------------
//
CBrowserBookmarksGotoPane* CBrowserBookmarksGotoPane::NewL
( const CCoeControl& aParent, CBrowserFavouritesView* aView )
    { 
    CBrowserBookmarksGotoPane* gotoPane =
        new(ELeave) CBrowserBookmarksGotoPane(aView);
    CleanupStack::PushL( gotoPane );
    gotoPane->ConstructL( aParent );
    CleanupStack::Pop();    // gotoPane
    return gotoPane;
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::~CBrowserBookmarksGotoPane
// ---------------------------------------------------------
//
CBrowserBookmarksGotoPane::~CBrowserBookmarksGotoPane()
    {   
    delete iEditor;
    delete iInputFrame;
    delete iSkinContext;
    delete iInputContext;
    delete iBAdaptiveListPopup;
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::BeginEditingL()
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::BeginEditingL()
    {
    if ( !iFrozen )
        {
       	// set the default string, make visible, start editing
       	SetTextL( KWWWString );                   
       	MakeVisible( ETrue );
       	SetEditingL( ETrue );
        if (AknLayoutUtils::PenEnabled())
            {
            ActivateVKB();
            }
        }
    }
    
// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::CancelEditingL()
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::CancelEditingL()
    {
    if ( !iFrozen )
        {
        SetEditingL( EFalse );

		// Cancel Adaptive List popup if its active
		if( NULL != iBAdaptiveListPopup)
			{
		 	iBAdaptiveListPopup->SetDirectoryModeL( ETrue );
		 	iBAdaptiveListPopup->HidePopupL();
		 	}
        }
    
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::SetTextL
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::SetTextL
( const TDesC& aText, TBool aClipToFit /*=ETrue*/, TBool aCancelEditing )
    {
    
    CFbsFont * font = NULL;
    	
    if ( !iFrozen )
        {
        if ( aClipToFit && aText.Length() )
            {
            // Allocate 3 chars more (make sure "..." fits).
            HBufC* buf = HBufC::NewLC( aText.Length() + 3 );
            *buf = aText;
            TPtr ptr( buf->Des() );
            CTextView* textView = iEditor->TextView();
            if ( textView )
                {
                //The Textview does not use the iConEnv->NormalFont() for drawing so we can not use it for measurement.
                //Textview uses the default char format font for drawing. See bug: EHCN-6U59SN
	            TBuf<KMaxTypefaceNameLength> typeface;
	            CEikonEnv::Static()->ReadResource(typeface,R_EIK_DEFAULT_CHAR_FORMAT_TYPEFACE);
	            TResourceReader reader;
	            CEikonEnv::Static()->CreateResourceReaderLC(reader,R_EIK_DEFAULT_CHAR_FORMAT_HEIGHT);
                TInt height =  reader.ReadInt16();
	            CleanupStack::PopAndDestroy(); // reader
                
	            TFontSpec fontSpec(typeface, height);
                CEikonEnv::Static()->ScreenDevice()->GetNearestFontInTwips((CFont*&)font, fontSpec);

                AknTextUtils::ClipToFit
                    (
                    ptr,
                    *font,
                    textView->ViewRect().Width() - KRightSpace
                    );
                }
            iEditor->SetTextL( &ptr );
            CleanupStack::PopAndDestroy();  // buf
            }
        else
            {
			if ( aText.Length() )
				{
				iEditor->SetTextL( &aText );
				}
			else
				{   
    			TBufC<16> buf(KAddressText);
    			HBufC* tmpBuf = NULL;
    			tmpBuf = buf.AllocLC();
    			iEditor->SetTextL( tmpBuf );
    			CleanupStack::PopAndDestroy(); // tmpBuf
				}
            }
	
        iEditor->SetCursorPosL( iEditor->TextLength(), EFalse );
        if ( !iEverFocused )
			{
			iEditor->SetFocus( ETrue );
			}

		if ( !iEverFocused )
			{
			iEditor->SetFocus( IsFocused() );
			iEverFocused = ETrue;
			}
			SetEditingL( !aCancelEditing );
			DrawDeferred();
        }
        
      // release font if it was used
      if(font != NULL) 
      	{
       	CEikonEnv::Static()->ScreenDevice()->ReleaseFont(font);
      	}
        
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::GetTextL
// ---------------------------------------------------------
//
HBufC* CBrowserBookmarksGotoPane::GetTextL()
    {
    HBufC* text = HBufC::NewL( iEditor->TextLength() + 1 );
    TPtr ptr = text->Des();
    iEditor->GetText( ptr );
    ptr.ZeroTerminate();
    
    Util::EncodeSpaces(text);

    return text;
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::SetObserver
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::SetGPObserver
( MBookmarksGotoPaneObserver* aObserver )
    {
    __ASSERT_DEBUG( !iGPObserver, \
        Util::Panic( Util::EGotoPaneObserverAlreadySet ) );
    iGPObserver = aObserver;
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::SetLineState
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::SetLineState( TBool aLineVisible )
    {
    iInputFrame->SetLineState( aLineVisible );
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::OfferKeyEventL
// ---------------------------------------------------------
//
TKeyResponse CBrowserBookmarksGotoPane::OfferKeyEventL
( const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    TKeyResponse response = EKeyWasNotConsumed;
	TChar iInputChar = TChar( aKeyEvent.iScanCode );


    if ( iBAdaptiveListPopup )
        {  
         response = iBAdaptiveListPopup->OfferKeyEventL( aKeyEvent, aType ); 
        }

    if ( iEditing && response != EKeyWasConsumed)
        {
        // Editing; most keys simply forwarded to the editor.
        switch ( aKeyEvent.iCode )
            {
            case EKeyOK:           
                {
                // MSK is now handled through HandleCommand in BrowserBookmarksView
                response = EKeyWasConsumed;
               	break;
                }
            case EKeyEnter:
            	// Handle EnterKey as "GOTO" for Touch, VKB's enter key
                if( AknLayoutUtils::PenEnabled() )
                	{
                	CBrowserAppUi::Static()->ActiveView()->HandleCommandL(EWmlCmdGotoPaneGoTo);
                	break;
                	}
                // FALL through to default in case Pen is not enabled.
            default:
                {
                response = iEditor->OfferKeyEventL(aKeyEvent, aType );
                break;
                }
            }
            
         // Force key event to be consumed if you're editing
         // regardless of what's happened to it.
         //
         // This forces user to press cancel to continue other tasks
         response = EKeyWasConsumed;
        }
    else if ( response != EKeyWasConsumed )
        {
            
        if ( (aKeyEvent.iScanCode == EStdKeyHash)  ||
                ( aKeyEvent.iModifiers &
                ( EModifierShift | EModifierLeftShift | EModifierRightShift |
                EModifierCtrl | EModifierLeftCtrl | EModifierRightCtrl ) ) )
            {
            
            // Hash key press will be used for Mark/UnMark functionality
            // Let Platform Listbox handle this.
            response = EKeyWasNotConsumed;
            }

        //
        // Not currently editing the GoTo pane.
        // Catch alpha and numeric to pop
        // up goto pane
        //
        // Only popup the GoTo Pane with a KeyDown event.
        // Otherwise, if browser is pushed to the backround (but still active) 
        // in favor of the App Shell, A long "0" press will reactivate the browser and 
        // leave a KeyUp to (wrongly) pop the pane up 
        //
        // Note that we exclude Unicode 0xC4 and 0xC5 which
        // are defined by EStdkeyYes and EStdkeyNo. These
        // map to the Send and End keys respectively. Normally,
        // 0xC4 and 0xC5 can be considered alpha digits, but we exclude them so that
        // they will not pop the goto pane
        //
        // key event is generated again (to channel through FEP)
        //        
        else if ( !iFrozen &&
    		aType == EEventKeyDown &&        
        	iInputChar.IsAlphaDigit() &&
    		iInputChar != EStdKeyYes &&
    		iInputChar != EStdKeyNo)
    		{
            // These keys activate the Goto Pane.
            response = EKeyWasConsumed;
            
            BeginEditingL();

            iEditor->SetCursorPosL( iEditor->TextLength(), EFalse );
            iCoeEnv->SyncNotifyFocusObserversOfChangeInFocus();
            iCoeEnv->SimulateKeyEventL( aKeyEvent, aType );
            
            }
        else
            {
            // Other keys do not activate the Goto Pane.
            response = EKeyWasNotConsumed;
            }
        }
    return response;
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::CountComponentControls
// ---------------------------------------------------------
//
TInt CBrowserBookmarksGotoPane::CountComponentControls() const
    {
    return 2;
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::ComponentControl
// ---------------------------------------------------------
//
CCoeControl* CBrowserBookmarksGotoPane::ComponentControl
( TInt aIndex ) const
    {
    switch ( aIndex )
        {
        case 0:
            {
            return iInputFrame;
            }

        case 1:
            {
            return iEditor;
            }

        default:
            {
            return NULL;
            }
        }
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::FocusChanged
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::FocusChanged( TDrawNow aDrawNow )
    {
    iEditor->SetFocus( IsFocused(), aDrawNow );
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::CBrowserBookmarksGotoPane
// ---------------------------------------------------------
//
CBrowserBookmarksGotoPane::CBrowserBookmarksGotoPane(CBrowserFavouritesView* aView)
: iView(aView), iEditing( EFalse ), iFrozen( EFalse ) 
    {
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::ConstructL
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::ConstructL
( const CCoeControl& aParent )
	{	
    SetContainerWindowL( aParent ); // This is now non-window owning control
    SetMopParent(const_cast<CCoeControl*>(&aParent));
    iEditor = new (ELeave) CEikEdwin;
    iInputFrame = CAknInputFrame::NewL( iEditor, EFalse, KAvkonBitmapFile,
                                    EMbmAvkonQgn_indi_find_goto, 
                                    EMbmAvkonQgn_indi_find_goto_mask, 
                                    0 );
    iInputFrame->SetContainerWindowL( *this );
    iEditor->SetContainerWindowL( *this );
    AknEditUtils::ConstructEditingL(iEditor, KFavouritesMaxUrlGotoPaneDefine, 1, EAknEditorCharactersLowerCase, EAknEditorAlignRight, EFalse, ETrue, EFalse);
	iEditor->AddFlagToUserFlags( CEikEdwin::EAlwaysShowSelection );
    iEditor->SetBorder( TGulBorder::ENone );
    iEditor->SetAknEditorCase( EAknEditorLowerCase );
    iEditor->SetAknEditorInputMode( EAknEditorTextInputMode );
    iEditor->SetSkinBackgroundControlContextL(NULL);
    if (AVKONAPAC)
        {
        // Disallow chinese input.
        iEditor->SetAknEditorAllowedInputModes( EAknEditorTextInputMode | EAknEditorNumericInputMode );
        }

    TInt editorFlags( EAknEditorFlagLatinInputModesOnly | EAknEditorFlagUseSCTNumericCharmap );
#ifdef RD_INTELLIGENT_TEXT_INPUT
    TInt physicalKeyboards = 0; 
    CRepository* aknFepRepository = CRepository::NewL( KCRUidAknFep );
	User::LeaveIfNull( aknFepRepository );

    aknFepRepository->Get( KAknFepPhysicalKeyboards,  physicalKeyboards );
    delete aknFepRepository; 
        
	if ( physicalKeyboards &&   EPtiKeyboardQwerty3x11 ) {
        editorFlags = (editorFlags | EAknEditorFlagNoT9);
	}
#endif 
    iEditor->SetAknEditorFlags( editorFlags ); 


    iEditor->SetAknEditorPermittedCaseModes( EAknEditorUpperCase | EAknEditorLowerCase );

    iEditor->CreateTextViewL(); 
    SetupSkinContextL();  
	iEditing = EFalse;
    //adaptive popuplist
    iBAdaptiveListPopup = NULL;
    if( AUTOCOMP ) //ask the feature manager
        {
        iBAdaptiveListPopup= new (ELeave) CBrowserAdaptiveListPopup(
            iEditor, this, EBookmarksGotoPane);
        iBAdaptiveListPopup->ConstructL();
        iEditor->SetObserver( iBAdaptiveListPopup );
        if (iView)
	    	{
			iBAdaptiveListPopup->SetUrlSuffixList(iView->ApiProvider().Preferences().URLSuffixList());
			iBAdaptiveListPopup->SetMaxRecentUrls(iView->ApiProvider().Preferences().MaxRecentUrls());			
	    	}
        }

    ActivateL();
    MakeVisible( EFalse ); 
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::SizeChanged
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::SizeChanged()
    {
    TAknLayoutRect layoutRectTemp;
    TAknWindowLineLayout windowLayoutTmp, windowLayoutTmp2;
    
    windowLayoutTmp = AknLayout::Find_pane_elements_Line_4();
    windowLayoutTmp2 = windowLayoutTmp;
    layoutRectTemp.LayoutRect( Rect(), windowLayoutTmp2 );
   	iEditor->SetRect( layoutRectTemp.Rect() );
   	iInputFrame->SetRect( Rect() );
    TRAP_IGNORE(SetupSkinContextL());
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::SetEditingL
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::SetEditingL( TBool aEditing )
    {
    if ( aEditing != iEditing )
        {
        iEditing = aEditing;
        SetFocus( iEditing );
        if ( iGPObserver )
            {
            iGPObserver->HandleBookmarksGotoPaneEventL
                (
                this,
                MBookmarksGotoPaneObserver::EEventEditingModeChanged
                );
            }
        }
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::PopupList
// ---------------------------------------------------------
//
CBrowserAdaptiveListPopup* CBrowserBookmarksGotoPane::PopupList()
	{
	return iBAdaptiveListPopup;
	}

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::HandleResourceChange
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::HandleResourceChange( 
			TInt aType 
			)
	{
	CCoeControl::HandleResourceChange( aType );

	if( iBAdaptiveListPopup )
    	{
        iBAdaptiveListPopup->HandleResourceChange( aType );
        }
	}

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::MopSupplyObject
// ---------------------------------------------------------
//
TTypeUid::Ptr CBrowserBookmarksGotoPane::MopSupplyObject( TTypeUid aId )
    {
    return MAknsControlContext::SupplyMopObject( aId, iSkinContext );
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksGotoPane::Editor
// ----------------------------------------------------------------------------
CEikEdwin* CBrowserBookmarksGotoPane::Editor() const
    {
    return iEditor;
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksGotoPane::HandlePointerEventL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksGotoPane::HandlePointerEventL(const TPointerEvent& aPointerEvent)
    {
    if (AknLayoutUtils::PenEnabled())
        {
        if(iEditor && Rect().Contains(aPointerEvent.iPosition))
            {
            iEditor->HandlePointerEventL(aPointerEvent);
            if ( !iEditing )
                {
                SetEditingL( ETrue );
                iCoeEnv->SyncNotifyFocusObserversOfChangeInFocus();
                CBrowserAppUi::Static()->UpdateCbaL();
                }
            }
        else
            {
            // pointer outside of control
            CBrowserAppUi::Static()->ActiveView()->HandleCommandL(EWmlCmdGotoPaneCancel);
            }
        }
    }
// ----------------------------------------------------------------------------
// CBrowserBookmarksGotoPane::SetupSkinContextL
// ----------------------------------------------------------------------------
void CBrowserBookmarksGotoPane::SetupSkinContextL()
    {
    TAknsItemID tileIID = KAknsIIDQsnBgColumnA;
    
    TAknWindowLineLayout tile =
        Column_background_and_list_slice_skin_placing_Line_2();
    
    TAknLayoutRect mainPane;
    mainPane.LayoutRect(
        iAvkonAppUi->ApplicationRect(),
        AKN_LAYOUT_WINDOW_main_pane( iAvkonAppUi->ApplicationRect(),
                                     0, 0, 1) );
    TAknLayoutRect listRect;
    listRect.LayoutRect( mainPane.Rect(), AknLayout::list_gen_pane(0) );

    TAknLayoutRect tileRect;
    tileRect.LayoutRect(mainPane.Rect(), tile);  

    if ( !iSkinContext )
        {
        iSkinContext = CAknsListBoxBackgroundControlContext::NewL(
            KAknsIIDQsnBgAreaMainListGene, 
            listRect.Rect(), 
            EUseSkinContext & EParentAbsolute,
            tileIID,
            tileRect.Rect() );
        }
    else
        {
        iSkinContext->SetRect( listRect.Rect() );
        }


    if ( !iInputContext )
        {
        iInputContext = CAknsFrameBackgroundControlContext::NewL(
            KAknsIIDQsnFrInput, TRect(0,0,0,0),TRect(0,0,0,0),EFalse );
        }

    iInputFrame->SetInputContext(iInputContext);
    iEditor->SetSkinBackgroundControlContextL(iInputContext);

    }
    
// ----------------------------------------------------------------------------
// CBrowserBookmarksGotoPane::ActivateVKB
// ----------------------------------------------------------------------------
void CBrowserBookmarksGotoPane::ActivateVKB()
    {
    if (iEditor && iEditor->TextView())
        {
        // make sure observer is set
        iCoeEnv->SyncNotifyFocusObserversOfChangeInFocus();
       	
       	// simulate pointer event to force VKB
       	
       	// first get point at cursor location
        TInt pos = iEditor->CursorPos();
       	CTextView* textView = iEditor->TextView();
       	TPoint curPos;
       	textView->DocPosToXyPosL(pos, curPos);
       	
       	TPointerEvent pe;
       	pe.iPosition = curPos;
       	
       	pe.iType = TPointerEvent::EButton1Down;
       	iEditor->HandlePointerEventL(pe);
       	
        // VKB will only activate is nothing selected
        iEditor->SetSelectionL(pos,pos); 
       	
       	pe.iType = TPointerEvent::EButton1Up;
       	iEditor->HandlePointerEventL(pe);
        }
    }

// End of File
