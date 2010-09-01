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
* Description:  Push Mtm settings class declaration
*
*/



#ifndef PUSHMTMSETTINGS_H
#define PUSHMTMSETTINGS_H

// INCLUDE FILES

#include <e32base.h>
#include <e32std.h>

// FORWARD DECLARATIONS

class CPushInitiatorList;
class CRepository;

// CLASS DECLARATION

/**
* CPushMtmSettings supports an API with which push mtm settings can be 
* read or edited. These settings are stored in Central Repository.
*
* On construction with the static NewL or NewLC methods the new instance is 
* initialized with the stored values (with the usage of the LoadL method).
* Changes made are stored in the associated file if the SaveL method 
* is called. It is possible that there are more than one instances of this 
* class. The instances manipulate on the same CenRep.
*
* The instances do not update their settings values from the Repository 
* automatically if it is modified. If an instance's owner wants that the 
* settings values be up-to-date, then it should call LoadL or reconstruct a 
* new CPushMtmSettings object.
*/
class CPushMtmSettings : public CBase
    {
    public: // Type definitions

        enum TServiceLoading    ///< Service loading type
            {
            EAutomatic,         ///< Automatic loading
            EManual             ///< Manual loading
            };

    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        * @return New CPushMtmSettings object.
        */
        IMPORT_C static CPushMtmSettings* NewL();

        /**
        * Two-phased constructor.
        * @return New CPushMtmSettings object.
        */
        IMPORT_C static CPushMtmSettings* NewLC();

        /**
        * Destructor.
        */
        IMPORT_C virtual ~CPushMtmSettings();

    public: // New functions

        // ==================== Setters =====================

        /**
        * Set service reception.
        * @param aReceive If this parameter is ETrue, service 
        *        reception is on. Otherwise it is off.
        * @return none
        */
        IMPORT_C void SetServiceReception( TBool aReceive );

        /**
        * Set service loading type.
        * @param aType Service loading type.
        * @return none
        */
        IMPORT_C void SetServiceLoadingType( TServiceLoading aType );

        // ==================== Getters =====================

        /**
        * Return wap push service reception setting.
        * @return Message reception setting.
        *         It is ETrue, if setting is on.
        *         It is EFalse, if setting is off.
        */
        IMPORT_C TBool ServiceReception() const;

        /**
        * Return wap push service loading type setting.
        * @return Service loading type setting.
        */
        IMPORT_C TServiceLoading ServiceLoadingType() const;

        /**
        * Return the 'White list' i.e. list of the addresses of push 
        * initiators from where we accept push messages.
        * @return The push initiator list.
        */
        IMPORT_C CPushInitiatorList& PushInitiatorList() const;

        // ==================== Saving and loading =====================

        /**
        * The data stored in the permanent store is internalized into the 
        * instance of this class.
        * The existing values will be overwritten.
        * @return none
        */
        IMPORT_C void LoadL();

        /**
        * Save changes if one ore more of then have been changed.
        * @param aForce Force saving to permanent store.
        * @return None.
        */
        IMPORT_C void SaveL( const TBool aForce = EFalse );

        /**
        * Restore factory settings from the Shared Data.
        * Note that it issues SaveL() to persist the settings!
        * @return None.
        */
        IMPORT_C void RestoreFactorySettingsL();

    private: // Constructors
        
        /**
        * Symbian OS constructor.
        */
        void ConstructL();

        /**
        * Constructor.
        */
        CPushMtmSettings();

    private: // New functions

        /**
        * Reset data members.
        * It is called at the beginning of LoadL(), for example.
        * @return none
        */
        void Reset();

        /**
        * Externalize into the Repository.
        * @param aRepository Central Repository
        * @return none
        */
        void ExternalizeL( CRepository& aRepository ) const;

        /**
        * Internalize from the Repository.
        * @param aRepository Central Repository
        * @return none
        */
        void InternalizeL( CRepository& aRepository );

        /**
        * Parse and process the streamed White List.
        * @param aStreamedBuf The streamed White List.
        * @return None.
        */
        void ParseAndProcessWhiteListBufL( const TDesC& aStreamedBuf );

    private: // Data

        TBool               iServiceReception; ///< Setting value.
        TServiceLoading     iServiceLoadingType; ///< Setting value.
        CPushInitiatorList* iPushInitiatorList; ///< The 'white list'. Owned.

        /// The following members indicate if a value has been changed.
        TBool               iServiceReceptionChanged;
        TBool               iServiceLoadingTypeChanged;

        TBool               iFeatureManagerInitialized;
    };

#endif // PUSHMTMSETTINGS_H

// End of File
