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
*      Utilities.
*      
*
*/


#ifndef BROWSER_UTILS_H
#define BROWSER_UTILS_H

// INCLUDE FILES

#include <e32base.h>

// FORWARD DECLARATIONS

class CFavouritesItem;
class TFavouritesWapAp;
class MApiProvider;
class CAiwGenericParamList;

// CLASS DECLARATION

/**
* Static utility and debug support methods. This class cannot be instantiated,
* hence the name.
*/
class Util
    {
    public:     // types

        /**
        * Panic reasons.
        */
        enum TPanicReason
            {
            EInvalidDirection,
            EInvalidScrollingUnitType,
            EUninitializedData,
            EOutOfRange,
            ENoChildren,
            EBadPosition,
            ENullPointer,
            EInvalidArgument,
            EUnExpected,
            ERootLevel,
            ENoScriptProxy,
			EMissingFont,
            /// Attempt to remove a non-existing database observer.
            EActiveBookmarkDbObserverNotFound,
            /// Event reported from an unexpected listbox control.
            EFavouritesBadListbox,
            /// The database table is not open as it ought to be.
            EFavouritesDbTableIsNotOpen,
            /// The database table is not closed as it ought to be.
            EFavouritesDbTableIsNotClosed,
            /// Unexpected Uid.
            EFavouritesBadBookmarkUid,
            /// The Ap Model already exists.
            EFavouritesApModelAlreadyExists,
            /// Bad icon index.
            EFavouritesBadIconIndex,
            /// Attempt to create filter twice.
            EFavouritesFilterAlreadyExists,
            /// Item array count and icon index array count do not match.
            EFavouritesBadIconIndexArray,
            /// Model closed with observers still in.
            EFavouritesObserversStillActive,
            /// Attempt to remove an observer which is not added / already removed.
            EFavouritesObserverNotFound,
            /// Attempt to set AP Observer over an existing one.
            EApObserverAlreadySet,
            /// Attempt to set Goto Pane Observer over an existing one.
            EGotoPaneObserverAlreadySet,
            /// Some internal limit doe not match the external dependency.
            ELimitMismatch,
            /// Internal error.
            EFavouritesInternal,
			/// Undefined AP
			EUndefinedAccessPoint,
			/// EUnable to set temporary gateway
			EUnableToSetTempGw,
            /// WIM is off but tried to used
            EUnsupportedFeature,
            /// Start an incremental operation when another is still running.
            EIncrementalOperationAlreadyRunning,
            /// Attempt ot update data which is in use (incremental operations)
            ECannotUpdateDataInUse,
			// Size string is not convertable to integer
			EInvalidSizeValue,
            /// Descriptor-based downloaded content handler already set.
            EDownloadedContentHandlerAlreadyInitialized,
            /// File-based downloaded content handler already set.
            EFileDownloadedContentHandlerAlreadyInitialized
            };

    public:     // methods

        /**
        * Panic the current thread.
        * @param aReason Panic reason.
        */
        static void Panic( TPanicReason aReason );

        /**
        * In debug builds, perform sanity check before application starts and
		* panic if some check fails. In release builds, do nothing.
		* Whatever "once-only" sanity checking you have, add that to this
		* method.
        */
        static void SanityCheckL();

		/**
		* Parses the given URL. Fills username and password fields of the
		* given item. Fills URL field with the given url excluding the
		* username and password parts.
		* @param aUrl URL.
		* @param aItem Bookmark item.
		*/
		static void RetreiveUsernameAndPasswordFromUrlL
			( const TDesC& aUrl, CFavouritesItem& aItem );

		/**
		* Returns descriptor to the middle of the url between '//' and '#'.
		* @param aUrl Original url.
		* @return Stripped url.
		*/
		static TPtrC StripUrl( const TDesC& aUrl );

		        /**
        * Create an URL from a file name (file://blabla) and return
        * it in an allocated buffer. Pushed on cleanup stack; owner is the
        * caller.
        * @param aFileName File name.
        * @return The created URL in a buffer.
        */
        static HBufC* UrlFromFileNameLC( const TDesC& aFileName );

        /**
        * Check if Disk space goes below critical level if allocating
        * more disk space.
        * @param aShowErrorNote If ETrue, a warning note is shown.
        * @param aBytesToWrite Amount of disk space to be allocated (use an
        * estimate if exact amount is not known).
        * @return ETrue if allocation would go below critical level.
        */
        static TBool FFSSpaceBelowCriticalLevelL
            ( TBool aShowErrorNote, TInt aBytesToWrite = 0 );

        static TBool CheckBookmarkApL( const MApiProvider& aApiProvider, const TFavouritesWapAp& aItem );
        static TBool IsValidAPFromParamL( const MApiProvider& aApiProvider, TUint32 aIAPid );
        static TBool CheckApValidityL( const MApiProvider& aApiProvider, const TUint32 aItem );
        static TUint32 IapIdFromWapIdL( const MApiProvider& aApiProvider, const TUint32 aItem );
        static TUint32 WapIdFromIapIdL( const MApiProvider& aApiProvider, const TUint32 aItem );
        static void EncodeSpaces(HBufC*& aString);

        /**
        * Allocate a heap buffer for aUrl and make sure the URL is encoded.
        * If aUrl does not contain a scheme, then a default "http://" 
        * scheme is prepended to aUrl.
        * @param aUrl The URL to allocate a buffer for. 
        * @return The URL in a buffer. 
        */
        static HBufC* AllocateUrlWithSchemeL( const TDesC& aUrl );
    };

#endif

// End of file
