/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "tcMenu.h"
#include "tcUtil.h"
#include <BaseRenderers.h>
#include <RemoteConnector.h>
#include <BaseDialog.h>
#include <SwitchInput.h>

const char buttonOK[] PROGMEM = "ok";
const char buttonCancel[] PROGMEM = "cancel";
const char buttonClose[] PROGMEM = "close";
const char buttonAccept[] PROGMEM = "accept";

BaseDialog::BaseDialog() {
    headerPgm = NULL;
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
    needsDrawing = MENUDRAW_COMPLETE_REDRAW;
}

void BaseDialog::hide() {
    serdebugF("hide() - give back display");

    setRemoteUpdateNeededAll();

    // stop the renderer from doing any more rendering, and tell it to reset the menu
    setInUse(false);
    if(MenuRenderer::getInstance()->getRendererType() == RENDERER_TYPE_BASE) {
        ((BaseMenuRenderer*)MenuRenderer::getInstance())->giveBackDisplay();
    }

    // clear down all structures.
    button1 = button2 = BTNTYPE_NONE;
    setNeedsDrawing(false);
    headerPgm = NULL;
}

ButtonType BaseDialog::findActiveBtn(unsigned int currentValue) {
    ButtonType b = button2;
    if(currentValue==0) {
        b = (button1 == BTNTYPE_NONE) ? button2 : button1;
    }
    serdebugF4("findActiveBtn: ", currentValue, button1, button2);
    return b;
}

void BaseDialog::dialogRendering(unsigned int currentValue, bool userClicked) {
    if(currentValue != lastBtnVal) {
        setNeedsDrawing(true);
    }

    lastBtnVal = currentValue;

    if(userClicked) {
        // must be done before hide, which resets the encoder
        ButtonType btn = findActiveBtn(currentValue);
        serdebugF2("User clicked button: ", btn);
        hide();
        if(completedHandler) completedHandler(btn, userData);
        return;
    }

    if(needsDrawing != MENUDRAW_NO_CHANGE) {
        internalRender(currentValue);
        setNeedsDrawing(false);
    }
}

bool BaseDialog::copyButtonText(char* data, int buttonNum, int currentValue) {
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
    if((button1 == BTNTYPE_NONE || button2 == BTNTYPE_NONE) || buttonNum == currentValue) {
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
