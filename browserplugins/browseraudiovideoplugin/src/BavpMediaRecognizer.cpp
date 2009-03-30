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
* Description:  Implementation of CBavpMediaRecognizer
*
*/


// INCLUDE FILES
#include <eikenv.h>
#include <bautils.h>
#include <mmf/server/mmfdatasourcesink.hrh>
#include <mmf/common/mmfcontrollerpluginresolver.h>

#include "BavpMediaRecognizer.h"
#include "BavpLogger.h"

// CONSTANTS
_LIT8(KDataTypeRAM, "audio/x-pn-realaudio-plugin");
_LIT8(KDataTypeSDP, "application/sdp");
_LIT8(KDataTypePlaylist, "audio/mpegurl");
_LIT8(KDataTypeRNG, "application/vnd.nokia.ringing-tone"); 
_LIT8(KDataTypeMp3, "audio/mp3"); 
_LIT8(KDataTypeXmp3, "audio/x-mp3"); 
_LIT8(KDataTypeAudio3gp, "audio/3gpp");

#if defined(BRDO_ASX_FF)
_LIT8(KDataTypeWvx, "video/x-ms-wvx");
_LIT8(KDataTypeWax, "audio/x-ms-wax");
_LIT8(KDataTypeAsx, "video/x-ms-asx");
#endif // BRDO_ASX_FF

_LIT(KUrlRtsp, "rtsp:");

const TInt KDefaultBufferSize = 1000;
 
// _LIT(KRamFileExtension,".ram");

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBavpMediaRecognizer::CBavpMediaRecognizer
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
CBavpMediaRecognizer::CBavpMediaRecognizer()
    {
    }

// -----------------------------------------------------------------------------
// CBavpMediaRecognizer::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
void CBavpMediaRecognizer::ConstructL()
    {
    User::LeaveIfError( iRecognizer.Connect() );
    if ( iRecognizer.GetMaxDataBufSize( iBufSize) || iBufSize <= 0 )
        {
        iBufSize = KDefaultBufferSize;
        }
    CreateVideoFormatsArrayL();
    CreateAudioFormatsArrayL();
    }

// -----------------------------------------------------------------------------
// CBavpMediaRecognizer::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
CBavpMediaRecognizer* CBavpMediaRecognizer::NewL()
    {
    CBavpMediaRecognizer* self = new( ELeave ) CBavpMediaRecognizer; 
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// -----------------------------------------------------------------------------
// CBavpMediaRecognizer::~CBavpMediaRecognizer   
// Destructor
// -----------------------------------------------------------------------------
CBavpMediaRecognizer::~CBavpMediaRecognizer()
    {
    iVideoControllers.ResetAndDestroy();
    iAudioControllers.ResetAndDestroy();
    iRecognizer.Close();
    iFileHandle.Close();
    }

// -----------------------------------------------------------------------------
//  CBavpMediaRecognizer::IdentifyMediaTypeL
// -----------------------------------------------------------------------------
TBavpMediaType
CBavpMediaRecognizer::IdentifyMediaTypeL( const TDesC& aMediaName, 
                                          TBool aIncludeUrls )
    {
    TBavpMediaType ret = EUnidentified;

    if ( BaflUtils::FileExists( CEikonEnv::Static()->FsSession(), aMediaName ) )
        {
        TDataRecognitionResult fileRecognitionResult;
        RecognizeFileL( aMediaName, fileRecognitionResult );
        
        if ( !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypeSDP ) )
            {
            ret = ELocalSdpFile;
            Log( EFalse, _L("CBavpMediaRecognizer::IdentifyMediaTypeL - ELocalSdpFile") );
            }
        
        if ( ret == EUnidentified && 
             !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypePlaylist ) )
            {
            ret = ELocalAudioPlaylist;
            Log( EFalse, _L("CBavpMediaRecognizer::IdentifyMediaTypeL - ELocalAudioPlaylist") );
            }
        
        if ( ret == EUnidentified && 
             FileHasAudioSupport( aMediaName, fileRecognitionResult ) || 
             !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypeRNG ) ||
             !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypeMp3 ) ||
             !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypeXmp3 ) ||
             !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypeAudio3gp ) ) 
            {
            ret = ELocalAudioFile; 
            Log( EFalse, _L("CBavpMediaRecognizer::IdentifyMediaTypeL - ELocalAudioFile") );
            }
        
        if ( ret == EUnidentified && 
             FileHasVideoSupport( aMediaName, fileRecognitionResult ) )
            {
            ret = ELocalVideoFile;
            Log( EFalse, _L("CBavpMediaRecognizer::IdentifyMediaTypeL - ELocalVideoFile") );
            }
        
        if ( ret == EUnidentified && 
             !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypeRAM ) )
            {
            ret = ELocalRamFile;
            Log( EFalse, _L("CBavpMediaRecognizer::IdentifyMediaTypeL - ELocalRamFile") );
            }
