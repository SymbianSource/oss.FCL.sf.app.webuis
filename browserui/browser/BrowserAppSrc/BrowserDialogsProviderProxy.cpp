/*
* Copyright (c) 2005-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation of the CBrowserDialogsProviderProxy class.
*
*/


// INCLUDE Files

// User includes
#include "BrowserDialogsProviderProxy.h"
#include "BrowserDialogsProviderBlocker.h"
#include "BrowserWindowFocusNotifier.h"
#include "BrowserWindow.h"
#include "logger.h"

// System includes
#include <BrowserDialogsProviderObserver.h>
#include <BrowserDialogsProvider.h>

// CONSTANTS

//-----------------------------------------------------------------------------
// CBrowserDialogsProviderProxy::CBrowserDialogsProviderProxy
//-----------------------------------------------------------------------------
CBrowserDialogsProviderProxy::CBrowserDialogsProviderProxy(
						    CBrowserDialogsProvider& aDialogsProvider,
						    MBrowserDialogsProviderObserver* aDialogsObserver,
						    CBrowserWindow& aBrowserWindow )
    : iDialogsProvider( aDialogsProvider ),
      iDialogsObserver( aDialogsObserver ),
      iBrowserWindow( aBrowserWindow ),
      iCancelWaitingDialogs( EFalse )
	{
	}


//-----------------------------------------------------------------------------
// CBrowserDialogsProviderProxy::~CBrowserDialogsProviderProxy
//-----------------------------------------------------------------------------
CBrowserDialogsProviderProxy::~CBrowserDialogsProviderProxy()
	{
LOG_ENTERFN("~CBrowserDialogsProviderProxy");
    delete iWinFocusNotifier;
	}


//-----------------------------------------------------------------------------
//	CBrowserDialogsProviderProxy* CBrowserDialogsProviderProxy::NewL
//-----------------------------------------------------------------------------
CBrowserDialogsProviderProxy* CBrowserDialogsProviderProxy::NewL(
						    CBrowserDialogsProvider& aDialogsProvider,
						    MBrowserDialogsProviderObserver* aDialogsObserver,
						    CBrowserWindow& aBrowserWindow  )
	{
	CBrowserDialogsProviderProxy* self = new (ELeave)
	        CBrowserDialogsProviderProxy(   aDialogsProvider,
	                                        aDialogsObserver,
	                                        aBrowserWindow );

	CleanupStack::PushL( self );
	self->ConstructL( );
	CleanupStack::Pop( self );   // self
	return self;
	}

//-----------------------------------------------------------------------------
//  CBrowserDialogsProviderProxy::ConstructL
//
//-----------------------------------------------------------------------------
void CBrowserDialogsProviderProxy::ConstructL()
	{
	// Keeps track of blocked dialogs
	iWinFocusNotifier = CBrowserWindowFocusNotifier::NewL();
	}


//-----------------------------------------------------------------------------
//  CBrowserDialogsProviderProxy::DialogNotifyErrorL
//
//-----------------------------------------------------------------------------
void CBrowserDialogsProviderProxy::DialogNotifyErrorL( TInt aErrCode )
	{
	LOG_ENTERFN("CBrowserDialogsProviderProxy::DialogNotifyErrorL");

	// Forward dialog to DialogsProvider if it has not been cancelled
	if ( iBrowserWindow.IsWindowActive() && !iCancelWaitingDialogs )
    	{
    	iDialogsProvider.DialogNotifyErrorL( aErrCode );
    	if ( iDialogsObserver )
    	    {
    	    iDialogsObserver->ReportDialogEventL(
                                MBrowserDialogsProviderObserver::ENotifyError,
        	                            aErrCode );
    	    }
        }
//    LOG_LEAVEFN("CBrowserDialogsProviderProxy::DialogNotifyErrorL");
	}


