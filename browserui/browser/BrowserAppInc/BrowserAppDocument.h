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
*     Browser document class
*     
*
*/

#ifndef __BROWSERAPPDOCUMENT_H
#define __BROWSERAPPDOCUMENT_H

#include <browseroverriddensettings.h>
#include "BrowserUtil.h"
#include <akndoc.h>

//CONSTANTS
_LIT( KAttachment,"c:\\system\\temp\\Attachment.html");
_LIT( KMailerStoreUid,"1000484b");

class MDownloadedContentHandler;

// CAknDocument
class CBrowserAppDocument : public CAknDocument
{
public:
    CBrowserAppDocument(CEikApplication& aApp);
	virtual ~CBrowserAppDocument();

    inline void SetFolderToOpen(TInt aFolderId) { iFolderToOpen = aFolderId; }

    /**
    * Set the MDownloadedContentHandler implementation.
    * @param aObserver The implementation.
    * @return None.
    */
    inline void SetDownloadedContentHandler(MDownloadedContentHandler *aObserver);

    inline void SetIsContentHandlerRegistered( TBool aIsContentHandlerRegistered ) 
    	{ iIsContentHandlerRegistered = aIsContentHandlerRegistered; }
    
    inline TBool IsContentHandlerRegistered() const 
    	{ return iIsContentHandlerRegistered; };

    TInt GetFolderToOpen() const;

    HBufC* GetUrlToOpen() const;

    MDownloadedContentHandler *GetDownloadedContentHandler() const;
    
    CFileStore* OpenFileL(TBool aDoOpen, const TDesC& aFileName, RFs& aFs);

    void OpenFileL(CFileStore*& aFileStore, RFile& aFile);
	    
    TBrowserOverriddenSettings* GetOverriddenSettings();

    inline void SetOverriddenSettings(TBrowserOverriddenSettings* aSettings){ iOverriddenSettings = aSettings; };

protected:
	CEikAppUi* CreateAppUiL();

private:
    MDownloadedContentHandler *iDownloadedContentHandler;
    TInt iFolderToOpen;
    TBool iIsContentHandlerRegistered;
    HBufC* iUrlToOpen;
    TBrowserOverriddenSettings* iOverriddenSettings;
};

#include "BrowserAppDocument.inl"

#endif