/*
* Copyright (c) 2002-2004 Nokia Corporation and/or its subsidiary(-ies).
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



#ifndef PushViewerAppServer_H
#define PushViewerAppServer_H

//  INCLUDES
#include <AknServerApp.h>
#include <ApaServerApp.h>


// CLASS DECLARATION

/**
*  ?one_line_short_description.
*  ?other_description_lines
*
*  @lib ?library
*  @since Series ?XX ?SeriesXX_version
*/
class CPushViewerAppServer : public CAknAppServer
    {
    public: // Constructors and destructor
    
        CPushViewerAppServer();
        
    public: // from CAknAppServer
    
        CApaAppServiceBase* CreateServiceL( TUid aServiceType ) const;
        
    protected: // from CPolicyServer
    
	    virtual TCustomResult CustomFailureActionL( const RMessage2& aMsg, 
	                                                TInt aAction, 
	                                                const TSecurityInfo& aMissing );
    };

#endif      // PushViewerAppServer_H   
            
// End of File
