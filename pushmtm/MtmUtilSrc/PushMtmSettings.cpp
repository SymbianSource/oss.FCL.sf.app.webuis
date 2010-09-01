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
* Description:  CPushMtmSettings class definition.
*
*/



// INCLUDE FILES

#include "PushMtmSettings.h"
#include "PushInitiatorList.h"
#include "PushMtmUtilPanic.h"
#include "PushMtmLog.h"
#include "PushMtmPrivateCRKeys.h"
#include <centralrepository.h> 
#include <PushEntry.h>
#include <SysUtil.h>
#include <bldvariant.hrh>
#include <FeatMgr.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushMtmSettings::NewL
// ---------------------------------------------------------
//
EXPORT_C CPushMtmSettings* CPushMtmSettings::NewL()
    {
    CPushMtmSettings* self = CPushMtmSettings::NewLC();
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------
// CPushMtmSettings::NewLC
// ---------------------------------------------------------
//
EXPORT_C CPushMtmSettings* CPushMtmSettings::NewLC()
    {
    PUSHLOG_ENTERFN("CPushMtmSettings::NewLC")

    CPushMtmSettings* self = new (ELeave) CPushMtmSettings();
    CleanupStack::PushL( self );
    self->ConstructL();

    PUSHLOG_LEAVEFN("CPushMtmSettings::NewLC")
    return self;
    }

// ---------------------------------------------------------
// CPushMtmSettings::~CPushMtmSettings
// ---------------------------------------------------------
//
EXPORT_C CPushMtmSettings::~CPushMtmSettings()
    {
    PUSHLOG_ENTERFN("CPushMtmSettings::~CPushMtmSettings")
    delete iPushInitiatorList;
    if ( iFeatureManagerInitialized )
        {
        FeatureManager::UnInitializeLib();
        iFeatureManagerInitialized = EFalse;
        }
    PUSHLOG_LEAVEFN("CPushMtmSettings::~CPushMtmSettings")
    }

// ---------------------------------------------------------
// CPushMtmSettings::SetServiceReception
// ---------------------------------------------------------
//
EXPORT_C void CPushMtmSettings::SetServiceReception( TBool aReceive )
    {
    PUSHLOG_WRITE_FORMAT
        ("CPushMtmSettings::SetServiceReception <%d>",aReceive?1:0)
    if ( iServiceReception != aReceive )
        {
        iServiceReception = aReceive;
        iServiceReceptionChanged = ETrue;
        }
    }

// ---------------------------------------------------------
// CPushMtmSettings::SetServiceLoadingType
// ---------------------------------------------------------
//
EXPORT_C void CPushMtmSettings::SetServiceLoadingType( TServiceLoading aType )
    {
    PUSHLOG_WRITE_FORMAT
        ("CPushMtmSettings::SetServiceLoadingType <%d>",(TInt)aType)
    if ( iServiceLoadingType != aType )
        {
        iServiceLoadingType = aType;
        iServiceLoadingTypeChanged = ETrue;
        }
    }

// ---------------------------------------------------------
// CPushMtmSettings::ServiceReception
// ---------------------------------------------------------
//
EXPORT_C TBool CPushMtmSettings::ServiceReception() const
    {
    PUSHLOG_WRITE_FORMAT
        ("CPushMtmSettings::ServiceReception <%d>",(TInt)iServiceReception)
    return iServiceReception;
    }

// ---------------------------------------------------------
// CPushMtmSettings::ServiceLoadingType
// ---------------------------------------------------------
//
EXPORT_C CPushMtmSettings::TServiceLoading 
    CPushMtmSettings::ServiceLoadingType() const
    {
    PUSHLOG_WRITE_FORMAT
        ("CPushMtmSettings::ServiceLoadingType <%d>",(TInt)iServiceLoadingType)
    return iServiceLoadingType;
    }

// ---------------------------------------------------------
// CPushMtmSettings::PushInitiatorList
// ---------------------------------------------------------
//
EXPORT_C CPushInitiatorList& CPushMtmSettings::PushInitiatorList() const
    {
    __ASSERT_DEBUG( iPushInitiatorList, UtilPanic( EPushMtmUtilPanNull ) );
    return *iPushInitiatorList;
    }

// ---------------------------------------------------------
// CPushMtmSettings::LoadL
// ---------------------------------------------------------
//
EXPORT_C void CPushMtmSettings::LoadL()
    {
    PUSHLOG_ENTERFN("CPushMtmSettings::LoadL")

    Reset(); // Remove outdated data.

    // Open the Repository for reading:
    CRepository* repository = CRepository::NewL( KCRUidPushMtm );
    CleanupStack::PushL( repository );

    InternalizeL( *repository );

    CleanupStack::PopAndDestroy( repository ); // repository

    // ******************** Reset indicators ***********************
    iServiceReceptionChanged = EFalse;
    iServiceLoadingTypeChanged = EFalse;
    __ASSERT_DEBUG( iPushInitiatorList, UtilPanic( EPushMtmUtilPanNull ) );
    iPushInitiatorList->ResetChanged();

    PUSHLOG_LEAVEFN("CPushMtmSettings::LoadL")
    }

// ---------------------------------------------------------
// CPushMtmSettings::SaveL
// ---------------------------------------------------------
//
EXPORT_C void CPushMtmSettings::SaveL( const TBool aForce )
    {
    PUSHLOG_ENTERFN("CPushMtmSettings::SaveL")
    PUSHLOG_WRITE_FORMAT(" aForce <%d>",aForce?1:0)
    PUSHLOG_WRITE_FORMAT(" SR changed <%d>",iServiceReceptionChanged?1:0)
    PUSHLOG_WRITE_FORMAT(" SL changed <%d>",iServiceLoadingTypeChanged?1:0)
    __ASSERT_DEBUG( iPushInitiatorList, UtilPanic( EPushMtmUtilPanNull ) );
    PUSHLOG_WRITE_FORMAT(" WL changed <%d>",iPushInitiatorList->Changed()?1:0)

    if ( aForce || ( iServiceReceptionChanged || 
                     iServiceLoadingTypeChanged || 
                     iPushInitiatorList->Changed() ) )
        {
        // Save the settings to Repository.
        CRepository* repository = CRepository::NewL( KCRUidPushMtm );
        CleanupStack::PushL( repository );

        ExternalizeL( *repository );

        CleanupStack::PopAndDestroy( repository ); // repository

        // ***************** Reset indicators ***********************
        iServiceReceptionChanged = EFalse;
        iServiceLoadingTypeChanged = EFalse;
        __ASSERT_DEBUG( iPushInitiatorList, 
                        UtilPanic( EPushMtmUtilPanNull ) );
        iPushInitiatorList->ResetChanged();
        }

    PUSHLOG_LEAVEFN("CPushMtmSettings::SaveL")
    }

// ---------------------------------------------------------
// CPushMtmSettings::RestoreFactorySettingsL
// ---------------------------------------------------------
//
EXPORT_C void CPushMtmSettings::RestoreFactorySettingsL()
    {
    PUSHLOG_ENTERFN("CPushMtmSettings::RestoreFactorySettingsL")

    // The factory settings are stored in the shared data.
    CRepository* repository = CRepository::NewL( KCRUidPushMtm );
    CleanupStack::PushL( repository );
    User::LeaveIfError( repository->Reset() );
    PUSHLOG_WRITE(" repository->Reset OK")
    CleanupStack::PopAndDestroy( repository ); // repository

    // Load up-to-date values.
    LoadL();

    PUSHLOG_LEAVEFN("CPushMtmSettings::RestoreFactorySettingsL")
    }

// ---------------------------------------------------------
// CPushMtmSettings::ConstructL
// ---------------------------------------------------------
//
void CPushMtmSettings::ConstructL()
    {
    // Create an empty push initiator list.
    iPushInitiatorList = CPushInitiatorList::NewL();
    FeatureManager::InitializeLibL();
    iFeatureManagerInitialized = ETrue;
    // Initialize:
    LoadL();
    }

// ---------------------------------------------------------
// CPushMtmSettings::CPushMtmSettings
// ---------------------------------------------------------
//
CPushMtmSettings::CPushMtmSettings() 
:   CBase(), 
    iServiceReception( ETrue ), 
    iServiceLoadingType( EManual ), 
    iServiceReceptionChanged ( EFalse ), 
    iServiceLoadingTypeChanged ( EFalse ),
    iFeatureManagerInitialized( EFalse )
    {
    }

// ---------------------------------------------------------
// CPushMtmSettings::Reset
// ---------------------------------------------------------
//
void CPushMtmSettings::Reset()
    {
    PUSHLOG_ENTERFN("CPushMtmSettings::Reset")
    __ASSERT_DEBUG( iPushInitiatorList, UtilPanic( EPushMtmUtilPanNull ) );
    iPushInitiatorList->ResetAndDestroy();
    PUSHLOG_LEAVEFN("CPushMtmSettings::Reset")
    }

// ---------------------------------------------------------
// CPushMtmSettings::ExternalizeL
// ---------------------------------------------------------
//
void CPushMtmSettings::ExternalizeL( CRepository& aRepository ) const
    {
    PUSHLOG_ENTERFN("CPushMtmSettings::ExternalizeL")

    __ASSERT_DEBUG( iPushInitiatorList, UtilPanic( EPushMtmUtilPanNull ) );

    TInt value;
    PUSHLOG_WRITE_FORMAT
        (" iServiceReception <%d>",(TInt)iServiceReception)
    value = iServiceReception ? 1 : 0;
    User::LeaveIfError( aRepository.Set( KPushMtmServiceReception, value ) );
    PUSHLOG_WRITE(" SR written")
    
    PUSHLOG_WRITE_FORMAT
        (" iServiceLoadingType <%d>",(TInt)iServiceLoadingType)
    value = ( iServiceLoadingType == CPushMtmSettings::EManual ) ? 1 : 0;
    User::LeaveIfError( aRepository.Set( KPushMtmServiceLoading, value ) );
    PUSHLOG_WRITE(" SL written")
    
    iPushInitiatorList->ExternalizeL( aRepository );

    PUSHLOG_LEAVEFN("CPushMtmSettings::ExternalizeL")
    }

// ---------------------------------------------------------
// CPushMtmSettings::InternalizeL
// ---------------------------------------------------------
//
void CPushMtmSettings::InternalizeL( CRepository& aRepository )
    {
    PUSHLOG_ENTERFN("CPushMtmSettings::InternalizeL")

    __ASSERT_DEBUG( iPushInitiatorList, UtilPanic( EPushMtmUtilPanNull ) );
    
    TInt value(0);
    User::LeaveIfError( aRepository.Get( KPushMtmServiceReception, value ) );
    PUSHLOG_WRITE_FORMAT(" KPushMtmServiceReception <%d>",(TInt)value)
    iServiceReception = ( value == 1 ) ? ETrue : EFalse;
    PUSHLOG_WRITE(" SR loaded")
    
    User::LeaveIfError( aRepository.Get( KPushMtmServiceLoading, value ) );
    PUSHLOG_WRITE_FORMAT(" KPushMtmServiceLoading <%d>",(TInt)value)
    iServiceLoadingType = ( value == 1 ) ? CPushMtmSettings::EManual : 
                                           CPushMtmSettings::EAutomatic;
    PUSHLOG_WRITE(" SL loaded")
    
    iPushInitiatorList->InternalizeL( aRepository );

    PUSHLOG_LEAVEFN("CPushMtmSettings::InternalizeL")
    }

//  End of File.
