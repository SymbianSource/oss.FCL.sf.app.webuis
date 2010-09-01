/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef __BROWSER_APPLICATION_H
#define __BROWSER_APPLICATION_H

#include <aknapp.h>

const TUid KUidBrowserApplication = { 0x10008D39 };

class CBrowserApplication : public CAknApplication
{
protected:
	TUid AppDllUid() const;
	CApaDocument* CreateDocumentL();
    void NewAppServerL(CApaAppServer*& aAppServer);
};

#endif