//-----------------------------------------------------------------------------
//  CBrowserDialogsProviderProxy::DialogNotifyHttpErrorL
//-----------------------------------------------------------------------------
void CBrowserDialogsProviderProxy::DialogNotifyHttpErrorL(
								            TInt aErrCode, const TDesC& aUri )
	{
	LOG_ENTERFN("CBrowserDialogsProviderProxy::DialogNotifyHttpErrorL");

	// Forward dialog to DialogsProvider if it has not been cancelled
	if ( iBrowserWindow.IsWindowActive() && !iCancelWaitingDialogs )
    	{
    	iDialogsProvider.DialogNotifyHttpErrorL( aErrCode, aUri );
    	if ( iDialogsObserver )
    	    {
    	    iDialogsObserver->ReportDialogEventL(
    	                    MBrowserDialogsProviderObserver::ENotifyHttpError,
                            NULL );
    	    }
    	}
//    LOG_LEAVEFN("CBrowserDialogsProviderProxy::DialogNotifyHttpErrorL");
	}


//-----------------------------------------------------------------------------
//  CBrowserDialogsProviderProxy::DialogFileSelectLC
//-----------------------------------------------------------------------------
TBool CBrowserDialogsProviderProxy::DialogFileSelectLC(
                                                    const TDesC& aStartPath,
													const TDesC& aRootPath,
													HBufC*& aSelectedFileName )
	{
	LOG_ENTERFN("CBrowserDialogsProviderProxy::DialogFileSelectLC");
	TBool retVal( EFalse );

	// Forward dialog to DialogsProvider if it has not been cancelled
	if ( iBrowserWindow.IsWindowActive() && !iCancelWaitingDialogs )
    	{
        CEikButtonGroupContainer* currentCba = CEikButtonGroupContainer::Current();
        TBool softKeysVisible( ETrue );
        if ( currentCba )
            {
            softKeysVisible = currentCba->IsVisible();
            }

    	retVal = iDialogsProvider.DialogFileSelectLC( aStartPath,
    	                                                    aRootPath,
    	                                                    aSelectedFileName );

        // If the softkeys were not visible before, then make sure they are not 
        // visible after. 
        if ( currentCba && !softKeysVisible )
            {
            currentCba->MakeVisible( EFalse );
            }
    	}

//	LOG_LEAVEFN("CBrowserDialogsProviderProxy::DialogFileSelectLC");
	return retVal;
	}

//-----------------------------------------------------------------------------
//  CBrowserDialogsProviderProxy::DialogSelectOptionL
//-----------------------------------------------------------------------------
TBool CBrowserDialogsProviderProxy::DialogSelectOptionL(
								const TDesC& aTitle,
								TBrCtlSelectOptionType aBrCtlSelectOptionType,
								CArrayFix<TBrCtlSelectOptionData>& aOptions )
	{
	LOG_ENTERFN("CBrowserDialogsProviderProxy::DialogSelectOptionL");
	TBool retVal( EFalse );

	// Forward dialog to DialogsProvider if it has not been cancelled
	if ( iBrowserWindow.IsWindowActive() && !iCancelWaitingDialogs )
	    {
        CEikButtonGroupContainer* currentCba = CEikButtonGroupContainer::Current();
        TBool softKeysVisible( ETrue );
        if ( currentCba )
            {
            softKeysVisible = currentCba->IsVisible();
            }

	    retVal = iDialogsProvider.DialogSelectOptionL(
	                                                aTitle,
	                                                aBrCtlSelectOptionType,
	                                                aOptions );
        // If the softkeys were not visible before, then make sure they are not 
        // visible after. 
        if ( currentCba && !softKeysVisible )
            {
            currentCba->MakeVisible( EFalse );
            }
	    }

//    LOG_LEAVEFN("CBrowserDialogsProviderProxy::DialogSelectOptionL");
	return retVal;
	}


