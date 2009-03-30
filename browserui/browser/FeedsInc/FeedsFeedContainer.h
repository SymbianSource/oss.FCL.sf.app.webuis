/*
* Copyright (c) 2005-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  A container to browse a given feed.
*
*/


#ifndef FEED_CONTAINER_H
#define FEED_CONTAINER_H


// INCLUDES
#include <coecntrl.h>
#include <coemop.h>
#include <aknnavidecoratorobserver.h>
#include <BrCtlSpecialLoadObserver.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class CAknNavigationDecorator;
class CBrCtlInterface;
//class CBufFlat;
class CFeed;
class CFeedsFeedView;
//class CXmlEncoding;
class MApiProvider;
class CFeedsEntity;

// CLASS DECLARATION


/**
*  A container to browse a given feed.
*  @lib FeedsEngine.lib
*  @since 3.0
*/
class CFeedsFeedContainer : public CCoeControl, public MBrCtlSpecialLoadObserver
    {
    public:
        /**
        * Two-phased constructor.
        */
        static CFeedsFeedContainer* NewL(
        	CFeedsFeedView* aView,
            MApiProvider& aApiProvider);

        /**
        * Destructor.
        */        
        virtual ~CFeedsFeedContainer();


    public: // From CoeControl
        /**
        * Handles key event.
        *
        * @param aKeyEvent The key event.
        * @param aType The type of the event.
        * @return Indicates whether the key event was used by this control or not
        */
        virtual TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);

#ifdef __SERIES60_HELP
        /**
        * Get help context for the control.
        *
        * @since ?
        * @param aContext The context that is filled in.
        * @return None.
        */
        virtual void GetHelpContext(TCoeHelpContext& aContext) const;
