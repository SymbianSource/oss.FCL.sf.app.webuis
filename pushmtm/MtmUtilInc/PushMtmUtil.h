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
* Description:  Declaration of CPushMtmUtil.
*
*/



#ifndef PUSHMTMUTIL_H
#define PUSHMTMUTIL_H

// INCLUDE FILES

#include <e32base.h>
#include <e32std.h>
#include <msvstd.h>

// CONSTANTS

/** 
* Defines for masking values used to extract information from iMtmData2.
* Note that iMtmData1 is used by CPushMsgEntryBase and derived classes!
*/
const TUint32 KPushMtmMaskOnlyAttrs             = 0x000000FF;
const TUint32 KPushMtmShiftOnlyAttrs            = 0;
const TUint32 KPushMtmMaskEverythingButAttrs    = 0xFFFFFF00;
/**
* Attributes. The maximum number of attribute is determined by 
* **************** KPushMtmMaskOnlyAttrs ****************** 
* Each attribute can be set (1) or reseted (0).
*/
const TUint32 EPushMtmAttrHasHref           = 0x00000001;
const TUint32 EPushMtmReadButContentChanged = 0x00000002;
const TUint32 EPushMtmNotUsed1              = 0x00000004;
const TUint32 EPushMtmNotUsed2              = 0x00000008;
const TUint32 EPushMtmNotUsed3              = 0x00000010;
const TUint32 EPushMtmNotUsed4              = 0x00000020;
const TUint32 EPushMtmNotUsed5              = 0x00000040;
const TUint32 EPushMtmNotUsed6              = 0x00000080;

// FORWARD DECLARATIONS

class CMsvSession;

// CLASS DECLARATION

/**
* Utility class.
*/
class CPushMtmUtil : public CBase
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor. Leaves on failure.
        * @param aMsvSession Message Server Session to be used by this operation.
        * @return The constructed object.
        */
        IMPORT_C static CPushMtmUtil* NewL( CMsvSession& aMsvSession );

        /**
        * Destructor.
        */
        IMPORT_C virtual ~CPushMtmUtil();

    public: // New functions

        /**
        * Mark the given entry unread and new.
        * @param aEntryId The entry id to use.
        * @param aUnread ETrue=mark unread, EFalse=mark read.
        * @return None.
        */
        IMPORT_C void MarkServiceUnreadL( TMsvId aEntryId, TBool aUnread );

        /**
        * Find all the push messages of the specified type in the given folder 
        * and its subfolders recursively (if specified).
        * @param aFolderId The given folder.
        * @param aMsgType Push message type.
        * @param aRecursive Is recursive search desired.
        * @return TMsvIds of the messages.
        */
        IMPORT_C CMsvEntrySelection* FindMessagesL( TMsvId aFolderId, 
                                                    TUid aMsgType, 
                                                    TBool aRecursive );

        /**
        * Find all the push messages of the specified type in the given folders.
        * @param aFolders Selection of folders.
        * @param aMsgType Push message type.
        * @return TMsvIds of the messages.
        */
        IMPORT_C CMsvEntrySelection* FindMessagesL
            ( const CMsvEntrySelection& aFolders, TUid aMsgType );

        /**
        * Find all the push messages of the specified type in the 'push' folders.
        * @param aMsgType Push message type.
        * @return TMsvIds of the messages.
        */
        IMPORT_C CMsvEntrySelection* FindMessagesL( TUid aMsgType );

        /**
        * Find all the folders that may contain push messages.
        * @return TMsvIds of the folders.
        */
        IMPORT_C CMsvEntrySelection* FindPushFoldersL();

        /**
        * Locates SI entries with the specified SI-ID. Search is made in the 
        * 'push' folders returned by FindPushFoldersL().
        * @param aSiId Id of SI message to locate.
        * @return Selection of entry IDs found.
        */
        IMPORT_C CMsvEntrySelection* FindSiIdLC( const TDesC& aSiId );

        /**
        * Finds SI or SL entries with the specified Url and Push Message Type.
        * Search is made in the 'push' folders returned by FindPushFoldersL().
        * @param aUrl URL to find
        * @param aPushType Push Message type to filter entries by.
        * @return Selection of entry IDs found.
        */
        IMPORT_C CMsvEntrySelection* FindUrlLC( const TDesC& aUrl, 
                                                TUid aPushType );

        /**
        * Delete the given entry.
        * @param aId Id of the message to locate.
        * @return None.
        */
        IMPORT_C void DeleteEntryL( TMsvId aId );

        /**
        * Convert the "From" information to the format required by the UI spec:
        * it extracts the Host part of the URI. If it is ampty string, then 
        * it returns the copy of the original string.
        * @param aFrom The sender information of a push message.
        * @return The converted string in unicode.
        */
        IMPORT_C HBufC* ConvertDetailsL( const TDesC8& aFrom ) const;

        /**
        * Sets the given attributes of the context
        * @param aContext Context entry that is modified.
        * @param aAttr Attributes to be set.
        * @return None.
        */
        IMPORT_C static void SetAttrs( TMsvEntry& aContext, TUint32 aAttrs );

        /**
        * Resets the given attributes of the context.
        * @param aContext Context entry that is modified.
        * @param aAttr Attributes to be reset.
        * @return None.
        */
        IMPORT_C static void ResetAttrs( TMsvEntry& aContext, TUint32 aAttrs );

        /**
        * Get the value of the attributes of the context.
        * @param aContext Context entry.
        * @return The attributes.
        */
        IMPORT_C static TUint32 Attrs( const TMsvEntry& aContext );

        /**
        * Convert the given URI to an appropriate form to be displayed.
        * See UriUtils::ConvertToDisplayFormL for more information.
        * @param aUri URI to be converted.
        * @return The converted URI.
        */
        IMPORT_C static HBufC* ConvertUriToDisplayFormL( const TDesC& aUri );

    protected: // Constructors
        
        /**
        * Constructor.
        * @param aMsvSession Message server session.
        */
        CPushMtmUtil( CMsvSession& aMsvSession );

        /**
        * Symbian OS constructor.
        */
        void ConstructL();

    protected: // New functions

        /**
        * Find all the push messages of the specified type in the given folder 
        * and its subfolders recursively (if specified).
        * @param aFolderId The given folder.
        * @param aMsgType Push message type.
        * @param aRecursive Is recursive search desired.
        * @param aResult The result (TMsvIds of the messages) is appended to 
        *        this array.
        * @return None.
        */
        void FindMessagesL( TMsvId aFolderId, 
                            TUid aMsgType, 
                            TBool aRecursive, 
                            CMsvEntrySelection& aResult );

    private: // Data

        CMsvSession& iMsvSession; ///< Message Server session.
    };

#endif // PUSHMTMUTIL_H

// End of file.