//-----------------------------------------------------------------------------
// CBrowserDialogsProviderProxy::DialogUserAuthenticationLC
//-----------------------------------------------------------------------------
TBool CBrowserDialogsProviderProxy::DialogUserAuthenticationLC(
												const TDesC& aUrl,
												const TDesC& aRealm,
												const TDesC& aDefaultUserName,
												HBufC*& aReturnedUserName,
												HBufC*& aReturnedPasswd,
												TBool aBasicAuthentication )
	{
	LOG_ENTERFN("CBrowserDialogsProviderProxy::DialogUserAuthenticationLC");
	TBool retVal( EFalse );

	// Forward dialog to DialogsProvider if it has not been cancelled
	if ( iBrowserWindow.IsWindowActive() && !iCancelWaitingDialogs )
	    {
        CEikButtonGroupContainer* currentCba = CEikButtonGroupContainer::Current();
        TBool softKeysVisible( ETrue );
        if ( currentCba )
            {
            softKeysVisible = currentCba->IsVisible();
            }

	    retVal = iDialogsProvider.DialogUserAuthenticationLC(
	                                                aUrl,
	                                                aRealm,
	                                                aDefaultUserName,
	                                                aReturnedUserName,
	                                                aReturnedPasswd,
	                                                aBasicAuthentication );
	    if ( iDialogsObserver )
	        {
            iDialogsObserver->ReportDialogEventL(
                        MBrowserDialogsProviderObserver::EUserAuthentication,
                        ( TInt ) retVal );
	        }

        // If the softkeys were not visible before, then make sure they are not 
        // visible after. 
        if ( currentCba && !softKeysVisible )
            {
            currentCba->MakeVisible( EFalse );
            }
	    }

//	LOG_LEAVEFN("CBrowserDialogsProviderProxy::DialogUserAuthenticationLC");
	return retVal;
	}


//-----------------------------------------------------------------------------
//  CBrowserDialogsProviderProxy::DialogNoteL
//-----------------------------------------------------------------------------
void CBrowserDialogsProviderProxy::DialogNoteL( const TDesC& aMessage )
	{
	LOG_ENTERFN("CBrowserDialogsProviderProxy::DialogNoteL");

    // Forward dialog to DialogsProvider if it has not been cancelled
	if ( iBrowserWindow.IsWindowActive() && !iCancelWaitingDialogs )
	    {
	    iDialogsProvider.DialogNoteL( aMessage );
	    }
//	LOG_LEAVEFN("CBrowserDialogsProviderProxy::DialogNoteL");
	}


//-----------------------------------------------------------------------------
//  CBrowserDialogsProviderProxy::DialogAlertL
//-----------------------------------------------------------------------------
void CBrowserDialogsProviderProxy::DialogAlertL( const TDesC& aTitle,
											        const TDesC& aMessage )
	{
	LOG_ENTERFN("CBrowserDialogsProviderProxy::DialogAlertL");

	// Forward dialog to DialogsProvider if it has not been cancelled
	if ( iBrowserWindow.IsWindowActive() && !iCancelWaitingDialogs )
	    {
	    iDialogsProvider.DialogAlertL( aTitle, aMessage );
	    }
//	LOG_LEAVEFN("CBrowserDialogsProviderProxy::DialogAlertL");
	}


//-----------------------------------------------------------------------------
//  CBrowserDialogsProviderProxy::DialogConfirmL
//-----------------------------------------------------------------------------
TBool CBrowserDialogsProviderProxy::DialogConfirmL( const TDesC& aTitle,
												    const TDesC& aMessage,
												    const TDesC& aYesMessage,
												    const TDesC& aNoMessage )
	{
	LOG_ENTERFN("CBrowserDialogsProviderProxy::DialogConfirmL");
	TBool retVal( EFalse );

	// Forward dialog to DialogsProvider if it has not been cancelled
	if ( iBrowserWindow.IsWindowActive() && !iCancelWaitingDialogs )
	    {
        CEikButtonGroupContainer* currentCba = CEikButtonGroupContainer::Current();
        TBool softKeysVisible( ETrue );
        if ( currentCba )
            {
            softKeysVisible = currentCba->IsVisible();
            }

	    retVal = iDialogsProvider.DialogConfirmL( aTitle, aMessage,
	                                                aYesMessage, aNoMessage );

        // If the softkeys were not visible before, then make sure they are not 
        // visible after. 
        if ( currentCba && !softKeysVisible )
            {
            currentCba->MakeVisible( EFalse );
            }
	    }

//    LOG_LEAVEFN("CBrowserDialogsProviderProxy::DialogConfirmL");
    return retVal;
	}

