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



// INCLUDE FILES

#include "PushContentHandlerUtils.h"
#include <e32std.h>
#include <nw_dom_document.h>

// ---------------------------------------------------------
// NwxStatusToErrCode
// ---------------------------------------------------------
//
TInt NwxStatusToErrCode( NW_Status_t aNwxErrorCode )
    {
    TInt ret( KErrGeneral );

    switch ( aNwxErrorCode )
        {
        case NW_STAT_SUCCESS:
            {
            ret = KErrNone;
            break;
            }
        case NW_STAT_OUT_OF_MEMORY:
            {
            ret = KErrNoMemory;
            break;
            }
        case NW_STAT_CANCELLED:
        case NW_STAT_CONN_CANCELLED:
            {
            ret = KErrCancel;
            break;
            }
        default:
            {
            ret = KErrGeneral;
            break;
            }
        }

    return ret;
    }

// ---------------------------------------------------------
// RWbxmlDictionary::InitializeL
// ---------------------------------------------------------
//
void RWbxmlDictionary::InitializeL( NW_Int32 aCount, 
                                    NW_WBXML_Dictionary_t* aDictArray[] )
    {
    NW_Status_t stat = NW_STAT_SUCCESS;
    stat = NW_WBXML_Dictionary_initialize( aCount, aDictArray );
    User::LeaveIfError( NwxStatusToErrCode( stat ) );
    }

// ---------------------------------------------------------
// RWbxmlDictionary::Close
// ---------------------------------------------------------
//
void RWbxmlDictionary::Close()
    {
    NW_WBXML_Dictionary_destroy();
    }

// ---------------------------------------------------------
// CDocumentTreeOwner::CDocumentTreeOwner
// ---------------------------------------------------------
//
CDocumentTreeOwner::CDocumentTreeOwner() 
:   iDocTree( NULL )
    {
    }

// ---------------------------------------------------------
// CDocumentTreeOwner::CDocumentTreeOwner
// ---------------------------------------------------------
//
CDocumentTreeOwner::CDocumentTreeOwner( NW_DOM_DocumentNode_t* aDocTree ) 
:   iDocTree( aDocTree )
    {
    }

// ---------------------------------------------------------
// CDocumentTreeOwner::~CDocumentTreeOwner
// ---------------------------------------------------------
//
CDocumentTreeOwner::~CDocumentTreeOwner() 
    {
    if ( iDocTree )
        {
        NW_DOM_DocumentNode_Delete( iDocTree );
        }
    }

// ---------------------------------------------------------
// CDocumentTreeOwner::SetDocTree
// ---------------------------------------------------------
//
void CDocumentTreeOwner::SetDocTree( NW_DOM_DocumentNode_t* aDocTree ) 
    {
    iDocTree = aDocTree;
    }

// ---------------------------------------------------------
// CStringOwner::CStringOwner
// ---------------------------------------------------------
//
CStringOwner::CStringOwner() 
:   iString( NULL )
    {
    }

// ---------------------------------------------------------
// CStringOwner::CStringOwner
// ---------------------------------------------------------
//
CStringOwner::CStringOwner( NW_String_t* aString ) 
:   iString( aString )
    {
    }

// ---------------------------------------------------------
// CStringOwner::~CStringOwner
// ---------------------------------------------------------
//
CStringOwner::~CStringOwner() 
    {
    if ( iString )
        {
        NW_String_delete( iString );
        }
    }

// ---------------------------------------------------------
// CStringOwner::SetString
// ---------------------------------------------------------
//
void CStringOwner::SetString( NW_String_t* aString ) 
    {
    iString = aString;
    }
