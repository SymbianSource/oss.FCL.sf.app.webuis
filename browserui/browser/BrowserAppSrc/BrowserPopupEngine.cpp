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
*      Implementation of popupblocking
*
*
*/


// INCLUDES
#include <Uri16.h>
#include "BrowserPopupEngine.h"
#include <f32file.h>
#include "logger.h"
#include "BrowserUtil.h"

// CONSTANTS
_LIT( KWhiteListFileName, "bpopupwl.db" );
_LIT( KWhiteTmpFileName, "bpopwtmp.db" );
_LIT( KWhiteBkpFileName, "bpopbbkp.db" );
_LIT( KEndMark, "\n" );
const TInt KArrayGranularity = 10;
const TInt KCacheSize = 32;//The maximum size of the memory cache
const TInt KMaxDbSize = 16384;// (255/max length of the domain/+1/\n/)*64

// ================= MEMBER FUNCTIONS =======================


// -----------------------------------------------------------------------------
// CBrowserPopupEngine::CBrowserPopupEngine()
// -----------------------------------------------------------------------------
//
CBrowserPopupEngine::CBrowserPopupEngine()
    {
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::~CBrowserPopupEngine
// -----------------------------------------------------------------------------
//
CBrowserPopupEngine::~CBrowserPopupEngine()
    {
    iWhiteListFile.Flush();
    iWhiteListFile.Close();
    iFs.Close();
    delete iCachedWhiteUrls;
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::NewLC
// -----------------------------------------------------------------------------
//
CBrowserPopupEngine* CBrowserPopupEngine::NewLC()
    {
    CBrowserPopupEngine* self = new (ELeave) CBrowserPopupEngine;
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::NewL
// -----------------------------------------------------------------------------
//
CBrowserPopupEngine* CBrowserPopupEngine::NewL()
    {
    CBrowserPopupEngine* self = new (ELeave) CBrowserPopupEngine;
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::ConstructL
// -----------------------------------------------------------------------------
//
void CBrowserPopupEngine::ConstructL()
    {
LOG_ENTERFN("CBrowserPopupEngine::ConstructL");
    User::LeaveIfError(iFs.Connect());
//open databases
    BROWSER_LOG( ( _L( "<-Popup engine-> RFs Connected." ) ) );
    OpenDatabaseL( &iWhiteListFile );
    BROWSER_LOG( ( _L( "<-Popup engine-> white file opened." ) ) );
//Create memory cache 
    iCachedWhiteUrls = new(ELeave) CDesCArrayFlat(KArrayGranularity);
    BROWSER_LOG( ( _L( "<-Popup engine-> Cache created." ) ) );
    iCachedWhiteUrls->Reset();
//Load some to memory cache
    LoadUrlsToCacheL( &iWhiteListFile, iCachedWhiteUrls, &iWhiteCacheNo);
    BROWSER_LOG( ( _L( "<-Popup engine-> Urls loaded to cache." ) ) );
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::OpenDatabaseL
// -----------------------------------------------------------------------------
//
void CBrowserPopupEngine::OpenDatabaseL(RFile* aFile)
    {
LOG_ENTERFN("CBrowserPopupEngine::OpenDatabaseL");

	__ASSERT_DEBUG( (aFile != NULL), Util::Panic( Util::EUninitializedData ));

    TInt err;
    RFile tmpFile;
    TPtrC fileNamePtr;
    TPtrC bkpFileNamePtr;
    TBuf<KMaxFileName> privatePath;
    TBuf<KMaxFileName> listFileName;
    TBuf<KMaxFileName> bkpFileName;
    TBuf<KMaxFileName> tmpFileName;
    iFs.PrivatePath( privatePath );
    listFileName.Copy( privatePath );
    listFileName.Append( KWhiteListFileName );
    bkpFileName.Copy( privatePath );
    bkpFileName.Append( KWhiteBkpFileName );
    tmpFileName.Copy( privatePath );
    tmpFileName.Append( KWhiteTmpFileName );
    err = aFile->Open( iFs, listFileName, EFileWrite | EFileStream | EFileShareExclusive );
    BROWSER_LOG( ( _L( "<-Popup engine-> File open error: %d" ), err ) );
    if ( err == KErrNotFound ) 
        {
        err = tmpFile.Open( iFs, bkpFileName, EFileWrite | EFileShareExclusive );
        if( err != KErrNone )
            {
            err = aFile->Create( iFs, listFileName, EFileWrite | EFileShareExclusive );
            BROWSER_LOG( ( _L( "<-Popup engine-> white file created." ) ) );
            }
        else 
            {
//check validity of tmp file if the last char is \n it is probably OK
            if( !CheckDbValidity( &tmpFile ) )
                {
                User::LeaveIfError( 
                    aFile->Create( iFs, listFileName, 
                        EFileWrite | EFileShareExclusive ) );
                }
            else
                {
//rename tmp to origin
                err = tmpFile.Rename( listFileName );
                if(err == KErrNone)
                    {
                    tmpFile.Close();
                    User::LeaveIfError(aFile->Open( iFs, listFileName, EFileWrite | EFileStream | EFileShareExclusive ) );
                    }
                else
                    {
                    User::LeaveIfError( 
                        aFile->Create( iFs, listFileName, 
                            EFileWrite | EFileShareExclusive ) );
                    }
                }
            }
        }
    else if ( err == KErrPathNotFound )
        {
        User::LeaveIfError( iFs.CreatePrivatePath( EDriveC ) );
        User::LeaveIfError(
            aFile->Create( iFs, listFileName, EFileWrite | EFileShareExclusive ) );
        }
    else if((err != KErrInUse)&&( err != KErrNone ))
        {
        User::Leave(err);
        }
    else if( !CheckDbValidity( aFile ) )
        {
//if the file is not valid delete it and create a new one
        aFile->Close();
        User::LeaveIfError( iFs.Delete( listFileName ) );
        User::LeaveIfError(
            aFile->Create( iFs, listFileName, EFileWrite | EFileShareExclusive ) );
        }
//delete tmp and bkp files if they exist
    iFs.Delete( tmpFileName );
    iFs.Delete( bkpFileName );
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::AddUrlToWhiteListL
// -----------------------------------------------------------------------------
//
void CBrowserPopupEngine::AddUrlToWhiteListL(const TDesC& aUrl)
    {
LOG_ENTERFN("CBrowserPopupEngine::AddUrlToWhiteListL");
    TUriParser16 urlParser;
    BROWSER_LOG( ( _L( "<-Popup engine-> url to add to white list : %S" ), &aUrl ) );
//check whether file allready exists in list
    /*if( IsUrlOnWhiteListL( aUrl ) )
        {
        RDebug::Print(_L("<-Popup engine-> url is in white cache allready.\n"));
        return;
        }*/
//get the domain from the url
    urlParser.Parse(aUrl);
    if( urlParser.IsSchemeValid() )
        {
//write domain to file
        HandleUrlOrderChangeL( &iWhiteListFile, urlParser.Extract( EUriHost), EFalse, ETrue );
//add domain to memory cache
        AddUrlToCacheL( iCachedWhiteUrls, urlParser.Extract( EUriHost) );
        }
    else
        {
        User::Leave( KErrBadName );
        }
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::IsUrlOnWhiteListL
// -----------------------------------------------------------------------------
//
TBool CBrowserPopupEngine::IsUrlOnWhiteListL(const TDesC& aUrl)
    {
LOG_ENTERFN("CBrowserPopupEngine::IsUrlOnWhiteListL");
    TUriParser16 urlParser;
    urlParser.Parse(aUrl);
    if( !urlParser.IsSchemeValid() )
        {
        User::Leave( KErrBadName );
        }
//look for URL in memory cache
    if( IsUrlInWhiteCacheL( urlParser.Extract( EUriHost) ) )
        {
        BROWSER_LOG( ( _L( "<-Popup engine-> url is in white cache." ) ) );
        HandleUrlOrderChangeL( &iWhiteListFile, urlParser.Extract( EUriHost), ETrue, ETrue );
        return ETrue;
        }
//look for URL in file
    if( IsUrlInWhiteFileL( urlParser.Extract( EUriHost) ) )
        {
        HandleUrlOrderChangeL( &iWhiteListFile, urlParser.Extract( EUriHost), ETrue, ETrue );
//if found and not in memory cache put it there
        AddUrlToCacheL( iCachedWhiteUrls, urlParser.Extract( EUriHost) );
        BROWSER_LOG( ( _L( "<-Popup engine-> url is in white file." ) ) );
        return ETrue;
        }
    BROWSER_LOG( ( _L( "<-Popup engine-> url is not on white list." ) ) );
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::IsUrlInWhiteCacheL
// -----------------------------------------------------------------------------
//
TBool CBrowserPopupEngine::IsUrlInWhiteCacheL(const TDesC& aUrl)
    {
    return IsUrlInCacheL( iCachedWhiteUrls, aUrl, iWhiteCacheNo );
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::IsUrlInWhiteFileL
// -----------------------------------------------------------------------------
//
TBool CBrowserPopupEngine::IsUrlInWhiteFileL(const TDesC& aUrl)
    {
    return IsUrlInFileL( &iWhiteListFile , aUrl );
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::IsUrlInFileL
// -----------------------------------------------------------------------------
//
TBool CBrowserPopupEngine::IsUrlInFileL(const RFile* aFile, const TDesC& aUrl)
    {
    
   	__ASSERT_DEBUG( (aFile != NULL), Util::Panic( Util::EUninitializedData ));
    
    HBufC8* line = HBufC8::NewL( 256 );
    CleanupStack::PushL( line );
    HBufC8* parturl = HBufC8::NewL( 256 );
    CleanupStack::PushL( parturl );
    TInt match = 1;
    TInt pos = 0;
    TInt result;
    TPtrC16 linePtr16;
    HBufC8* url8 = TDesC16ToHBufC8LC( aUrl );
    User::LeaveIfError( aFile->Seek( ESeekStart, pos ) );
    TPtr8 linePtr = line->Des();
    result = aFile->Read( linePtr );
    parturl->Des().Zero();
    while ( ( line->Des().Length() > 0 ) && ( result == KErrNone ) ) 
        {
        if(parturl->Des().Length() != 0 )
            {
            pos = linePtr.Locate( '\n' );
            parturl->Des().Append( linePtr.Left( pos ) );
            match = url8->Des().Compare( parturl->Des() );
            if( match == 0 )
                {
                CleanupStack::PopAndDestroy( 3 );
                return ETrue;
                }
            parturl->Des().Zero();
            }
        match = linePtr.Find( url8->Des() );
        if( match == KErrNotFound )
            {
            pos = linePtr.LocateReverse( '\n' );
            if(( linePtr.Length() != pos )&&( linePtr.Length() > 1 ))
                {
                parturl->Des().Append( linePtr.Mid( pos + 1, linePtr.Length() - pos - 1 ));
                }
            }
        else
            {
            CleanupStack::PopAndDestroy( 3 );
            return ETrue;
            }
        result = aFile->Read( linePtr, 256 );
        }
    CleanupStack::PopAndDestroy( 3 );
    User::LeaveIfError( result );
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::LoadUrlsToCacheL
// -----------------------------------------------------------------------------
//
void CBrowserPopupEngine::LoadUrlsToCacheL(const RFile* aFile, CDesCArrayFlat* aCacheArray, TInt* aLoadedUrlNo)
    {
LOG_ENTERFN( "PopupEngine::LoadUrlsToCacheL" );

	__ASSERT_DEBUG( (aFile != NULL), Util::Panic( Util::EUninitializedData ));
	__ASSERT_DEBUG( (aCacheArray != NULL), Util::Panic( Util::EUninitializedData ));
	__ASSERT_DEBUG( (aLoadedUrlNo != NULL), Util::Panic( Util::EUninitializedData ));

    TPtrC8 test;
    HBufC8* line = HBufC8::NewL( 256 );
    CleanupStack::PushL( line );
    HBufC8* parturl = HBufC8::NewL( 256  );
    CleanupStack::PushL( parturl );
    TInt result;
    TInt itemno = 0;
    TInt offset = 0;
    TInt prevOffset = 0;
    TPtr8 linePtr = line->Des();
    parturl->Des().Zero();
    User::LeaveIfError( aFile->Seek( ESeekStart, offset ) );
    result = aFile->Read( linePtr, 256 );
    while ( ( line->Des().Length() > 0 ) &&( result == KErrNone ) && ( itemno < KCacheSize )) 
        {
        offset = linePtr.Locate('\n' );
        if( parturl->Length() != 0 )
            {
            parturl->Des().Append( linePtr.Mid( prevOffset, offset ) );
            aCacheArray->AppendL(  *TDesC8ToHBufC16LC( parturl->Des() ) );
            CleanupStack::PopAndDestroy( );
            prevOffset = prevOffset + offset +1;
            itemno++;
            if( itemno == KCacheSize )
                {
                break;
                }
            offset = ( linePtr.Right( linePtr.Length() - prevOffset ) ).Locate('\n');
            }
        while(( offset != KErrNotFound ) && ( itemno < KCacheSize ))
            {
            aCacheArray->AppendL( *TDesC8ToHBufC16LC( linePtr.Mid( prevOffset, offset ) ) );
            CleanupStack::PopAndDestroy( );
            prevOffset = prevOffset + offset +1;
            itemno++;
            offset = ( linePtr.Right( linePtr.Length() - prevOffset ) ).Locate('\n' );
            }
        if( prevOffset != linePtr.Length() )
            {
            parturl->Des().Zero();
            parturl->Des().Append( linePtr.Right( linePtr.Length() - prevOffset ));
            }
        prevOffset = 0;
        result = aFile->Read( linePtr, 256 );
        }
    *aLoadedUrlNo = itemno;
    CleanupStack::PopAndDestroy( 2 );
    User::LeaveIfError( result );
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::IsUrlInCacheL
// -----------------------------------------------------------------------------
//
TBool CBrowserPopupEngine::IsUrlInCacheL( const CDesCArrayFlat* aCacheArray, const TDesC& aUrl, const TInt aLoadedurlno )
    {
LOG_ENTERFN("CBrowserPopupEngine::IsUrlInCacheL");

	__ASSERT_DEBUG( (aCacheArray != NULL), Util::Panic( Util::EUninitializedData ));

    TInt itemno=0;
    BROWSER_LOG( ( _L( "<-Popup engine-> url to find in cache : %S" ), &aUrl ) );
    while( ( itemno < aLoadedurlno ) && ( aUrl.Compare((*aCacheArray)[ itemno ]) != 0 ) )
        {
        itemno++;
        }
    if( itemno != aLoadedurlno )
        {
        BROWSER_LOG( ( _L( "<-Popup engine-> url found in cache" ) ) );
        return ETrue;
        }
    BROWSER_LOG( ( _L( "<-Popup engine-> url not found in cache" ) ) );
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::AddUrlToCacheL
// -----------------------------------------------------------------------------
//
void CBrowserPopupEngine::AddUrlToCacheL( CDesCArrayFlat* aCacheArray, const TDesC& aUrl )
    {
LOG_ENTERFN("CBrowserPopupEngine::AddUrlToCacheL");

	__ASSERT_DEBUG( (aCacheArray != NULL), Util::Panic( Util::EUninitializedData ));

    if( iWhiteCacheNo < KCacheSize )
        {
        aCacheArray->AppendL( aUrl );
        iWhiteCacheNo++;
        BROWSER_LOG( ( _L( "<-Popup engine-> url added to cache : %S" ), &aUrl ) );
        }
    else
        {
        BROWSER_LOG( ( _L( "<-Popup engine-> url not added to cache : %S" ), &aUrl ) );
        }
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::TDesC16ToTPtrC8
// -----------------------------------------------------------------------------
//
TPtrC8 CBrowserPopupEngine::TDesC16ToTPtrC8(const TDesC16 &aString)
    {
    TPtrC8 ptr8(reinterpret_cast<const TUint8*>(aString.Ptr()),(aString.Length()*2));
    return ptr8;
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::TDesC8ToTPtrC16
// -----------------------------------------------------------------------------
//
TPtrC16 CBrowserPopupEngine::TDesC8ToTPtrC16(const TDesC8 &aString)
    {
    TPtrC16 ptr16(reinterpret_cast<const TUint16*>(aString.Ptr()),(aString.Length()/2));
    return ptr16;
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::HandleUrlOrderChange
// -----------------------------------------------------------------------------
//
void CBrowserPopupEngine::HandleUrlOrderChangeL(RFile* aFile, const TDesC& aUrl, const TBool aExists,const TBool aToAdd )
    {
LOG_ENTERFN("CBrowserPopupEngine::HandleUrlOrderChangeL");

	__ASSERT_DEBUG( (aFile != NULL), Util::Panic( Util::EUninitializedData ));

    RFile tmpFile;
    TInt err, match;
    HBufC8* line = HBufC8::NewL( 256 );
    CleanupStack::PushL( line );
    HBufC8* parturl = HBufC8::NewL( 256 );
    CleanupStack::PushL( parturl );
    HBufC8* url8;
    parturl->Des().Zero();
    TPtr8 linePtr = line->Des();
    TPtrC8 linePtrOffset;
    TPtrC8 tmpPtr;
    TInt result;
    TInt pos = 0;
    TInt offset;
    TInt writtenSize = 0;
    TBuf<KMaxFileName> privatePath;
    TBuf<KMaxFileName> listFileName;
    TBuf<KMaxFileName> bkpFileName;
    TBuf<KMaxFileName> tmpFileName;
//set the filenames
    iFs.PrivatePath( privatePath );
    listFileName.Copy( privatePath );
    listFileName.Append( KWhiteListFileName );
    bkpFileName.Copy( privatePath );
    bkpFileName.Append( KWhiteBkpFileName );
    tmpFileName.Copy( privatePath );
    tmpFileName.Append( KWhiteTmpFileName );
//create a tmp file
    err = tmpFile.Create( iFs, tmpFileName, EFileWrite | EFileShareExclusive );
    if( err != KErrNone )
        {
        RDebug::Print(_L("Creation of tmp file failed.\n"));
        BROWSER_LOG( ( _L( "<-Popup engine-> Creation of tmp file failed." ) ) ); 
        User::LeaveIfError( err );
        }
//convert url to 8bit
    url8 = TDesC16ToHBufC8LC( aUrl );
//add url to tmp file
    if( aToAdd )
        {
//by url removal it is not needed to write the url
        tmpFile.Seek( ESeekStart, pos );
        tmpFile.Write( url8->Des() );
        tmpFile.Write( *TDesC16ToHBufC8LC( KEndMark )  );
        CleanupStack::PopAndDestroy();
        tmpFile.Flush();
        writtenSize = url8->Des().Size() + 1;
        }
//write all urls to tmp file except the if needed
    parturl->Des().Zero();
    User::LeaveIfError( aFile->Seek( ESeekStart, pos ) );
    result = aFile->Read( linePtr, 256 );
    linePtr.Set(line->Des());
    writtenSize += linePtr.Length();
    if ( aExists )
       {
       while ( ( line->Des().Length() > 0 ) &&( result == KErrNone ))
            {
            offset=0;
//if there was a part of an url check it too
            if( parturl->Length() != 0 )
                {
                offset = linePtr.Locate('\n' );
                parturl->Des().Append( linePtr.Left( offset ) );
                match = url8->Des().Compare( parturl->Des() );
                if( match != 0 )
                    {
                    tmpFile.Write( parturl->Des() );
                    tmpFile.Write( *TDesC16ToHBufC8LC( KEndMark )  );
                    CleanupStack::PopAndDestroy();
                    }
                offset++;
                parturl->Des().Zero();
                }
//find the new url in the old file
            linePtrOffset.Set( linePtr.Right( linePtr.Length() - offset ));
            match = linePtrOffset.Find( url8->Des()  );
            if( match == KErrNotFound )
                {
                pos =  linePtrOffset.LocateReverse('\n' );
//store the remaining part of the last url
                if( pos != KErrNotFound )
                    {
                    if( pos == linePtrOffset.Length() -1 )
                        {
                        tmpFile.Write( linePtrOffset );
                        }
                    else
                        {
                        parturl->Des( ).Append( linePtrOffset.Mid( pos + 1, linePtrOffset.Length() - pos - 1 ));
                        tmpFile.Write( linePtrOffset.Left( pos+1 ));
                        }
                    }
                else if( linePtrOffset.Length() > 0 )
                    {
                    parturl->Des( ).Append( linePtrOffset );
                    }
                }
            else
                {
                if( match == 0)
                    {
                    tmpFile.Write( linePtrOffset.Right( linePtrOffset.Length() - url8->Des().Length() - 1 ) );
                    }
                else
                    {
                    pos =  linePtrOffset.LocateReverse('\n' );
                    tmpFile.Write( linePtrOffset, match );
                    if( pos < linePtrOffset.Length() - 1  )
                        {
                        if( pos == linePtrOffset.Length() - url8->Des().Length() - 1 )
                            {
                            parturl->Des( ).Append( linePtrOffset.Right( url8->Des().Length() ) );
                            }
                        else//add to offset
                            {
                            tmpFile.Write( linePtrOffset.Right( linePtrOffset.Length() - match - url8->Des().Length() - 1 ) );
                            }
                        }
                    }
                }
            result = aFile->Read( linePtr, 256 );
            linePtr.Set(line->Des());
            }
       }
    else
        {
//copy the whole file
        while ( ( line->Des().Length() > 0 ) &&( result == KErrNone ))
            {
            result = tmpFile.Write( linePtr );
            if ( result == KErrNone )
                {
                result = aFile->Read( linePtr, 256 );
                linePtr.Set( line->Des() );
                writtenSize += linePtr.Length();
                if( writtenSize > KMaxDbSize )
                    {
                    offset = linePtr.Length();
                    while( writtenSize > KMaxDbSize )
                        {
                        offset = linePtr.Left( offset ).LocateReverse('\n' );
                        writtenSize -= linePtr.Length() - offset ;
                        }
                    tmpFile.Write( linePtr, offset );
                    tmpFile.Write( *TDesC16ToHBufC8LC( KEndMark )  );
                    CleanupStack::PopAndDestroy();
                    break;
                    }
                }
            }
        }
    tmpFile.Flush();
//rename original file to bkp
    User::LeaveIfError( aFile->Rename( bkpFileName ) );
    aFile->Close();
//rename temp file to original
    User::LeaveIfError( tmpFile.Rename( listFileName ) );
    tmpFile.Close();
    User::LeaveIfError( aFile->Open( iFs, listFileName, EFileWrite | EFileStream | EFileShareExclusive ) );
//delete bkp file
    User::LeaveIfError( iFs.Delete( bkpFileName ) );
    CleanupStack::PopAndDestroy( 3 );
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::RemoveUrlFromWhiteListL
// -----------------------------------------------------------------------------
//
void CBrowserPopupEngine::RemoveUrlFromWhiteListL(const TDesC& aUrl)
    {
LOG_ENTERFN("CBrowserPopupEngine::RemoveUrlFromWhiteListL");
    TUriParser16 urlParser;
    urlParser.Parse(aUrl);
    if( !urlParser.IsSchemeValid() )
        {
        BROWSER_LOG( ( _L( "<-Popup engine-> Url is not valid." ) ) );
        User::Leave( KErrBadName );
        }
    HandleUrlOrderChangeL( &iWhiteListFile, urlParser.Extract( EUriHost), ETrue, EFalse );
    BROWSER_LOG( ( _L( "<-Popup engine-> Url removed from database." ) ) );
    if( iWhiteCacheNo > 0)
        {
        RemoveUrlFromCacheL( iCachedWhiteUrls, urlParser.Extract( EUriHost), iWhiteCacheNo );
        }
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::RemoveUrlFromCacheL
// -----------------------------------------------------------------------------
//
void CBrowserPopupEngine::RemoveUrlFromCacheL( CDesCArrayFlat* aCacheArray, const TDesC& aUrl, TInt &aCacheNo )
    {
LOG_ENTERFN("CBrowserPopupEngine::RemoveUrlFromCacheL");

	__ASSERT_DEBUG( (aCacheArray != NULL), Util::Panic( Util::EUninitializedData ));

    TInt pos;
    TInt ret;
    ret = aCacheArray->Find( aUrl, pos );
    if( ret == 0 )
        {
        aCacheArray->Delete( pos );
        aCacheArray->Compress();
        aCacheNo--;
        BROWSER_LOG( ( _L( "<-Popup engine-> Url is found in cache and removed." ) ) );
        }
    else
        {
        BROWSER_LOG( ( _L( "<-Popup engine-> Url is not in cache." ) ) );
        }
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::CheckDbValidity
// -----------------------------------------------------------------------------
//
TBool CBrowserPopupEngine::CheckDbValidity(RFile* aFile)
{
LOG_ENTERFN("CBrowserPopupEngine::CheckDbValidity");

	__ASSERT_DEBUG( (aFile != NULL), Util::Panic( Util::EUninitializedData ));

    TInt size;
    TBuf8<1> tmp;
    aFile->Size( size );
    if( size == 0)
        {
//empty file is ok
        BROWSER_LOG( ( _L( "<-Popup engine-> File is valid." ) ) );
        return ETrue;
        }
    else
        {
//check whether the last char is \n
        size = -1;
        aFile->Seek( ESeekEnd, size );
        aFile->Read( tmp, 1 );
        if( tmp.Compare( *TDesC16ToHBufC8LC( KEndMark ) ) == 0 )
            {
            CleanupStack::PopAndDestroy( );
            BROWSER_LOG( ( _L( "<-Popup engine-> File is valid." ) ) );
            return ETrue;
            }
        else
            {
            CleanupStack::PopAndDestroy( );
            BROWSER_LOG( ( _L( "<-Popup engine-> File is not valid." ) ) );
            return EFalse;
            }
        }
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::ConvertTDesC16ToHBufC8LC
// -----------------------------------------------------------------------------
//
HBufC8 *CBrowserPopupEngine::TDesC16ToHBufC8LC(const TDesC16 &string)
    {
    HBufC8 *buff = HBufC8::NewLC(string.Length());
    buff->Des().Copy(string);
    return buff;
    }

// -----------------------------------------------------------------------------
// CBrowserPopupEngine::TDesC8ToHBufC16LC
// -----------------------------------------------------------------------------
//
HBufC16 *CBrowserPopupEngine::TDesC8ToHBufC16LC(const TDesC8 &aString)
    {
    HBufC16 *buff = HBufC16::NewLC(aString.Length());
    buff->Des().Copy(aString);
    return buff;
    }

// End of File
