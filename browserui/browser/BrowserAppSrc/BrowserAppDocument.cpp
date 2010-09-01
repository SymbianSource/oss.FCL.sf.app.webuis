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
*     Browser app document
*     
*
*/

#include "BrowserAppDocument.h"
#include "BrowserAppUi.h"
#include <favouritesitem.h>
#include <brctlinterface.h>
#include "CommonConstants.h"

CBrowserAppDocument::CBrowserAppDocument(CEikApplication& aApp): CAknDocument(aApp)
{
	iFolderToOpen = KFavouritesRootUid;
}

CBrowserAppDocument::~CBrowserAppDocument()
{
	if ( iUrlToOpen )
		{
		delete iUrlToOpen;
		iUrlToOpen = NULL;
		}
}

CEikAppUi* CBrowserAppDocument::CreateAppUiL()
{
	return new (ELeave) CBrowserAppUi;
}

TInt CBrowserAppDocument::GetFolderToOpen() const
{
    return iFolderToOpen;
}

HBufC* CBrowserAppDocument::GetUrlToOpen() const
{
    return iUrlToOpen;
}

MDownloadedContentHandler *CBrowserAppDocument::GetDownloadedContentHandler() const
{
    return iDownloadedContentHandler;
}

CFileStore* CBrowserAppDocument::OpenFileL(TBool /*aDoOpen*/, const TDesC& aFileName, RFs& /*aFs*/)
{
    TInt len = aFileName.Length();
    delete iUrlToOpen;
    iUrlToOpen = NULL;
    iUrlToOpen = HBufC::NewL(len);
    *iUrlToOpen = aFileName;
	HBufC8 *url8 = HBufC8::NewL(len);

	url8->Des().Copy( aFileName );
    //STATIC_CAST(CBrowserAppUi*, iAppUi)->FetchL(*iUrlToOpen);
	STATIC_CAST(CBrowserAppUi*, iAppUi)->ParseAndProcessParametersL( url8->Des() );
	delete url8;
    return NULL;
}

// From CAknDocument
void CBrowserAppDocument::OpenFileL( CFileStore*& aFileStore, RFile& aFile )
	{
    CBrowserAppUi* appUi = STATIC_CAST(CBrowserAppUi*, iAppUi);
	appUi->InitBrowserL();	
    
    TBuf<KMaxDataTypeLength> dataTypeDes;
    
    // See if Browser UI should handle this open file or not
    if(appUi->RecognizeUiFileL(aFile, dataTypeDes))
    	{
    	// confirm the filehandling with the user,
    	// retrieving filename from the RFILE
		TFileName fileName;
		User::LeaveIfError(aFile.Name(fileName));
		if(appUi->ConfirmUiHandlingL(fileName, dataTypeDes))
			{
			// need to copy file to a temp file, then pass off to Feeds
			// prepend the path to the fileName
			TFileName filePath = _L("c:\\system\\temp\\");
			filePath.Append(fileName);
			appUi->CopyUiRFileToFileL(aFile,filePath);
			
			// Pass off to appUi to set up views, etc.
			appUi->HandleUiFileL(filePath, dataTypeDes);
			}
		else
			{
			appUi->HandleCommandL( EWmlCmdCloseBrowser );
			}
       	}
    else
    	{
    	// send to content view
    	appUi->WaitCVInit();
    	appUi->SetLastActiveViewId( KUidBrowserContentViewId );
        // view activation and bringing the browser to foreground are two 
        // distinct actions.
        appUi->ActivateLocalViewL( KUidBrowserContentViewId );
    	appUi->HandleForegroundEventL( ETrue );
    
    	TFileName fileName;
    	User::LeaveIfError( aFile.FullName( fileName ) );
    	if ( fileName.Find( KMailerStoreUid )  != KErrNotFound )
    		{
    		appUi->CopyUiRFileToFileL( aFile,KAttachment() );
    		appUi->BrCtlInterface().LoadFileL( KAttachment() );
    		}
    	else
    		{
    		appUi->BrCtlInterface().LoadFileL( aFile );
    		}
    	
    	}
	aFileStore = 0;
	aFile.Close();
	}

TBrowserOverriddenSettings* CBrowserAppDocument::GetOverriddenSettings()
    {
    return iOverriddenSettings;
    }
