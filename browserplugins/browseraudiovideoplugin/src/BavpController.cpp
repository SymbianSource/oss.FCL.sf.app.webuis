/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Base Controller class for handling browser requests to play
*                audio or video
*
*/


// INCLUDE FILES
#include <bautils.h>
#include <f32file.h>
#include <utf.h>
#include <e32cmn.h>
#include <aknquerydialog.h>                 // Volume list query dialog
#include <aknlists.h>
#include <CTSYDomainPSKeys.h>

#include <browseruisdkcrkeys.h>
#include <BrowserAudioVideoPlugin.rsg>

#include "BavpController.h"
#include "BavpClipInfo.h"
#include "BavpView.h"
#include "BavpLogger.h"

#if defined(BRDO_ASX_FF)
#include "AsxParser.h"
#endif //BRDO_ASX_FF

#include "eikon.hrh"

using namespace RT_GestureHelper;

// CONSTANTS
const TInt KBavpMaxLinkFileSize = 5120; // 5kB
const TInt KRectPadding = 2;

// Define our static BavpController array
RPointerArray<CBavpController> CBavpController::iBavpControllerArray;


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBavpController::CBavpController
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
CBavpController::CBavpController( MBavpView* aView,
                                  TUint aAccessPtId )
    : CActive( EPriorityStandard ),
      iCurrentState( EBavpNone ),
      iPreCallState( EBavpNone ),
      iLastCommand( EBavpCmdUnknown ),
      iLoopCount( 1 ),
      iInitLoopCount( 1 ),
      iInfiniteLoopFlag( EFalse ),
      iAccessPtId( aAccessPtId ),
      iBavpView( aView ),
      iOriginalFileName ( NULL ),
      iMimeType ( NULL )
    {
        CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CBavpController::~CBavpController()
// Destructor
// -----------------------------------------------------------------------------
CBavpController::~CBavpController()
    {
    Log( EFalse, _L("CBavpController::~CBavpController this="), (TInt)this );

    if ( iClipInfo )
        {
        delete iClipInfo;
        }

    iIncomingCalls.Close();

    // Cancel active object and deleting removes from the ActiveScheduler
    Cancel();

    if ( iBavpVolumeHandler )
        {
        delete iBavpVolumeHandler;
        }

    // No need to check, if not active, Cancel() does nothing
    if ( iPositionUpdater )
        {
        iPositionUpdater->Cancel();
        delete iPositionUpdater;
        }

    if ( iHwKeyEvents )
        {
        delete iHwKeyEvents;
        }

     if ( iOriginalFileName )
     {
        delete iOriginalFileName;
     }

     if ( iMimeType )
     {
        delete iMimeType;
     }

    // Remove this instance from the array
    TInt index = iBavpControllerArray.Find( this );
    if ( index != KErrNotFound )
        {
        iBavpControllerArray.Remove( index );
        }
    if ( iBavpControllerArray.Count() == 0 )
        {
        iBavpControllerArray.Close();
        }
    }

// -----------------------------------------------------------------------------
// CBavpController::HandleBrowserNotification
// Default implementation, override as needed.
// Handles notification from the Browser concerning the Browser's focus state
// (Browser is in-focus, out-focus, foreground, or the background application)
// and if this plugin is in-focus or out-focus. If the Browser is out-focus,
// this plugin (all plugins) are out-focus. This is not used currently,
// because we do not react to the Browser changing focus. We historically would
// stop playing audio when the browser's focus changed (including when option
// menu is popped up, ie menu top of stack).
// -----------------------------------------------------------------------------
void CBavpController::HandleBrowserNotification( TBool aFocus )
    {
    Log( EFalse, _L("CBavpController::HandleBrowserNotification: aBrowserFocus="),
            aFocus );
    }

// -----------------------------------------------------------------------------
// CBavpController::HandleError
// Default implementation, override as needed.
// This default error handling implementation just displays the broken (bad)
// content animation and sets state to EBavpBadContent.
// -----------------------------------------------------------------------------
void CBavpController::HandleError( TInt aError )
    {
    Log( EFalse, _L("CBavpController::HandleError(): aError="), aError );

    if ( aError != KErrNone )
        {
        // Any error, display we have bad content
        iCurrentState = EBavpBadContent;
        iBavpView->UpdateView();
        }
    }

// -----------------------------------------------------------------------------
// CBavpController::HandleKeysL
// Default implementation, override as needed.
// Handles user key presses when the Plugin is Activated.
// -----------------------------------------------------------------------------
TKeyResponse CBavpController::HandleKeysL( const TKeyEvent& aKeyEvent,
                                           TEventCode /*aType*/ )
    {
    Log( EFalse, _L("CBavpController::HandleKeysL()") );

    TKeyResponse keyResponse = EKeyWasNotConsumed;

    switch ( aKeyEvent.iCode )
        {
        // Cancel key (RSK) was pressed
        case EKeyDevice1:
            // Return the focus to the Browser, remove it from Bavp
            iBavpView->CoeControl().SetFocus( EFalse );
            keyResponse = EKeyWasConsumed;
            break;

        // Arrow keys was pressed
        case EKeyUpArrow:             // North

        case EKeyRightUpArrow:        // Northeast
        case EStdKeyDevice11:         // Extra KeyEvent supports diagonal event simulator wedge

        case EKeyRightArrow:          // East

        case EKeyRightDownArrow:      // Southeast
        case EStdKeyDevice12:         // Extra KeyEvent supports diagonal event simulator wedge

        case EKeyDownArrow:           // South

        case EKeyLeftDownArrow:       // Southwest
        case EStdKeyDevice13:         // Extra KeyEvent supports diagonal event simulator wedge

        case EKeyLeftArrow:           // West

        case EKeyLeftUpArrow:         // Northwest
        case EStdKeyDevice10:         // Extra KeyEvent supports diagonal event simulator wedge

            // Do nothing with these keys, there is no navigating inside Bavp.
            keyResponse = EKeyWasConsumed;
            break;

        default:
        // Do nothing, let the rest of the key events pass back not consumed
        break;
        }   // end of switch

    return keyResponse;
    }

// -----------------------------------------------------------------------------
// CBavpController::HandleCommandL
// Default implementation, override as needed.
// Handles the commands from the Option Menu defined in TBavpCommandIds
// -----------------------------------------------------------------------------
void CBavpController::HandleCommandL( TBavpCommandIds aCommand )
    {
    Log( EFalse, _L("CBavpController::HandleCommandL(): aCommand="), (TInt)aCommand );

    switch ( aCommand )
        {
        case EBavpCmdPlay:
            iLastCommand = EBavpCmdPlay;
            PlayL();
            break;

        case EBavpCmdStop:
            iLastCommand = EBavpCmdStop;
            Stop();
            break;

        case EBavpCmdPause:
            iLastCommand = EBavpCmdPause;
            PauseL();
            break;

        case EBavpCmdFastForward:
            iLastCommand = EBavpCmdFastForward;
            FastForwardL();
            break;

        case EBavpCmdRewind:
            iLastCommand = EBavpCmdRewind;
            RewindL();
            break;

        case EBavpCmdChangeVolume:
            iLastCommand = EBavpCmdChangeVolume;
            CreateVolumeListDlgL();
            break;

        case EBavpCmdMuteVolume:
            iLastCommand = EBavpCmdMuteVolume;
            iBavpVolumeHandler->HandleNotifyInt( KBrowserMediaVolumeControl,
                                                 KCRVolumeMute );
            break;

        default:
            iLastCommand = EBavpCmdUnknown;
            break;
        }
    }

// -----------------------------------------------------------------------------
// CBavpController::BavpGainedFocus
// Default implementation, override as needed.
// Called by CBavpView to CBavpPlugin whenever Bavp Plugin has focus and soft key
// cancel is hit. When this happens the Browser calls NotifyL with 1.
// -----------------------------------------------------------------------------
void CBavpController::BavpFocusChanged( TBool aFocus )
    {
    iBavpHasFocus = aFocus;
    }

// -----------------------------------------------------------------------------
// CBavpController::SetVolumeFromAttribute
// Provides CBavpController with the volume attribute from the tags on the
// HTTP page, i.e. <object volume="high"...
// -----------------------------------------------------------------------------
void CBavpController::SetVolumeFromAttribute( TInt aAttributeVolume )
    {
    // Use the lower volume:
    // 1) Current volume (iCurrentVolume)
    // 2) Volume setting on web page (aAttributeVolume)
    if ( aAttributeVolume < iCurrentVolume )
        {
        iCurrentVolume = aAttributeVolume;
        iBavpVolumeHandler->HandleNotifyInt( KBrowserMediaVolumeControl,
                                             iCurrentVolume );
        }
    }

// -----------------------------------------------------------------------------
// CBavpController::SetLoopFromAttribute
// Provides CBavpController with the loop attribute from the tags on the
// HTTP page, i.e. <object loop="infinite"... or <object loop="4"...
// -----------------------------------------------------------------------------
void CBavpController::SetLoopFromAttribute( TInt aLoopValue, TBool aInfiniteFlag )
    {
    // Save the loop count, already checked for valid range
    iLoopCount = aLoopValue;

    // Save the initial loop count, in case we get an error while looping
    // through the content
    iInitLoopCount = iLoopCount;

    // If the loop value is set to "infinite" looping. We currently use
    // a max loop of 50 for infinite
    iInfiniteLoopFlag = aInfiniteFlag;
}

// -----------------------------------------------------------------------------
// CBavpControllerAudioo::RefreshRectCoordinatesL
// Refresh the coordinates of rect for display
// -----------------------------------------------------------------------------
void CBavpController::RefreshRectCoordinatesL()
    {
    Log( EFalse, _L("CBavpController::RefreshRectCoordinatesL()") );
    }

// -----------------------------------------------------------------------------
// CBavpController::ParseRamFileL
// Read the ram file and parse the the url, put into iClipInfo->iUrl
// -----------------------------------------------------------------------------
void CBavpController::ParseRamFileL()
    {
    Log( EFalse, _L("CBavpController::ParseRamFileL() ") );

    RFs rFs;
    RFile ramFile;

    // Connect to file server:
    User::LeaveIfError( rFs.Connect() );
    CleanupClosePushL( rFs );

    if ( !BaflUtils::FileExists( rFs, *iClipInfo->iFileName ) )
        {
        User::Leave( KErrNotFound );
        }

    User::LeaveIfError( ramFile.Open( rFs, *iClipInfo->iFileName, EFileShareAny ) );
    CleanupClosePushL( ramFile );

    TInt size;
    User::LeaveIfError( ramFile. Size( size ) );
    if ( size > KBavpMaxLinkFileSize )
        {
        User::Leave( KErrNotFound );
        }

    HBufC8* urlBuf = HBufC8::NewLC( size );
    TPtr8 ptr = urlBuf->Des();

    // Read file to urlBuf
    User::LeaveIfError( ramFile.Read( ptr ) );

    // Set the iClipInfo->iUrl
    if ( iClipInfo->iUrl )
        {
        delete iClipInfo->iUrl;
        iClipInfo->iUrl = NULL;
        }
    iClipInfo->iUrl = HBufC::NewL( urlBuf->Length() );

    TPtr urlDes = iClipInfo->iUrl->Des();

    // Read to the end of the line that should be a link
    TInt lineChange = urlBuf->LocateF( EKeyLineFeed );
    if ( lineChange == KErrNotFound )
        {
        // No line change was found --> last line had no line change
        // Copy last line to (unicode) aLink
        CnvUtfConverter::ConvertToUnicodeFromUtf8( urlDes, *urlBuf );
        }
    else
        {
        // Set the descriptor to the  end of the line
        if ( (lineChange > 0) && (urlBuf->Des()[lineChange - 1] == EKeyEnter) )
            {
            lineChange--;
            }

        // Copy line to (unicode) url
        CnvUtfConverter::ConvertToUnicodeFromUtf8( urlDes, urlBuf->Left( lineChange ) );
        }

    CleanupStack::PopAndDestroy( 3 ); // ramFile, urlBuf, rFs
    }


#if defined(BRDO_ASX_FF)
// -----------------------------------------------------------------------------
// CBavpController::ParseAsxFileL
// Read the asx file and parse the the url, put into iClipInfo->iUrl
// Only retrieve the first URL found
// -----------------------------------------------------------------------------
void CBavpController::ParseAsxFileL()
    {
    Log( EFalse, _L("CBavpController::ParseAsxFileL() ") );

    TUint urls = 0;
    TPtrC8 url;

    if ( iClipInfo->iUrl )
        {
        delete iClipInfo->iUrl;
        iClipInfo->iUrl = NULL;
        }

    CAsxParser* asxParser = CAsxParser::NewL(*iClipInfo->iFileName);

    CleanupStack::PushL( asxParser );

    if ( asxParser )
        {
        asxParser->GetUrlCount(urls);
        if ( urls > 0 )
            asxParser->GetUrl(1,url);
        }

    iClipInfo->iUrl = HBufC::NewL( url.Length() );
    TPtr urlDes = iClipInfo->iUrl->Des();
    CnvUtfConverter::ConvertToUnicodeFromUtf8( urlDes, url);

    CleanupStack::PopAndDestroy();  // asxparser
    }
#endif // BRDO_ASX_FF


// CBavpController::GetClipRect
// Retrieve the rect position. Ex. not intersect with title pane and status pane
// -----------------------------------------------------------------------------
TRect CBavpController::GetClipRect( TRect aRect )
    {
    TRect clipRect ( aRect );
    Log( EFalse, _L("GetClipRect") );

    AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EMainPane, iNormalScreenRect );

  // Calculate not to cover the bottom or the top of the rect.
  if ( clipRect.iTl.iX < iNormalScreenRect.iTl.iX )
    {
    clipRect.iTl.iX = iNormalScreenRect.iTl.iX;
    }

  if ( clipRect.iTl.iY < iNormalScreenRect.iTl.iY )
    {
    clipRect.iTl.iY = iNormalScreenRect.iTl.iY;
    }

  if ( clipRect.iBr.iY > iNormalScreenRect.iBr.iY )
    {
    clipRect.iBr.iY = iNormalScreenRect.iBr.iY;
    }

  if ( clipRect.iBr.iX > iNormalScreenRect.iBr.iX )
    {
    clipRect.iBr.iX = iNormalScreenRect.iBr.iX - KRectPadding;
    }

    return clipRect;
    }

