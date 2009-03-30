/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies).
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


/*
** WARNING
**
** DO NOT EDIT - THIS CODE IS AUTOMATICALLY GENERATED
**               FROM A DATA FILE BY THE DICTIONARY CREATION PROGRAM
**
** This file generated on Fri Sep 26 19:53:43 2003
**                        (coordinated universal time)
**
** Command line: dict_creator si.dict si_dict.c si_dict.h
*/

#ifndef HEADER_GUARD_si_dict_h
#define HEADER_GUARD_si_dict_h


#include <nw_wbxml_dictionary.h>


#ifdef __cplusplus
extern "C"
{
#endif


#define NW_SI_PublicId 5

typedef enum NW_SI_ElementToken_0_e{
	NW_SI_ElementToken_si = 0x0005,
	NW_SI_ElementToken_indication = 0x0006,
	NW_SI_ElementToken_info = 0x0007,
	NW_SI_ElementToken_item = 0x0008
}NW_SI_ElementToken_0_t;


typedef enum NW_SI_AttributeToken_0_e{
	NW_SI_AttributeToken_action_signal_none = 0x0005,
	NW_SI_AttributeToken_action_signal_low = 0x0006,
	NW_SI_AttributeToken_action_signal_medium = 0x0007,
	NW_SI_AttributeToken_action_signal_high = 0x0008,
	NW_SI_AttributeToken_action_delete = 0x0009,
	NW_SI_AttributeToken_created = 0x000a,
	NW_SI_AttributeToken_href = 0x000b,
	NW_SI_AttributeToken_href_http___ = 0x000c,
	NW_SI_AttributeToken_href_http___www_ = 0x000d,
	NW_SI_AttributeToken_href_https___ = 0x000e,
	NW_SI_AttributeToken_href_https___www_ = 0x000f,
	NW_SI_AttributeToken_si_expires = 0x0010,
	NW_SI_AttributeToken_si_id = 0x0011,
	NW_SI_AttributeToken_class = 0x0012,
	NW_SI_AttributeToken_action = 0x0013,
	NW_SI_AttributeToken__com_ = 0x0085,
	NW_SI_AttributeToken__edu_ = 0x0086,
	NW_SI_AttributeToken__net_ = 0x0087,
	NW_SI_AttributeToken__org_ = 0x0088
}NW_SI_AttributeToken_0_t;

extern const NW_WBXML_Dictionary_t NW_SI_WBXMLDictionary;

/*
** WARNING
**
** DO NOT EDIT - THIS CODE IS AUTOMATICALLY GENERATED
**               FROM A DATA FILE BY THE DICTIONARY CREATION PROGRAM
**
** This file generated on Fri Sep 26 19:53:43 2003
**                        (coordinated universal time)
**
** Command line: dict_creator si.dict si_dict.c si_dict.h
*/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