#endif // __SERIES60_HELP

        /**
        * Sets this control as visible or invisible.
        *
        * @since ?
        * @param aVisible ETrue to make the control visible, EFalse to make it invisible.
        * @return Void
        */
        virtual void MakeVisible(TBool aVisible);
        

    protected: // From CoeControl
        /**
        * Called by framework when the view size is changed.
        *
        * @since ?
        * @return void.
        */
        virtual void SizeChanged();

        /**
        * Called by the framework when a display resource changes (i.e. skin or layout).
        *
        * @since ?
        * @return void.
        */
        virtual void HandleResourceChange(TInt aType);

        /**
        * Returns number of components.
        *
        * @since ?
        * @return Number of component controls.
        */
        virtual TInt CountComponentControls() const;

        /**
        * Returns pointer to particular component.
        *
        * @since ?
        * @param aIndex Index whose control's pointer has to returned.
        * @return Pointer to component control
        */
        virtual CCoeControl* ComponentControl(TInt aIndex) const;


    public:  // From MBrCtlSpecialLoadObserver
        /**
        * Request to create a network connection.
        *
        * @since 2.8
        * @param aConnectionPtr A pointer to the new connection. If NULL, the 
                                proxy filter will automatically create a network connection
        * @param aSockSvrHandle A handle to the socket server.
        * @param aNewConn A flag if a new connection was created. If the 
                          connection is not new, proxy filter optimization will not 
                          read the proxy again from CommsBd.
        * @param aBearerType The bearer type of the new connection
        * @return void
        */
        virtual void NetworkConnectionNeededL(TInt* aConnectionPtr, TInt* aSockSvrHandle,
                TBool* aNewConn, TApBearerType* aBearerType);

        /**
        * Request the host applicaion to handle non-http request.
        *
        * @since 2.8
        * @param aUrl The non-http(s) or file URL
        * @param aParamList Parameters to pass to the host application. 
                            Contain referer header. It could be NULL
        * @return ETrue is handled by the host application. EFlase if not
        */
        virtual TBool HandleRequestL(RArray<TUint>* aTypeArray, CDesCArrayFlat* aDesArray);

        /**
        * Request the host applicaion to handle downloads
        *
        * @since 2.8
        * @param aTypeArray array of download parameter types
        * @param aDesArray array of values associated with the types in the type array
        * @return ETrue is handled by the host application. EFlase if not
        */
        virtual TBool HandleDownloadL(RArray<TUint>* aTypeArray, CDesCArrayFlat* aDesArray);

    public:  // New methods.
        /**
        * Clears the navigation pane.
        *
        * @since 3.0
        * @return void
        */
        void ClearNavigationPane();

        /**
        * Sets the current feed
        *
        * @since 3.0
        * @param aFeed The new feed.
        * @param aInitialItem The initial item to show.
        * @return Void
        */
        void SetCurrentFeedL(CFeedsEntity& aFeed, TInt aInitialItem);

        /**
        * Returns the current item's url.
        *
        * @since 3.0
        * @return void
        */
        const TDesC& GetItemUrl();

        /**
        * Returns the index of the current item.
        *
        * @since 3.0
        * @return The index.
        */
        TInt CurrentItem();

        /**
        * Returns the number of items.
        *
        * @since 5.0
        * @return The number of items
        */
        TInt ItemCount() const;

        /**
        * Shows the next item if possible.
        *
        * @since 3.0
        * @return The index.
        */
        void ShowNextItemL();

        /**
        * Shows the prev item if possible.
        *
        * @since 3.0
        * @return The index.
        */
        void ShowPrevItemL();


    private:  // New methods.
        /**
        * C++ default constructor.
        */
        CFeedsFeedContainer
                   ( CFeedsFeedView* aView,
                             MApiProvider& aApiProvider );        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();

        /**
        * Handles the changes needed to the Navigation Pane.
        *
        * @since 3.0
        * @return void
        */
        void UpdateNavigationPaneL();

        /**
        * Shows the given feed item.
        *
        * @since 3.0
        * @return void
        */
        void ShowFeedItemL();
        
        /**
        * Loads the template html file.
        *
        * @since 3.1
        * @param aTemplateName The name of the template.
        * @return void.
        */
        void LoadTemplateL(const TDesC& aTemplateName);

        /**
        * Loads and resolves the tokens in the template html file.
        *
        * @since 3.1
        * @param aTitle The title.
        * @param aTimestamp The timestamp.
        * @param aDescription The description.
        * @param aUrl The url.
        * @param aShowFullStory The "show full story" string.
        * @param aShowPrev If ETrue the "prev" element is shown.
        * @param aShowNext If ETrue the "next" element is shown.
        * @return The resolved buffer.
        */
        HBufC* ResolveTemplateL(const TDesC& aTitle, const TDesC& aTimestamp, 
                const TDesC& aDescription, const TDesC& aUrl, 
                const TDesC& aEnclosure);

        /**
        * If need be copy the template from ROM.
        *
        * @since 3.1
        * @param aName The name of the template file.
        * @return void
        */
        void EnsureTemplateL(const TDesC& aName);
        
        /**
        * Extract the given BrCtl parameter from the list.
        * @param aParamTypeToFind Extract this parameter.
        * @param aTypeArray array of download parameter types.
        * @param aDesArray array of values associated with the types in the type array.
        * @param aParamFound Output: ETrue if the parameter was found.
        * @return A TPtrC pointer for the value.
        * @since 3.1
        */
        TPtrC ExtractBrCtlParam( 
                                 TUint aParamTypeToFind, 
                                 RArray<TUint>* aTypeArray,
                                 CDesCArrayFlat* aDesArray, 
                                 TBool& aParamFound ) const;

    public:  // Friends
        friend class CFeedsFeedView;


    protected:
    	CFeedsFeedView*				iView;				// not owned
    	MApiProvider&				iApiProvider;       // not owned
    	CBrCtlInterface*			iBrowserControl;
    	CAknNavigationDecorator*	iNaviPaneTabsGroup;

        HBufC*                       iTemplate;
        TInt                         iTitleCount;
        TInt                         iWebUrlCount;
        TInt                         iDateCount;
        TInt                         iDescriptionCount;
        TInt                         iEnclosureCount;
        TInt                         iShowPrevCount;
        TInt                         iShowNextCount;
        
        CFeedsEntity*                iFeed;
        TInt                         iCurrentItem;
        TPtrC                        iUrl;
    };

#endif      // FEED_CONTAINER_H
            
// End of File