// -----------------------------------------------------------------------------
// CBavpController::CreateVolumeListDlg
// Create the volume list query dialog
// -----------------------------------------------------------------------------
void CBavpController::CreateVolumeListDlgL()
{
    CEikTextListBox* list = new(ELeave) CAknSinglePopupMenuStyleListBox;
    CleanupStack::PushL( list );

    // Create popup list and PUSH it.
    CAknPopupList* popupList = CAknPopupList::NewL( list,
                                                    R_AVKON_SOFTKEYS_OK_CANCEL,
                                                    AknPopupLayouts::EMenuWindow);
    CleanupStack::PushL( popupList );

    // Initialize listbox.
    list->ConstructL( popupList, CEikListBox::ELeftDownInViewRect );
    list->CreateScrollBarFrameL( ETrue );
    list->ScrollBarFrame()->SetScrollBarVisibilityL( CEikScrollBarFrame::EOff,
                                                     CEikScrollBarFrame::EAuto );

    // Make list items and PUSH it
    CDesCArrayFlat* items =
            CCoeEnv::Static()->ReadDesCArrayResourceL( R_AKNTAPOPLIST_MENU_VOLUME_ITEMS );

    // Set list items.
    CTextListBoxModel* model = list->Model();
    model->SetItemTextArray( items );
    model->SetOwnershipType( ELbmOwnsItemArray );

    // Using the current volume set to determine the index of the volume value
    // to highlight in the list
    TInt volumeIndex( 0 );
    if ( iCurrentVolume == KCRVolume0 )
        {
        volumeIndex = 0;
        }
    else if ( iCurrentVolume == KCRVolume2 )
        {
        volumeIndex = 1;
        }
    else if ( iCurrentVolume == KCRVolume5 )
        {
        volumeIndex = 2;
        }
    else if ( iCurrentVolume == KCRVolume8 )
        {
        volumeIndex = 3;
        }
    else if ( iCurrentVolume == KCRVolume10 )
        {
        volumeIndex = 4;
        }

    // Highlight the current item in the list box
    list->SetCurrentItemIndexAndDraw( volumeIndex );

    HBufC* title = CCoeEnv::Static()->AllocReadResourceLC( R_TEXT_VIDEO_PLUGIN_MEDIA_VOLUME);
    popupList->SetTitleL( *title );
    CleanupStack::PopAndDestroy();  // title

    // Show popup list and then show return value.
    TInt popupOk = popupList->ExecuteLD();
    CleanupStack::Pop();    // popuplist

    // Handle the returned value from the volume list dialog
    if ( popupOk )
        {
        // Check if there is change for the volume select
        if ( volumeIndex != list->CurrentItemIndex() )
            {
            // Set the current index to volume index
            volumeIndex = list->CurrentItemIndex();

            // To match the list box data to the values defined in
            // the Central repository: (mute)0,2,5,8,10(maximum)
            if ( volumeIndex == 0 )
                {
                iCurrentVolume = KCRVolume0;
                }
            else if ( volumeIndex == 1 )
                {
                iCurrentVolume = KCRVolume2;
                }
            else if ( volumeIndex == 2 )
                {
                iCurrentVolume = KCRVolume5;
                }
            else if ( volumeIndex == 3 )
                {
                iCurrentVolume = KCRVolume8;
                }
            else if ( volumeIndex == 4 )
                {
                iCurrentVolume = KCRVolume10;
                }

            Log( EFalse, _L("Return Volume from Player="), iCurrentVolume );

            // Tell the volume handler about our new volume the user selected
            iBavpVolumeHandler->HandleNotifyInt( KBrowserMediaVolumeControl,
                                                 iCurrentVolume );
            }
        }

    CleanupStack::PopAndDestroy();  // list
}

