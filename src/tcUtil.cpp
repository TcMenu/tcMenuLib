/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Dave Cherry).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#include "MenuItems.h"
#include "tcMenu.h"
#include "tcUtil.h"
#include "BaseDialog.h"
#include "tcMenuVersion.h"

#if defined __AVR__ || defined ESP_H
char szGlobalBuffer[16];
#endif

uint8_t safeProgCpy(char* dst, const char* pgmSrc, uint8_t size) {
    uint8_t pos = 0;
    char nm = get_info_char(pgmSrc);
    while (nm && pos < (size - 1)) {
		dst[pos] = nm;
		++pgmSrc;
        ++pos;
        nm = get_info_char(pgmSrc);
    }
    dst[pos] = 0;
    return pos;
}

void showVersionDialog(const ConnectorLocalInfo *localInfo) {
    static const ConnectorLocalInfo *localInfoStatic = localInfo;
    withMenuDialogIfAvailable([](MenuBasedDialog *dialog) {
        dialog->setButtons(BTNTYPE_NONE, BTNTYPE_CLOSE);
        dialog->show(localInfoStatic->name, false, nullptr);
        char sz[10];
        tccore::copyTcMenuVersion(sz, sizeof(sz));
        dialog->copyIntoBuffer(sz);
    });

}
