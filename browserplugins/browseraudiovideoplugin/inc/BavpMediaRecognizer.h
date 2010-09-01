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


#ifndef BAVPMEDIARECONGNIZER_H
#define BAVPMEDIARECONGNIZER_H

// INCLUDES
#include <apgcli.h>
// L960: header file name with non-standard character 
#include <mmf/common/mmfcontrollerpluginresolver.h> 

// Media types
enum TBavpMediaType
    {
    ELocalVideoFile,
    ELocalAudioFile,
    ELocalRamFile,
    ELocalSdpFile,
    EUrl,
    ELocalAudioPlaylist,
#if defined(BRDO_ASX_FF)
    ELocalAsxFile,
#endif //BRDO_ASX_FF
    EUnidentified 
    };

/**
*  CBavpMediaRecognizer
*  Implementation of CBavpMediaRecognizer
*  @lib npBrowserVideoPlugin.lib
*  @since 3.2
*/
NONSHARABLE_CLASS(CBavpMediaRecognizer) : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CBavpMediaRecognizer* NewL();
        
        /**
        * Destructor.
        */
        ~CBavpMediaRecognizer();

    public: // New functions
        
        /**
        * Recognizes media type. 
		* @since 3.2
        * @param aMediaName Media file name or Url
        * @param aIncludeUrls Are Urls included or not
        * @return TBavpMediaType
        */
        TBavpMediaType IdentifyMediaTypeL( const TDesC& aMediaName, 
                                           TBool aIncludeUrls = EFalse );

        /**
        * Recognizes media type. 
		* @since 3.2
        * @param aMediaName Media file name or Url
        * @param aFile file handle 
        * @param aIncludeUrls Are Urls included or not
        * @return TBavpMediaType
        */
        TBavpMediaType IdentifyMediaTypeL( const TDesC& aMediaName, 
                                           RFile& aFile,  
                                           TBool aIncludeUrls = EFalse );

        /**
        * Recognizes Media files MimeType. 
		* @since 3.2
        * @param aLocalFile File name
        * @return TBuf<KMaxDataTypeLength>, Mime type.
        */
        TBuf<KMaxDataTypeLength> MimeTypeL( const TDesC& aLocalFile );

        /**
        * Checks if Url is valid or not.
		* @since 3.2
        * @param aUrl
        * @return ETrue:    Valid
        *         EFalse:   Not valid
        */
        TBool ValidUrl( const TDesC& aUrl );

        /**
        * Recognizes Media files MimeType. 
		* @since 3.2
        * @param aFile File handle
        * @return TBuf<KMaxDataTypeLength>, Mime type.
        */
        TBuf<KMaxDataTypeLength> MimeTypeL( RFile& aFile ); 
                    
    private:  // New functions
              
        /**
        * Checks if there is video playback support.
        * @param aFileName
        * @param aFileRecognitionResult Recognition result
        * @return ETrue:    Supported
        *         EFalse:   Not supported
        */
        TBool FileHasVideoSupport( const TDesC& aFileName, 
                        TDataRecognitionResult& aFileRecognitionResult );

        /**
        * Checks if there is audio playback support.
        * @param aFileName
        * @param aFileRecognitionResult Recognition result
        * @return ETrue:    Supported
        *         EFalse:   Not supported
        */
        TBool FileHasAudioSupport( const TDesC& aFileName, 
                        TDataRecognitionResult& aFileRecognitionResult );
        
        /**
        * Creates array to hold all the controller plugins that support video.
        * @return void
        */
        void CreateVideoFormatsArrayL();

        /**
        * Creates array to hold all the controller plugins that support audio.
        * @return void
        */
        void CreateAudioFormatsArrayL();

        /**
        * Recognizes local file
        * @param aFileName
        * @param aFileRecognitionResult Recognition result
        * @return void
        */
        void RecognizeFileL( const TDesC& aFileName, 
                             TDataRecognitionResult& aResult );
        
        /**
        * Reads local file to buffer
        * @param aFileName
        * @param aBuf, buffer
        * @return KErrNone or one of the system wide error codes.
        */
        TInt ReadFile( const TDesC& aFileName, TDes8& aBuf );

        /**
        * Determines presence of file handle.
        * @return ETrue if file handle exists.
        */
        TBool FileHandleExists();

    private:

        /**
        * C++ default constructor.
        */
        CBavpMediaRecognizer();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

    private:    // Data
        // Array to hold all the controller plugins that support video
        RMMFControllerImplInfoArray iVideoControllers;
        // Array to hold all the controller plugins that support audio
        RMMFControllerImplInfoArray iAudioControllers;
        // bufsize
        TInt iBufSize;
        // Mime type recognizer
        RApaLsSession iRecognizer;
        RFile iFileHandle; 
    };

#endif  // BAVPMEDIARECONGNIZER_H
            
// End of File