// -----------------------------------------------------------------------------
// CBavpController::IsVideoOrAudioCall
// Check if there is an incoming call
// -----------------------------------------------------------------------------
TBool CBavpController::IsVideoOrAudioCall()
    {
    Log( EFalse, _L("CBavpController::IsVideoOrAudioCall() entered") );

    // Pause if there is video call
    TInt callType;

    RProperty::Get( KPSUidCtsyCallInformation,
                    KCTsyCallType,
                    callType ); // Ignore errors

    return ( callType == EPSCTsyCallTypeH324Multimedia ||
             callType == EPSCTsyCallTypeCSVoice );
    }

// -----------------------------------------------------------------------------
// CBavpController::BaseConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
void CBavpController::BaseConstructL( TBavpMediaType aMediaType,
                                      const TDesC& aFileName )
    {
    Log( EFalse, _L("CBavpController::BaseConstructL enter") );

    // Create Clipinfo
    iClipInfo = new (ELeave) CBavpClipInfo();
    iClipInfo->iMediaType = aMediaType;
    iClipInfo->iFileName = aFileName.AllocL();
    iClipInfo->iUrl = aFileName.AllocL();

    // Watch for incoming calls
    User::LeaveIfError( iIncomingCalls.Attach( KPSUidCtsyCallInformation,
                        KCTsyCallState ) );
    iIncomingCalls.Subscribe( iStatus );

    // Set the AO active. so we will watch for incoming calls, see RunL()
    SetActive();

    // Create Volume Handler to get the initial volume from settings CR, and
    // watch the CR for volume changes (browser or system)
    iBavpVolumeHandler = CBavpVolumeHandler::NewL( this );

    // Create the timer for jumping (skipping) to new position
    iPositionUpdater = CPeriodic::NewL( CActive::EPriorityStandard );

  //Get normal display screen rect - not including title pane and toolbar pane
  AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EMainPane, iNormalScreenRect );
    // If the phone has side volume keys, create an observer for the hardware
    // volume key events
    if ( HAS_HW_VOLUME_KEYS )
        {
        TRAP_IGNORE(iHwKeyEvents = CBavpHwKeyEvents::NewL( *this ) );
        }

    // Add this BavpController instance to the array
    iBavpControllerArray.Append( this );

    Log( EFalse, _L("CBavpController::BaseConstructL exited") );
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::RunL
// If incoming call arrives, handle the audio or video player content.
// If incoming call ends, resume the audio or video player
// -----------------------------------------------------------------------------
void CBavpController::RunL()
    {
    Log( EFalse, _L("CBavpController::RunL() entered") );

    // Do we have an incoming audio or video call
    if ( IsVideoOrAudioCall() )
        {
        // If we are playing content, pause if seekable or stop if streaming
        if ( iCurrentState == EBavpPlaying )
            {
            // Set precall state, so we will resume playing the content,
            // after the call is done
            iPreCallState = EBavpPlaying;

            if ( iClipInfo->iSeekable )
                {
                // Seekable content, pause, if there is an incoming call
                PauseL();
                }
            else
                {
                HandleInComingCallL();
                // Streaming content, stop, if there is an incoming call
                Stop();
                }
            }
        // If we are seeking content, pause it (these states only for seekable)
        else if ( iCurrentState == EBavpFastForwarding ||
                  iCurrentState == EBavpRewinding )
            {
            // Set precall state, so we will not resume playing the content,
            // after the call is done
            iPreCallState = EBavpPaused;

            // Stop the position updater
            if ( iPositionUpdater->IsActive() )
                {
                iPositionUpdater->Cancel();
                }
            // Seekable content, pause, if there is an incoming call
            PauseL();
            }
        }
    else
        {
        // If the prev status was playing and has been paused by a phone call
        // or a video call. Try to get the player to the previous state and
        // invoke play function.
        if ( iPreCallState == EBavpPlaying )
            {
            iPreCallState = EBavpNone;
            PlayL();
            }
        }

    // Listen for the incoming call property changes
    iIncomingCalls.Subscribe( iStatus );

    SetActive();
    }

