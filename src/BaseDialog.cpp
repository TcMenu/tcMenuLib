/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "tcMenu.h"
#include "tcUtil.h"
#include "BaseGraphicalRenderer.h"
#include <BaseRenderers.h>
#include <RemoteConnector.h>
#include <BaseDialog.h>
#include <SwitchInput.h>

const char buttonOK[] PROGMEM = "ok";
const char buttonCancel[] PROGMEM = "cancel";
const char buttonClose[] PROGMEM = "close";
const char buttonAccept[] PROGMEM = "accept";

BaseDialog::BaseDialog() {
    headerPgm = nullptr;
    bitWrite(flags, DLG_FLAG_INUSE, false);
    button1 = button2 = BTNTYPE_NONE;
}

void BaseDialog::show(const char* headerPgm, bool allowRemote, CompletedHandlerFn completedHandler) {
    serdebugF("showing new dialog");
    setInUse(true);
    setRemoteAllowed(allowRemote);
    setRemoteUpdateNeededAll();
    this->headerPgm = headerPgm;
    this->completedHandler = completedHandler;
    internalSetVisible(true);
    needsDrawing = MENUDRAW_COMPLETE_REDRAW;
}

void BaseDialog::internalSetVisible(bool visible) {
    if(MenuRenderer::getInstance()->getRendererType() == RENDER_TYPE_BASE) {
        ((BaseMenuRenderer*)MenuRenderer::getInstance())->giveBackDisplay();
    }
    bitWrite(flags, DLG_FLAG_NEEDS_RENDERING, visible);
}

void BaseDialog::hide() {
    serdebugF("hide() - give back display");
    setRemoteUpdateNeededAll();

    // stop the renderer from doing any more rendering, and tell it to reset the menu
    setInUse(false);
    internalSetVisible(false);

    // clear down all structures.
    button1 = button2 = BTNTYPE_NONE;
    setNeedsDrawing(false);
    headerPgm = nullptr;
}

ButtonType BaseDialog::findActiveBtn(unsigned int currentValue) {
    ButtonType b = button2;
    if(currentValue==0) {
        b = (button1 == BTNTYPE_NONE) ? button2 : button1;
    }
    serdebugF4("findActiveBtn: ", currentValue, button1, button2);
    return b;
}

void BaseDialog::actionPerformed(int btnNum) {
    // must be done before hide, which resets the encoder
    ButtonType btn = findActiveBtn(btnNum);
    serdebugF2("User clicked button: ", btn);
    hide();
    if(completedHandler) completedHandler(btn, userData);
}

void BaseDialog::dialogRendering(unsigned int currentValue, bool userClicked) {
    if(currentValue != lastBtnVal) {
        setNeedsDrawing(true);
    }

    lastBtnVal = currentValue;

    if(userClicked) {
        actionPerformed((int)currentValue);
        return;
    }

    if(needsDrawing != MENUDRAW_NO_CHANGE) {
        internalRender((int)currentValue);
        setNeedsDrawing(false);
    }
}

bool BaseDialog::copyButtonText(char* data, int buttonNum, int currentValue, bool sel) {
    const char* tx;
    int bt = (buttonNum == 0) ? button1 : button2;
    switch(bt) {
    case BTNTYPE_ACCEPT:
        tx = buttonAccept;
        break;
    case BTNTYPE_CANCEL:
        tx = buttonCancel;
        break;
    case BTNTYPE_CLOSE:
        tx = buttonClose;
        break;
    default:
        tx = buttonOK;
        break;
    }

    strcpy_P(data, tx);
    if((button1 == BTNTYPE_NONE || button2 == BTNTYPE_NONE) || sel) {
        while(*data) {
            *data = toupper(*data);
            ++data;
        }
        return true;
    }
    return false;
}

void BaseDialog::copyIntoBuffer(const char* sz) {
    if(isInUse()) {
        char* buffer = MenuRenderer::getInstance()->getBuffer();
        uint8_t bufferSize = MenuRenderer::getInstance()->getBufferSize();
        strncpy(buffer, sz, bufferSize);
        int l = strlen(buffer);

        if(!isCompressedMode()) {
            for(int i=l; i<bufferSize; i++) {
                buffer[i]=32;
            }
        }

        buffer[bufferSize]=0;
        setNeedsDrawing(true);
        setRemoteUpdateNeededAll();
        
    }
}

void BaseDialog::setButtons(ButtonType btn1, ButtonType btn2, int defVal) {
    serdebugF3("Set buttons on dialog", btn1, btn2);
    button1 = btn1;
    button2 = btn2;
    int noOfOptions = (button1 != BTNTYPE_NONE && button2 != BTNTYPE_NONE)  ? 1 : 0;
    if(switches.getEncoder()) switches.getEncoder()->changePrecision(noOfOptions, defVal);
    setNeedsDrawing(true);
}

