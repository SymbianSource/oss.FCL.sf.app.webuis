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
#ifndef OPERATORMENUPRIVATECRKEYS_H
#define OPERATORMENUPRIVATECRKEYS_H

const TUid KCrUidOperatorMenu = {0x10008D5E};

// Vertical layout enabled in browserui when it is started 
// from operator menu( 0 not enabled 1 enabled)
const TUint32 KOpMenuVerticalLayoutEnabled =    0x0000001;

// Autoload images enabled in browserui when it is started 
// from operator menu( 0 not enabled 1 enabled)
const TUint32 KOpMenuAutoLoadImages =           0x0000002;

// Sets the font size for the browser ui when it is started 
// from operator menu (values 0"All Large",1 "Large",2 "Normal",3 "Small", 4 "All Small")
const TUint32 KOpMenuFontSize =                 0x0000003;

// Full screen enabled in browserui when it is started from 
// operator menu(1"Softkeys Only", 2 "Full Screen")
const TUint32 KOpMenuFullScreen =               0x0000004;

// Custom access point provided by the operator. If specified, 
// contains an id of a read only access point what is then set 
// as default  and it cannot be changed manually. If not defined, 
// operator menu works as before.
// from operator menu( 0 - not specified, otherwise contains the access point id)
const TUint32 KOpMenuDefAp =                    0x0000005;

#endif      // OPERATORMENUPRIVATECRKEYS_H