//-----------------------------------------------------------------------------
// CBrowserDialogsProviderProxy::DialogPromptLC
//-----------------------------------------------------------------------------
TBool CBrowserDialogsProviderProxy::DialogPromptLC( const TDesC& aTitle,
												const TDesC& aMessage,
												const TDesC& aDefaultInput,
												HBufC*& aReturnedInput )
	{
	LOG_ENTERFN("CBrowserDialogsProviderProxy::DialogPromptLC");
	TBool retVal( EFalse );
    
	// Forward dialog to DialogsProvider if it has not been cancelled
	if ( iBrowserWindow.IsWindowActive() && !iCancelWaitingDialogs )
	    {
        CEikButtonGroupContainer* currentCba = CEikButtonGroupContainer::Current();
        TBool softKeysVisible( ETrue );
        if ( currentCba )
            {
            softKeysVisible = currentCba->IsVisible();
            }

	    retVal = iDialogsProvider.DialogPromptLC( aTitle, aMessage,
                                            aDefaultInput, aReturnedInput );

        // If the softkeys were not visible before, then make sure they are not 
        // visible after. 
        if ( currentCba && !softKeysVisible )
            {
            currentCba->MakeVisible( EFalse );
            }
	    }

//    LOG_LEAVEFN("CBrowserDialogsProviderProxy::DialogPromptLC");
    return retVal;
	}

//-----------------------------------------------------------------------------
//  CBrowserDialogsProviderProxy::DialogDownloadObjectL
//-----------------------------------------------------------------------------
TBool CBrowserDialogsProviderProxy::DialogDownloadObjectL(
										CBrCtlObjectInfo* aBrCtlObjectInfo )
	{
	LOG_ENTERFN("CBrowserDialogsProviderProxy::DialogDownloadObjectL");
	TBool retVal( EFalse );

	if (iBrowserWindow.IsWindowActive())
	    {
	    retVal = iDialogsProvider.DialogDownloadObjectL( aBrCtlObjectInfo );
	    }
//	LOG_LEAVEFN("CBrowserDialogsProviderProxy::DialogDownloadObjectL");
	return retVal;
	}

//-----------------------------------------------------------------------------
// CBrowserDialogsProviderProxy::DialogDisplayPageImagesL
//-----------------------------------------------------------------------------
void CBrowserDialogsProviderProxy::DialogDisplayPageImagesL(
							CArrayFixFlat<TBrCtlImageCarrier>& aPageImages )
	{
	LOG_ENTERFN("CBrowserDialogsProviderProxy::DialogDisplayPageImagesL");

	// Forward dialog to DialogsProvider if it has not been cancelled
	if ( iBrowserWindow.IsWindowActive() && !iCancelWaitingDialogs )
	    {
	    iDialogsProvider.DialogDisplayPageImagesL( aPageImages );
	    }
//	LOG_LEAVEFN("CBrowserDialogsProviderProxy::DialogDisplayPageImagesL");
	}


//-----------------------------------------------------------------------------
//  CBrowserDialogsProviderProxy::CancelAll
//-----------------------------------------------------------------------------
//
void CBrowserDialogsProviderProxy::CancelAll()
	{
	LOG_ENTERFN("CBrowserDialogsProviderProxy::CancelAll");
	// Dialogs should be flushed, but not displayed
    iCancelWaitingDialogs = ETrue;
    iDialogsProvider.CancelAll();
	iWinFocusNotifier->FlushAOStatusArray();
//	LOG_LEAVEFN("CBrowserDialogsProviderProxy::CancelAll");
	}
	
