/*
* Copyright (c) 2002-2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Inline methods for CBrowserAppDocument.
*
*/


#ifndef BROWSER_APP_DOCUMENT_INL
#define BROWSER_APP_DOCUMENT_INL

// CONSTANTS
_LIT( KBrowserAppDocPanicString, "BrowserAppDoc" );

// ============================ MEMBER FUNCTIONS ==============================

// ----------------------------------------------------------------------------
// CBrowserAppDocument::SetDownloadedContentHandler
// Implementation info:
// Only one of the DownloadedContentHandler is allowed to be set the same time!
// The following example will cause Panic:
//   browserAppDocument->SetDownloadedContentHandler( this );
//   browserAppDocument->SetFileDownloadedContentHandler( that );
// It should be used like:
//   browserAppDocument->SetDownloadedContentHandler( NULL );
//   browserAppDocument->SetFileDownloadedContentHandler( this );
// ----------------------------------------------------------------------------
//
inline void CBrowserAppDocument::SetDownloadedContentHandler
            ( MDownloadedContentHandler* aObserver )
    {
    if ( aObserver != NULL )
        {
        __ASSERT_ALWAYS( iDownloadedContentHandler == NULL, 
            User::Panic( KBrowserAppDocPanicString, 
            Util::EFileDownloadedContentHandlerAlreadyInitialized ) );
        }
    iDownloadedContentHandler = aObserver;
    }

#endif /* BROWSER_APP_DOCUMENT_INL */

// End of File
