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
*      Utilities.
*      
*
*/


// INCLUDE FILES

#include "logger.h"

#include <e32base.h>
#include <favouriteslimits.h>
#include <favouritesitem.h>
#include <uri16.h>
#include <SysUtil.h>
#include <ErrorUi.h>

#include "BrowserUtil.h"
#include "Browser.hrh"

#include "CommonConstants.h"
#include "ApiProvider.h"
#include <commdb.h>
#include <aputils.h>
#include "commsmodel.h"
#include "preferences.h"
#include <FeatMgr.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// Util::Panic
// ---------------------------------------------------------
//
void Util::Panic( Util::TPanicReason aReason )
    {
    _LIT( KAppName, "WmlBrowser" );
	User::Panic( KAppName, aReason );
    }

// ---------------------------------------------------------
// Util::SanityCheckL
// ---------------------------------------------------------
//
void Util::SanityCheckL()
    {
#ifdef _DEBUG

    // Check if the Engine limits for bookmarks match editing control sizes.
    // Cannot do this compile time, because Engine limits are TInts instead of
    // #define-s, so we must do run-time check (debug build only). Naturally,
    // the conditional expression here is constant. Avoid warnings by using
    // dummy variables (maxEngine...).
    TInt maxEngineUrl = KFavouritesMaxUrl;
    TInt maxEngineUsername = KFavouritesMaxUserName;
    TInt maxEnginePassword = KFavouritesMaxPassword;
    TInt maxEngineBookmarkName = KFavouritesMaxName;

    if (
        KFavouritesMaxUrlDefine != maxEngineUrl ||
        KFavouritesMaxUsernameDefine != maxEngineUsername ||
        KFavouritesMaxPasswordDefine != maxEnginePassword ||
        KFavouritesMaxBookmarkNameDefine != maxEngineBookmarkName
       )
        {
        Panic( ELimitMismatch );
        }

#endif
    }

// ---------------------------------------------------------
// Util::RetreiveUsernameAndPasswordFromUrlL
// ---------------------------------------------------------
//
void Util::RetreiveUsernameAndPasswordFromUrlL
	( const TDesC& aUrl, CFavouritesItem& aItem )
	{
    /// Empty string.
    _LIT( KEmptyBuf, "" ); 
/*	CUrl* url = NULL;
	TRAPD( error, url = CUrl::NewL( aUrl ) );
	if( error == KErrNone )
		{ 
		CleanupStack::PushL( url );
*/
    TUriParser16 url;
    url.Parse(aUrl);
	TPtrC userInfo = url.Extract( EUriUserinfo );
    TPtrC userName ;
    TPtrC pass ;
    userName.Set( KEmptyBuf );
    pass.Set( KEmptyBuf );
	if( userInfo.Length() )
        { 
        TInt position = userInfo.LocateF( ':' );
	    if( ( position != KErrNotFound ) )
            {
            userName.Set( userInfo.Left( position ) );
	        aItem.SetUserNameL( userName );
            pass.Set( userInfo.Right( userInfo.Length()-position-1  ) );
    		aItem.SetPasswordL( pass );
            }
	    }


	HBufC* parsedUrl = HBufC::NewLC( url.UriDes().Length() );
	// get the pure Url (without username & password)
	TInt position = url.UriDes().LocateF( '@' );
	if( ( position != KErrNotFound ) && ( userName.Length() )
           && ( url.UriDes().Find(userInfo) != KErrNotFound) )
		{
		parsedUrl->Des().Copy( url.Extract( EUriScheme ) );
		_LIT( KDoubleSlash, "://" );
		parsedUrl->Des().Append( KDoubleSlash );
		parsedUrl->Des().Append( url.UriDes().Right
			( url.UriDes().Length() - position - 1  ) );
		}
	else
		{
		parsedUrl->Des().Copy( url.UriDes() );
    	}
    if( parsedUrl->Length() )
		{
		aItem.SetUrlL( *parsedUrl );
		}
	CleanupStack::PopAndDestroy( 1 ); //  parsedUrl
	}


