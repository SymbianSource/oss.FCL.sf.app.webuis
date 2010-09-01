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
* Description:  Proxy definition.
*
*/



// INCLUDE FILES

#include "PushContentHandlerDef.hrh"
#include "CUnknownContentHandler.h"
#include "CSIContentHandler.h"
#include "CSLContentHandler.h"
#include "CCOContentHandler.h"
#include "CMultiPartMixedContentHandler.h"
#include "CMultiPartRelAndAltContentHandler.h"
#include <e32std.h>
#include <ecom/implementationproxy.h>


const TImplementationProxy ImplementationTable[] = 
	{
        IMPLEMENTATION_PROXY_ENTRY( EUidPushUnknownContentHandler, CUnknownContentHandler::NewL ),
        IMPLEMENTATION_PROXY_ENTRY( EUidPushSIContentHandler, CSIContentHandler::NewL ),
        IMPLEMENTATION_PROXY_ENTRY( EUidPushSLContentHandler, CSLContentHandler::NewL ),
        IMPLEMENTATION_PROXY_ENTRY( EUidPushCOContentHandler, CCOContentHandler::NewL ),
        IMPLEMENTATION_PROXY_ENTRY( EUidPushMultiPartMixedContentHandler, CMultiPartMixedContentHandler::NewL ),
        IMPLEMENTATION_PROXY_ENTRY( EUidPushMultiPartRelAndAltContentHandler, CMultiPartRelAndAltContentHandler::NewL )
	};

EXPORT_C const TImplementationProxy* ImplementationGroupProxy( TInt& aTableCount )
	{
	aTableCount = sizeof(ImplementationTable) / sizeof(TImplementationProxy);

	return ImplementationTable;
	}