// -----------------------------------------------------------------------------
// CBavpController::UpdateVolume
// From MBavpVolumeObserver
// Provides CBavpController with the media volume from the VolumeHandler
// -----------------------------------------------------------------------------
void CBavpController::UpdateVolume( TInt aVolume )
    {
    iCurrentVolume = aVolume;

    // Notify Player of volume change
    SetPlayerVolume( iCurrentVolume );
    }

// -----------------------------------------------------------------------------
// CBavpController::BavpHwKeyVolumeChange
// Handles the change in the Volume that is needed as per the Hardware Volume
// key events. Remember, all volume levels in CBavpController are CR based,
// values 0(mute) - 10(max). Use KCRVolumeX values.
// -----------------------------------------------------------------------------
void CBavpController::BavpHwKeyVolumeChange( TInt aVolumeChange )
    {
    TInt newVolume = iCurrentVolume;
    newVolume += aVolumeChange;

    Log( EFalse, _L("CBavpController::BavpHwKeyVolumeChange newVolume="), newVolume );

    // Ensure the volume is within range
    if ( newVolume < KCRVolume0 )
        {
        newVolume = KCRVolume0;
        }
    else if ( newVolume > KCRVolumeMax )
        {
        newVolume = KCRVolumeMax;
        }

    // Piggyback on this call to set the new volume from
    iBavpVolumeHandler->HandleNotifyInt( KBrowserMediaVolumeControl, newVolume );
    }