// ---------------------------------------------------------
// Util::StripUrl
// ---------------------------------------------------------
//
TPtrC Util::StripUrl( const TDesC& aUrl )
	{
	TUriParser url;
	TInt startPos;
	TInt endPos;
	
	url.Parse ( aUrl );

	//Return parsed url only if we have a scheme. Otherwise return full url
	if ( url.IsPresent ( EUriScheme ) )
		{
		startPos = url.Extract ( EUriScheme ).Length();
		//jump over the :// chars (or a mistyped version like :/ or :)
		while ( startPos < url.UriDes().Length() &&
				( aUrl.Mid( startPos, 1) == _L("/") || 
				  aUrl.Mid( startPos, 1) == _L(":") ) )
			{
			startPos++;
			}
		endPos = url.UriDes().Length() - url.Extract ( EUriFragment ).Length();
		}
	else
		{
		startPos = 0;
		endPos = url.UriDes().Length();
		}
	
	return aUrl.Mid( startPos, endPos - startPos );
	}

// ---------------------------------------------------------
// Util::UrlFromFileNameLC
// ---------------------------------------------------------
//
HBufC* Util::UrlFromFileNameLC( const TDesC& aFileName )
	{
    HBufC* buf = HBufC::NewLC
        ( aFileName.Length() + KWmlValueFileSlashSlashSlashStr().Length() );

    HBufC* url = HBufC::NewLC
        ( aFileName.Length() + KWmlValueFileSlashSlashSlashStr().Length() );
    url->Des().Append( KWmlValueFileSlashSlashSlashStr );
    url->Des().Append( aFileName );

    for ( TInt i = 0; i < url->Length(); i++ )
        {
        if ( !url->Mid( i, 1 ).Compare( KWmlBackSlash ) )
            {
            buf->Des().Append( TChar('/') );
            }
        else
            {
            buf->Des().Append( url->Mid( i, 1 ) );
            }
        }
    CleanupStack::PopAndDestroy();  // url
    return buf;
	}

// ---------------------------------------------------------
// Util::FFSSpaceBelowCriticalLevelL
// ---------------------------------------------------------
//
TBool Util::FFSSpaceBelowCriticalLevelL
        ( TBool aShowErrorNote, TInt aBytesToWrite /*=0*/ )
    {
    TBool ret( EFalse );
    if ( SysUtil::FFSSpaceBelowCriticalLevelL
                ( &(CCoeEnv::Static()->FsSession()), aBytesToWrite ) )
        {
        ret = ETrue;
        if ( aShowErrorNote )
            {
            CErrorUI* errorUi = CErrorUI::NewLC( *(CCoeEnv::Static()) );
            errorUi->ShowGlobalErrorNoteL( KErrDiskFull );
            CleanupStack::PopAndDestroy();  // errorUi
            }
        }
    return ret;
    }

// ---------------------------------------------------------
// Util::CheckBookmarkApL
// ---------------------------------------------------------
//
TBool Util::CheckBookmarkApL(
        const MApiProvider& aApiProvider,
        const TFavouritesWapAp& aItem )
    {
    TBool apValid( EFalse );

    if ( !aItem.IsNull() && !aItem.IsDefault() )
        {
        CCommsDatabase& db = aApiProvider.CommsModel().CommsDb();
        
        CApUtils* apUtils = CApUtils::NewLC( db );
        // if error, wapId is not found.
        TRAPD( err, apValid = apUtils->WapApExistsL( aItem.ApId() ) );
        if ( /* !apValid || */ err )
            {
            apValid = EFalse;
            }
        CleanupStack::PopAndDestroy();  // apUtils
        }
    else 
        {
        apValid = ETrue;
        }
    return apValid;
    }

// ---------------------------------------------------------
// Util::IsValidAPFromParamL
// ---------------------------------------------------------
//
TBool Util::IsValidAPFromParamL(
        const MApiProvider& aApiProvider,
        TUint32 aIAPid )
    {
    CCommsDatabase& db = aApiProvider.CommsModel().CommsDb();
    CApUtils* apUtils = CApUtils::NewLC( db );
    TBool retVal = apUtils->WapApExistsL( aIAPid );
    CleanupStack::PopAndDestroy();  // apUtils
    return retVal;
    }

