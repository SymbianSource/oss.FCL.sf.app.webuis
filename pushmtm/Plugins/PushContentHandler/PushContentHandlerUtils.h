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
* Description:  Utilities
*
*/



#ifndef PUSHCONTENTHANDLERUTILS_H
#define PUSHCONTENTHANDLERUTILS_H

// INCLUDE FILES

#include <e32def.h>
#include <e32base.h>
#include <nwx_status.h>
#include <nw_wbxml_dictionary.h>
#include <nw_encoder_stringtable.h>
#include <nw_string_string.h>

/**
* Convert cXML status code to Symbian OS error code.
* @param aNwxErrorCode cXML status.
* @return Symbian OS error code.
*/
TInt NwxStatusToErrCode( NW_Status_t aNwxErrorCode );

/*
*   The following tiny classes have been introduced to help putting C-style 
*   Browser Engine structures to the Cleanup Stack, so that we can implement 
*   a LEAVE-safe code.
*/

/**
* Encapsulate WBXML dictionary initialization.
*/
class RWbxmlDictionary
    {
    public:
        void InitializeL( NW_Int32 aCount, NW_WBXML_Dictionary_t* aDictArray[] );
        void Close();
    };

/**
* Encapsulate a WBXML document tree.
*/
class CDocumentTreeOwner : public CBase
    {
    public:
        CDocumentTreeOwner();
        CDocumentTreeOwner( NW_DOM_DocumentNode_t* aDocTree );
        ~CDocumentTreeOwner();
    public:
        void SetDocTree( NW_DOM_DocumentNode_t* aDocTree );
    private:
        NW_DOM_DocumentNode_t* iDocTree; ///< Owned.
    };

/**
* Encapsulate an NW_String_t.
*/
class CStringOwner : public CBase
    {
    public:
        CStringOwner();
        CStringOwner( NW_String_t* aString );
        ~CStringOwner();
    public:
        void SetString( NW_String_t* aString );
    private:
        NW_String_t* iString; ///< Owned.
    };

#endif // PUSHCONTENTHANDLERUTILS_H

