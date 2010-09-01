/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Class which implements handling Hardware Key events.
*                This class implements methods of MRemConCoreApiTargetObserver
*
*/


#ifndef BAVPHWKEYEVENTS_H
#define BAVPHWKEYEVENTS_H

// INCLUDES
#include <RemConCoreApiTargetObserver.h>

// FORWARD DECLARATIONS
class CRemConInterfaceSelector;
class CRemConCoreApiTarget;
class MRemConCoreApiTargetObserver;

// CONSTANTS

// CLASS DECLARATION
/**
*  MBavpHwKeyEventsObserver class for hardware key events
*  This class implements methods of MRemConCoreApiTargetObserver
*  @lib npBrowserVideoPlugin.lib
*  @since 3.2
*/
class MBavpHwKeyEventsObserver
    {
    public:
        /**
        * Called when volume level should be changed.
        * @since 3.2
    	* @param aVolumeChange: +1 change volume up
        *                       -1 change volume down
        * @return void
        */
        virtual void BavpHwKeyVolumeChange( TInt aVolumeChange ) = 0;

        /**
        * Called when a media hardware key (play, pause...) was presed.
        * @since 3.2
    	* @param aCommand: Command ID representing Play, Pause, ...
        * @return void
        */
        // Incr15 SCO item that was cancelled.
        // virtual void BavpHwKeyCommand( TBavpCommandIds aCommand ) = 0;
    };
    
/**
*  CBavpHwKeyEvents class to handle Side Volume Key events
*  @lib npBrowserVideoPlugin.lib
*  @since 3.2
*/
class CBavpHwKeyEvents : public CBase,
					     public MRemConCoreApiTargetObserver
    {
    public: // Constructors and destructor

        /**
        * Symbian constructor
        * @param aObserver observer to implement callback functions
        */
        static CBavpHwKeyEvents* NewL( MBavpHwKeyEventsObserver& aObserver );

        /**
        * Destructor.
        */
        ~CBavpHwKeyEvents();

    private: // Constructors and destructor

    	/**
    	* Default constructor.
        * @since 3.2
        */
        CBavpHwKeyEvents( MBavpHwKeyEventsObserver& aObserver );

        /**
        * Constructor.
        * @since 3.2
        */
         void ConstructL();
         
    protected: // From MRemConCoreApiTargetObserver
    
		/* Handles hardware key events
        * @since 3.2
    	*/
    	void MrccatoCommand( TRemConCoreApiOperationId aOperationId, 
    	 					 TRemConCoreApiButtonAction aButtonAct );

    private:

        /**
        * Change volume depending on the level of increase or decrease
        * @since 3.2
        */
        void DoChangeVolume( );

        /**
        * VolumeTimerCallback is the callback function called from change volume timer.
        * @since 3.2
        */
        static TInt VolumeTimerCallback( TAny* aThis );

    private:    // data
    
        MBavpHwKeyEventsObserver& iObserver;
        
        CRemConInterfaceSelector* iInterfaceSelector;
        
        CRemConCoreApiTarget* iCoreTarget;
        
        CPeriodic* iVolumeTimer;
            
        // Indicates size of volume level change
        TInt iChange;
        };

#endif // BAVPHWKEYEVENTS_H

// End of File