// ---------------------------------------------------------
// Util::CheckApValidityL
// ---------------------------------------------------------
//
TBool Util::CheckApValidityL(
        const MApiProvider& aApiProvider,
        const TUint32 aItem )
    {
    CCommsDatabase& db = aApiProvider.CommsModel().CommsDb();
    CApUtils* apUtils = CApUtils::NewLC( db );
    TBool apValid( EFalse );

    // if error, IapId is not found.
    TRAPD( err, apUtils->IapIdFromWapIdL( aItem ) );
    if( err == KErrNone )
        {
        apValid = ETrue;
        }
    CleanupStack::PopAndDestroy(); // apUtils

    return apValid;
    }

// ---------------------------------------------------------
// Util::IapIdFromWapIdL
// ---------------------------------------------------------
//
TUint32 Util::IapIdFromWapIdL( const MApiProvider& aApiProvider, const TUint32 aItem )
{
    CCommsDatabase& db = aApiProvider.CommsModel().CommsDb();
    CApUtils* apUtils = CApUtils::NewLC( db );
    TUint32 ap = apUtils->IapIdFromWapIdL( aItem );
    CleanupStack::PopAndDestroy();  // apUtil
    return ap;
}

// ---------------------------------------------------------
// Util::WapIdFromIapIdL
// ---------------------------------------------------------
//
TUint32 Util::WapIdFromIapIdL( const MApiProvider& aApiProvider, const TUint32 aItem )
{
    CCommsDatabase& db = aApiProvider.CommsModel().CommsDb();
    CApUtils* apUtils = CApUtils::NewLC( db );
    TUint32 ap = apUtils->WapIdFromIapIdL( aItem );
    CleanupStack::PopAndDestroy();  // apUtil
    return ap;
}

// ---------------------------------------------------------
// Util::EncodeSpaces
// ---------------------------------------------------------
//
void Util::EncodeSpaces(HBufC*& aString)
    {
	_LIT(KSpace," ");
	_LIT(KSpaceEncoded,"%20");

    TInt space(KErrNotFound);
    TInt spaces(0);
    TPtr ptr = aString->Des();
    
    //Trim leading and trailing spaces in URL before encoding spaces inside URL
    ptr.Trim();
        
    for (TInt i = ptr.Length()-1; i > 0 ; i--)
        {
        if (ptr[i] == ' ')
            {
            spaces++;
            space = i;
            }
        }
        
    if (spaces)
        {
        TInt newLen = aString->Length() + (spaces * KSpaceEncoded().Length());
        TRAPD(err,aString = aString->ReAllocL(newLen));
        if (err != KErrNone)
            {
            return;  // aString was not changed
            }
        ptr.Set( aString->Des() );
       
        }
        
	while (space != KErrNotFound)
    	{
    	ptr.Replace(space,1,KSpaceEncoded);
    	space = ptr.Find(KSpace);
    	}
        
    }

// ---------------------------------------------------------
// Util::AllocateUrlWithSchemeL
// ---------------------------------------------------------
//
HBufC* Util::AllocateUrlWithSchemeL( const TDesC& aUrl )
    {
    _LIT( KBrowserDefaultScheme, "http://" );
    TBool addDefaultScheme( EFalse );
    TInt len = aUrl.Length(); 

    // Do not check return value from parse. If the caller wants to allocate a
    // badly formed url then let them do so.
    TUriParser url;
    url.Parse( aUrl ); 

    if( !url.IsPresent( EUriScheme ) )
        { 
        addDefaultScheme = ETrue; 
        len = len + KBrowserDefaultScheme().Length();
        }

    HBufC* urlBuffer = HBufC::NewL( len );
    TPtr ptr = urlBuffer->Des();

    if ( addDefaultScheme )
        {
        ptr.Copy( KBrowserDefaultScheme );
        }
    ptr.Append( aUrl );

    // Handle rare case that the url needs escape encoding.
    // Below function will realloc enough space for encoded urlBuffer.
    Util::EncodeSpaces( urlBuffer );

    return urlBuffer;
    }


// End of File
