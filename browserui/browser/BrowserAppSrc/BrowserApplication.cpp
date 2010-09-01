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
*     Browser application class
*     
*
*/


#include "BrowserApplication.h"
#include "BrowserAppDocument.h"
#include "BrowserAppServer.h"


TUid CBrowserApplication::AppDllUid() const
{
	return KUidBrowserApplication;
}

CApaDocument* CBrowserApplication::CreateDocumentL()
{
	return new (ELeave) CBrowserAppDocument(*this);
}

void CBrowserApplication::NewAppServerL(CApaAppServer*& aAppServer)
    {
    aAppServer = new(ELeave) CBrowserAppServer;
    }


