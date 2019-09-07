#ifndef _SECURED_MENU_POPUP_H_
#define _SECURED_MENU_POPUP_H_

#include <RuntimeMenuItem.h>
#include <RemoteAuthentication.h>

int secPopupActionRenderFn(RuntimeMenuItem* item, uint8_t row, RenderFnMode mode, char* buffer, int bufferSize);

class ActivateSubMenuItem : public RuntimeMenuItem {
private:
	SubMenuItem* securedItem;
public:
	ActivateSubMenuItem(RuntimeRenderingFn customRenderFn, int activeStatus, MenuItem* next = NULL)
		: RuntimeMenuItem(MENUTYPE_ACTIVATE_SUBMENU, nextRandomId(), customRenderFn, activeStatus, 1, next) {
	}

	void setSecuredItem(SubMenuItem *secured) {
		securedItem = secured;
	}

	SubMenuItem* getSecuredItem() { return securedItem; }
};

/**
 * Secured menu popup is a detatched menu that is never presented remotely, it allows
 * for secured menus that can only be edited on the device after entering a pin.
 * It has a pin entry area, the ability to proceed (if the pin is correct) and the
 * ability to go back to the main menu.
 */
class SecuredMenuPopup {
private:
	TextMenuItem pinEntryItem;
	ActivateSubMenuItem actionProceedItem;
	ActivateSubMenuItem actionCancelItem;
	AuthenticationManager* authentication;
public:
	SecuredMenuPopup(AuthenticationManager *authentication);

	MenuItem* start(SubMenuItem* securedMenu);

	MenuItem* getRootItem() {
		return &pinEntryItem;
	}

	bool doesPinMatch() {
		return authentication->doesPinMatch(pinEntryItem.getTextValue());
	}
};

#endif // _SECURED_MENU_POPUP_H_
