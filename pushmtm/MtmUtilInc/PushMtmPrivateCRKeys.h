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
#ifndef PUSHMTMPRIVATECRKEYS_H
#define PUSHMTMPRIVATECRKEYS_H

const TUid KCRUidPushMtm = {0x10008D42};

// Service reception: on/off
const TUint32 KPushMtmServiceReception =    0x0000001;

// Service loading: automatic/manual
const TUint32 KPushMtmServiceLoading =      0x0000002;

// Read only White List
const TUint32 KPushMtmWhiteList =           0x0000003;

//To Enable to Disable PUSH_SL Service in Run-time
const TUint32 KPushMtmServiceEnabled   =   0x0000004; 

#endif      // PUSHMTMPRIVATECRKEYS_H