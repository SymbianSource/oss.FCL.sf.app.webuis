/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/

#ifndef BAVPNPOBJECT
#define BAVPNPOBJECT

#include <e32base.h>
#include <npscript.h>

class CBavpPlugin;

typedef struct
{
    NPObject object;
    CBavpPlugin *plugin;
    NPObject* stateChangedCallback;
    NPP nppInstance;
} BavpNPObject;  

BavpNPObject* BavpNPObject_new(NPP instance);
void BavpNPObject_stateChanged(BavpNPObject* obj, char* state);

#endif
