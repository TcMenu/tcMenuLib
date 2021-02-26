
#include <BaseRenderers.h>
#include "DialogRuntimeEditor.h"

void onScrollingChanged(int id) {

}

void DialogMultiPartEditor::presentAsDialog(BaseDialog *dialog, EditableMultiPartMenuItem* item) {
    dialog->setButtons(BTNTYPE_OK, BTNTYPE_NONE);
    char sz[25];
    strcpy(sz, "Edit ");
    item->copyNameToBuffer(sz, 5, sizeof sz);
    dialog->showRam(sz, false, this);
}

void DialogMultiPartEditor::dialogDismissed(ButtonType buttonType) {

}

bool DialogMultiPartEditor::dialogButtonPressed(int buttonNum) {
    return false;
}

void DialogMultiPartEditor::copyCustomButtonText(int buttonNumber, char *buffer, size_t bufferSize) {
    if(buttonNumber == 1) {
        strcpy(buffer, "Next");
    }
}