// -----------------------------------------------------------------------------
// CBavpController::IsAnotherControllerPlaying
// Returns ETrue, on the first controller it finds in playing state
// -----------------------------------------------------------------------------
TBool CBavpController::IsAnotherControllerPlaying()
    {
    if ( iBavpControllerArray.Count() > 1 )
        {
        for ( TInt i( 0 ); i < iBavpControllerArray.Count(); i++ )
            {
            CBavpController* bavpController = iBavpControllerArray[ i ];
            if ( ( this != bavpController ) &&
                 ( bavpController->iCurrentState == EBavpPlaying ) )
                {
                return ETrue;
                }
            }
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CBavpController::PauseOtherControllersPlaying
// This will attempt to pause all of the other controllers (expect this one), if
// the media can't be paused (ie live stream), it will be stopped.
// -----------------------------------------------------------------------------
void CBavpController::PauseOtherControllersPlaying()
    {
    if ( iBavpControllerArray.Count() > 1 )
        {
        for ( TInt i( 0 ); i < iBavpControllerArray.Count(); i++ )
            {
            CBavpController* bavpController = iBavpControllerArray[ i ];
            if ( this != bavpController )
                {
                TRAP_IGNORE( bavpController->PauseL() );
                iBavpView->UpdateView();
                }
            }
        }
    }

// --------------------------------------------------------------------------
// CBavpControllerVideo::HandleMultiInstanceError
// Handle error codes.
// NOTES:
// MMF errors start at -12000, see /epoc32/include/mmf/common/MMFErrors.h
// MMF also returns -1 (KErrNotFound) and few other system-wide errors
// -----------------------------------------------------------------------------
void CBavpController::HandleMultiInstanceError()
    {
    // Handle multiple media object case
    if ( iBavpControllerArray.Count() > 1 && IsAnotherControllerPlaying() )
        {
        if ( iLastCommand == EBavpCmdUnknown )
            {
            // Another controller is playing, and this controller was
            // initializing, so our error is because we can only play one
            // media object at a time. Stop this media and let
            // the first one initialized continue to play.
            //TRAP_IGNORE( PauseL() );
            Stop();
            iBavpView->UpdateView();
            }
        else if ( iLastCommand == EBavpCmdPlay )
            {
            // Another controller is playing, and the user wants to play
            // another media object. Pause (or stop) all of the others and
            // play this one.
            PauseOtherControllersPlaying();
            TRAP_IGNORE( PlayL() );
            }
        }
    }

// -----------------------------------------------------------------------------
// CBavpController::SetOriginalFileName
// Sets the file name. Used for scripting functionality
// -----------------------------------------------------------------------------
void CBavpController::SetOriginalFileName( const TDesC* originalFile )
{
    if ( originalFile )
    {
        iOriginalFileName = originalFile->Alloc();
    }
}

// -----------------------------------------------------------------------------
// CBavpController::SetMimeType
// Sets the mime type. Used for scripting functionality
// -----------------------------------------------------------------------------
void CBavpController::SetMimeType( const TDesC8* mimetype )
{
    if ( mimetype )
    {
        iMimeType = mimetype->Alloc();
    }
}

// -----------------------------------------------------------------------------
// CBavpController::BavpHwKeyCommand
// Handles transport commands (play, pause, ...) from dedicated hardware keys.
// -----------------------------------------------------------------------------
/*
void CBavpController::BavpHwKeyCommand( TBavpCommandIds aCommand )
    {
    // Do some basic checks for the hardware keys
    if ( aCommand == EBavpCmdPlay && iCurrentState == EBavpPlaying )
        {
        // We recieved a play command while playing, therefore make it pause
        aCommand = EBavpCmdPause;
        }

    HandleCommandL( aCommand );
    }
*/

// -----------------------------------------------------------------------------
// HandleGesture(TGestureEvent *gesture)
// -----------------------------------------------------------------------------
TBool CBavpController::HandleGesture(TGestureEvent* /*gesture*/)
    {
    // should be implemented in derrived class.    
    return EFalse;
    }

//  End of File