#if defined(BRDO_ASX_FF)
        if ( ret == EUnidentified && 
             !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypeWvx ) ||
             !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypeWax ) ||
             !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypeAsx ))
            {
            ret = ELocalAsxFile;
            Log( EFalse, _L("CBavpMediaRecognizer::IdentifyMediaTypeL - ELocalAsxFile") );
            }
#endif // BRDO_ASX_FF
        }
    else
        {
        if ( aIncludeUrls && ValidUrl( aMediaName ) )
            {
            ret = EUrl;
            Log( EFalse, _L("CBavpMediaRecognizer::IdentifyMediaTypeL - EUrl") );
            }
        }
    return ret;
    }

// -----------------------------------------------------------------------------
//  CBavpMediaRecognizer::IdentifyMediaTypeL 
// -----------------------------------------------------------------------------
TBavpMediaType 
CBavpMediaRecognizer::IdentifyMediaTypeL( const TDesC& aMediaName, 
                                          RFile& aFile,
                                          TBool aIncludeUrls )
    {
    if( aFile.SubSessionHandle() )
        {
        User::LeaveIfError( iFileHandle.Duplicate( aFile ) ); 
        }
    
    TBavpMediaType ret = EUnidentified;

    if ( BaflUtils::FileExists( CEikonEnv::Static()->FsSession(), aMediaName ) || FileHandleExists() ) 
        {
        TDataRecognitionResult fileRecognitionResult;
        RecognizeFileL( aMediaName, fileRecognitionResult );
        
        if ( !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypeSDP ) )
            {
            ret = ELocalSdpFile;
            Log( EFalse, _L("CBavpMediaRecognizer::IdentifyMediaTypeL - ELocalSdpFile") );
            }
            
        if ( ret == EUnidentified && 
             !fileRecognitionResult.iDataType.Des8().CompareF( 
             KDataTypePlaylist ) )
            {
            ret = ELocalAudioPlaylist;
            Log( EFalse, _L("CBavpMediaRecognizer::IdentifyMediaTypeL - ELocalAudioPlaylist") );
            }
        
        if ( ret == EUnidentified && 
             FileHasAudioSupport( aMediaName, fileRecognitionResult ) || 
             !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypeRNG ) ||
             !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypeMp3 ) ||
             !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypeXmp3 ) ||
             !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypeAudio3gp ) ) 
            {
            ret = ELocalAudioFile; 
            Log( EFalse, _L("CBavpMediaRecognizer::IdentifyMediaTypeL - ELocalAudioFile") );
            }

        if ( ret == EUnidentified && 
             FileHasVideoSupport( aMediaName, fileRecognitionResult ) )
            {
            ret = ELocalVideoFile;
            Log( EFalse, _L("CBavpMediaRecognizer::IdentifyMediaTypeL - ELocalVideoFile") );
            }

        if ( ret == EUnidentified && 
             !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypeRAM ) )
            {
            ret = ELocalRamFile;
            Log( EFalse, _L("CBavpMediaRecognizer::IdentifyMediaTypeL - ELocalRamFile") );
            }
#if defined(BRDO_ASX_FF)
        if ( ret == EUnidentified && 
             !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypeWvx ) ||
             !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypeWax ) ||
             !fileRecognitionResult.iDataType.Des8().CompareF( KDataTypeAsx ) )
            {
            ret = ELocalAsxFile;
            Log( EFalse, _L("CBavpMediaRecognizer::IdentifyMediaTypeL - ELocalAsxFile") );
            }