void BaseDialog::encodeMessage(TagValueRemoteConnector* remote) {
    remote->encodeDialogMsg(isInUse() ? DLG_VISIBLE : DLG_HIDDEN, button1, button2, headerPgm, MenuRenderer::getInstance()->getBuffer());
}

void BaseDialog::remoteAction(ButtonType btn) {
    serdebugF2("Remote clicked button: ", btn);
    hide();
    if(completedHandler) completedHandler(btn, userData);
}

//
// Menu based dialog
//

int dialogBackRenderFn(RuntimeMenuItem* item, uint8_t /*row*/, RenderFnMode mode, char* buffer, int bufferSize) {
    auto* renderer =  reinterpret_cast<BaseGraphicalRenderer*>(MenuRenderer::getInstance());
    auto* dlg = reinterpret_cast<MenuBasedDialog*>(renderer->getDialog());
    switch (mode) {
        case RENDERFN_INVOKE:
            dlg->remoteAction(BTNTYPE_CANCEL);
        case RENDERFN_NAME:
            safeProgCpy(buffer, dlg->getHeaderText(), bufferSize);
            return true;
        case RENDERFN_EEPROM_POS:
            return -1;
        case RENDERFN_VALUE:
            buffer[0] = 0;
            return true;
        default: return false;
    }
}

int dialogButtonRenderFn(RuntimeMenuItem* item, uint8_t /*row*/, RenderFnMode mode, char* buffer, int bufferSize) {
    auto* dlg = reinterpret_cast<MenuBasedDialog*>(MenuRenderer::getInstance()->getDialog());
    auto* btnItem = reinterpret_cast<LocalDialogButtonMenuItem*>(item);
    switch (mode) {
        case RENDERFN_INVOKE:
            dlg->actionPerformed(btnItem->getButtonNumber());
            return true;
        case RENDERFN_NAME:
            dlg->copyButtonText(buffer, btnItem->getButtonNumber(), bufferSize, btnItem->isActive());
            return true;
        case RENDERFN_EEPROM_POS:
            return -1;
        case RENDERFN_VALUE:
            buffer[0] = 0;
            return true;
        default: return false;
    }
}

RENDERING_CALLBACK_NAME_INVOKE(dialogTextRenderFn, textItemRenderFn, "Msg: ", -1, NO_CALLBACK)

MenuBasedDialog::MenuBasedDialog() :
        backItem(dialogBackRenderFn, nullptr),
        bufferItem(dialogTextRenderFn, nextRandomId(), 20, nullptr),
        btn1Item(dialogButtonRenderFn, nextRandomId(), 0, nullptr),
        btn2Item(dialogButtonRenderFn, nextRandomId(), 1, nullptr) {
    bitWrite(flags, DLG_FLAG_SMALLDISPLAY, false);
    bitWrite(flags, DLG_FLAG_MENUITEM_BASED, true);
    bufferItem.setReadOnly(true);
}

void MenuBasedDialog::copyIntoBuffer(const char *sz) {
    bufferItem.setTextValue(sz);
}

void MenuBasedDialog::internalSetVisible(bool visible) {
    bitWrite(flags, DLG_FLAG_NEEDS_RENDERING, false);
    if(visible) {
        auto* renderer =  reinterpret_cast<BaseGraphicalRenderer*>(MenuRenderer::getInstance());
        auto& factory = static_cast<ConfigurableItemDisplayPropertiesFactory&>(renderer->getDisplayPropertiesFactory());

        backItem.setNext(&bufferItem);
        bufferItem.setNext(&btn1Item);
        btn1Item.setNext(&btn2Item);

        btn1Item.setVisible(button1 != BTNTYPE_NONE);
        btn2Item.setVisible(button2 != BTNTYPE_NONE);

        factory.addGridPosition(&btn1Item, GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_CENTER_NO_VALUE, 2, 1, 2, 0));
        factory.addGridPosition(&btn1Item, GridPosition(GridPosition::DRAW_TEXTUAL_ITEM, GridPosition::JUSTIFY_CENTER_NO_VALUE, 2, 2, 2, 0));

        MenuItem* pItem = &backItem;
        while(pItem) {
            pItem->setActive(false);
            pItem->setEditing(false);
            pItem = pItem->getNext();
        }
        btn1Item.setActive(true);

        menuMgr.setCurrentMenu(&backItem);
    }
    else menuMgr.setCurrentMenu(menuMgr.getRoot());
}

void MenuBasedDialog::insertMenuItem(MenuItem* item) {
    if(!item) return;

    item->setNext(bufferItem.getNext());
    bufferItem.setNext(item);
}