//-----------------------------------------------------------------------------
//  CBrowserDialogsProviderProxy::DialogMimeFileSelectLC
//-----------------------------------------------------------------------------
TBool CBrowserDialogsProviderProxy::DialogMimeFileSelectLC(
													HBufC*& aSelectedFileName,
													const TDesC& aMimeType )
	{
	LOG_ENTERFN("CBrowserDialogsProviderProxy::DialogMimeFileSelectLC");
	TBool retVal( EFalse );

	// Forward dialog to DialogsProvider if it has not been cancelled
	if ( iBrowserWindow.IsWindowActive() && !iCancelWaitingDialogs )
    	{
        CEikButtonGroupContainer* currentCba = CEikButtonGroupContainer::Current();
        TBool softKeysVisible( ETrue );
        if ( currentCba )
            {
            softKeysVisible = currentCba->IsVisible();
            }

    	retVal = iDialogsProvider.DialogMimeFileSelectLC( aSelectedFileName,
    	                                                  aMimeType );
        // If the softkeys were not visible before, then make sure they are not 
        // visible after. 
        if ( currentCba && !softKeysVisible )
            {
            currentCba->MakeVisible( EFalse );
            }
    	}

//	LOG_LEAVEFN("CBrowserDialogsProviderProxy::DialogMimeFileSelectLC");
	return retVal;
	}

//-----------------------------------------------------------------------------
//  CBrowserDialogsProviderProxy::ShowTooltipL
//-----------------------------------------------------------------------------
//
void CBrowserDialogsProviderProxy::ShowTooltipL(    const TDesC& aText,
                                                    TInt aDuration,
                                                    TInt aDelay )
    {
    LOG_ENTERFN("CBrowserDialogsProviderProxy::ShowTooltipL");

    if ( iBrowserWindow.IsWindowActive() && !iCancelWaitingDialogs )
        {
        iDialogsProvider.ShowTooltipL(   aText, aDuration, aDelay );
        }
//	LOG_LEAVEFN("CBrowserDialogsProviderProxy::ShowTooltipL");
    }

//-----------------------------------------------------------------------------
//  CBrowserDialogsProviderProxy::UploadProgressNoteL
//-----------------------------------------------------------------------------
//
void CBrowserDialogsProviderProxy::UploadProgressNoteL(
                                TInt32 aTotalSize,
                                TInt32 aChunkSize,
                                TBool aIsLastChunk,
                                MBrowserDialogsProviderObserver* aObserver )
    {
    LOG_ENTERFN("CBrowserDialogsProviderProxy::UploadProgressNoteL");

    // Forward dialog to DialogsProvider if it has not been cancelled
	if ( iBrowserWindow.IsWindowActive() && !iCancelWaitingDialogs )
	    {
	    iDialogsProvider.UploadProgressNoteL(   aTotalSize,
	                                            aChunkSize,
	                                            aIsLastChunk,
	                                            aObserver );
	    }

//    LOG_LEAVEFN("CBrowserDialogsProviderProxy::UploadProgressNoteL");
    }

//-----------------------------------------------------------------------------
//  CBrowserDialogsProviderProxy::WindowActivated()
//-----------------------------------------------------------------------------
//
void CBrowserDialogsProviderProxy::WindowActivated()
    {
    LOG_ENTERFN("CBrowserDialogsProviderProxy::WindowActivated");
    // Window is now topmost, all waiting dialogs should be shown in turn
    iWinFocusNotifier->OnFocusGained();
//    LOG_LEAVEFN("CBrowserDialogsProviderProxy::WindowActivated");
    }

//  End of File