#endif // BRDO_ASX_FF
        }
    else
        {
        if (aIncludeUrls && ValidUrl(aMediaName))
            {
            ret = EUrl;
            }
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CBavpMediaRecognizer::MimeTypeL
// -----------------------------------------------------------------------------
TBuf<KMaxDataTypeLength>
CBavpMediaRecognizer::MimeTypeL( const TDesC& aLocalFile )
    {
    TDataRecognitionResult fileRecognitionResult;
    RecognizeFileL( aLocalFile,fileRecognitionResult );
    
    if ( fileRecognitionResult.iConfidence >= CApaDataRecognizerType::EPossible )
        {
        return fileRecognitionResult.iDataType.Des();
        }
        
    return KNullDesC();
    }

// -----------------------------------------------------------------------------
// CBavpMediaRecognizer::FileHasVideoSupport
// -----------------------------------------------------------------------------
TBool CBavpMediaRecognizer::FileHasVideoSupport( const TDesC& /*aFileName*/, 
                                TDataRecognitionResult& aFileRecognitionResult)
    {
    // Try with mime type
    // We have a good quess -> go with mime type
    if ( aFileRecognitionResult.iConfidence >= CApaDataRecognizerType::EPossible )
        {
        for (TInt i = 0 ; i < iVideoControllers.Count() ; i++)
            {           
            RMMFFormatImplInfoArray formats = 
                        iVideoControllers[i]->PlayFormats();
                for ( TInt j = 0; j < formats.Count() ; j++ )
                    { 
                    if ( formats[j]->SupportsMimeType( 
                            aFileRecognitionResult.iDataType.Des8() ) )
                        return ETrue;
                    }
            }
        } 
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CBavpMediaRecognizer::FileHasAudioSupport
// -----------------------------------------------------------------------------
TBool CBavpMediaRecognizer::FileHasAudioSupport(const TDesC& aFileName, 
                               TDataRecognitionResult& aFileRecognitionResult )
    {
     // try with mime type
    // we have a good quess -> gogo with mime type
    if (aFileRecognitionResult.iConfidence >= CApaDataRecognizerType::EPossible)
        {
        for (TInt i = 0 ; i < iAudioControllers.Count() ; i++)
            {           
            RMMFFormatImplInfoArray formats = 
                    iAudioControllers[i]->PlayFormats();
                for (TInt j = 0; j < formats.Count() ; j++)
                    { 
                    if ( formats[j]->SupportsMimeType( 
                            aFileRecognitionResult.iDataType.Des8() ) )
                        return ETrue;
                    }
            }
        }
 
    // we can't quess -> try with file extension
    else
        {
        TParsePtrC parser(aFileName);
        TBuf8<KMaxFileName> fileExtension;
        fileExtension.Copy(parser.Ext());
        for (TInt i = 0 ; i < iVideoControllers.Count() ; i++)
            {
            RMMFFormatImplInfoArray formats = 
                    iAudioControllers[i]->PlayFormats();
            for (TInt j = 0; j < formats.Count() ; j++)
                {
                if (formats[j]->SupportsFileExtension(fileExtension))
                    return ETrue;
                }
            }
        }
   
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CBavpMediaRecognizer::ValidUrl
// -----------------------------------------------------------------------------
TBool CBavpMediaRecognizer::ValidUrl(const TDesC& aUrl)
    {
    TBool ret = EFalse;
    if (aUrl.Find(KUrlRtsp) == 0)
        {
        ret = ETrue;
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// CBavpMediaRecognizer::MimeTypeL
// -----------------------------------------------------------------------------
TBuf<KMaxDataTypeLength> CBavpMediaRecognizer::MimeTypeL(RFile& aFile)
    {
    if( aFile.SubSessionHandle() )
        {
        // Preferred
        User::LeaveIfError( iFileHandle.Duplicate( aFile ) ); 
        }

    // If new handle is not valid, old might still be
    if ( !FileHandleExists() )
        {
        User::Leave( KErrBadHandle );
        }

    TDataRecognitionResult fileRecognitionResult;
    RecognizeFileL( KNullDesC(), fileRecognitionResult );
    
    if (fileRecognitionResult.iConfidence >= CApaDataRecognizerType::EPossible)
        {
        return fileRecognitionResult.iDataType.Des();
        }
    return KNullDesC();
    }

// -----------------------------------------------------------------------------
// CBavpMediaRecognizer::CreateVideoFormatsArrayL
// -----------------------------------------------------------------------------
void CBavpMediaRecognizer::CreateVideoFormatsArrayL()
    {
    CMMFControllerPluginSelectionParameters* cSelect = 
            CMMFControllerPluginSelectionParameters::NewLC();
    
    CMMFFormatSelectionParameters* fSelect = 
            CMMFFormatSelectionParameters::NewLC();
    
    // Set the play and record format selection parameters to be blank.  
    // - format support is only retrieved if requested.
    cSelect->SetRequiredPlayFormatSupportL(*fSelect);
    
    // Set the media ids
    RArray<TUid> mediaIds;
    CleanupClosePushL(mediaIds);
    User::LeaveIfError(mediaIds.Append(KUidMediaTypeVideo));
    
    // Get plugins that support at least video
    cSelect->SetMediaIdsL( mediaIds, 
            CMMFPluginSelectionParameters::EAllowOtherMediaIds );
    
    // iVideoControllers contains now all plugins that support at least video
    cSelect->ListImplementationsL(iVideoControllers);           
    
    // Clean up
    CleanupStack::PopAndDestroy( 3 ); // fSelect, cSelect, mediaIds
    }

// -----------------------------------------------------------------------------
// CBavpMediaRecognizer::CreateAudioFormatsArrayL
// -----------------------------------------------------------------------------
void CBavpMediaRecognizer::CreateAudioFormatsArrayL()
    {
    CMMFControllerPluginSelectionParameters* cSelect = 
            CMMFControllerPluginSelectionParameters::NewLC();

    CMMFFormatSelectionParameters* fSelect = 
            CMMFFormatSelectionParameters::NewLC();

    // Set the play and record format selection parameters to be blank.  
    // - format support is only retrieved if requested.
    cSelect->SetRequiredPlayFormatSupportL( *fSelect );

    // Set the media ids
    RArray<TUid> mediaIds;
    CleanupClosePushL( mediaIds );
    User::LeaveIfError( mediaIds.Append(KUidMediaTypeAudio) );

    // Get plugins that supports audio only
    cSelect->SetMediaIdsL( mediaIds, 
            CMMFPluginSelectionParameters::EAllowOnlySuppliedMediaIds ); 

    // iAudioControllers contains now all audio plugins that 
    // support at least audio.
    cSelect->ListImplementationsL( iAudioControllers );           

    // Clean up
    CleanupStack::PopAndDestroy( 3 ); // fSelect, cSelect, mediaIds
    }

// -----------------------------------------------------------------------------
// CBavpMediaRecognizer::RecognizeFileL
// -----------------------------------------------------------------------------
void CBavpMediaRecognizer::RecognizeFileL( const TDesC& aFileName, 
                                           TDataRecognitionResult& aResult )
    {
    aResult.Reset();
    HBufC8* fBuf = HBufC8::NewLC( iBufSize ); // fBuf in CS
    TPtr8 fileBuf = fBuf->Des();
 
    // Recognize file
    if ( FileHandleExists() )
        {
        User::LeaveIfError( iRecognizer.RecognizeData( iFileHandle, aResult ) );
        }
    else
        {
        // Read file
        User::LeaveIfError( ReadFile( aFileName, fileBuf ) );   
        User::LeaveIfError( iRecognizer.RecognizeData( aFileName, fileBuf, aResult ) ); 
        }
   
    CleanupStack::PopAndDestroy(); // fBuf
    }

// -----------------------------------------------------------------------------
// CBavpMediaRecognizer::ReadFile
// -----------------------------------------------------------------------------
TInt CBavpMediaRecognizer::ReadFile( const TDesC& aFileName, TDes8& aBuf )
    {
    RFile file;
    TInt err;

    if ( FileHandleExists() )
        {
        err = file.Duplicate( iFileHandle ); 
        }
    else
        {
        err = file.Open( CEikonEnv::Static()->FsSession(), aFileName,
                         EFileRead | EFileShareAny );
        if ( err ) 
            {
            err = file.Open( CEikonEnv::Static()->FsSession(), aFileName,
                             EFileRead | EFileShareReadersOnly );
            } 
        }

    if ( err )
        {
        // If file does not exist
        return err;
        }

    // Read the beginning of the file
    err = file.Read( 0, aBuf, iBufSize );
    file.Close();

    return err;
    }

// -----------------------------------------------------------------------------
// CBavpMediaRecognizer::FileHandleExists
// -----------------------------------------------------------------------------
TBool CBavpMediaRecognizer::FileHandleExists()
    {
    TInt size(0);
    TInt err(0);

    if ( iFileHandle.SubSessionHandle() )
        {
        err = iFileHandle.Size( size );
        }
    
    return ( !err && size ) ? ETrue : EFalse;
    }

//  End of File
