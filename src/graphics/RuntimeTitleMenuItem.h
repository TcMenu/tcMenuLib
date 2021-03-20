/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

#ifndef TCMENU_RUNTIMETITLEMENUITEM_H
#define TCMENU_RUNTIMETITLEMENUITEM_H

#include <BaseRenderers.h>

namespace tcgfx {

    int appTitleRenderingFn(RuntimeMenuItem *item, uint8_t, RenderFnMode mode, char *buffer, int bufferSize);

    class RuntimeTitleMenuItem : public RuntimeMenuItem {
    private:
        const char *titleHeaderPgm;
        MenuCallbackFn callback;
    public:
        RuntimeTitleMenuItem(uint16_t id, MenuItem *next) : RuntimeMenuItem(MENUTYPE_TITLE_ITEM, id,
                                                                            appTitleRenderingFn, 0, 1, next) {
            titleHeaderPgm = nullptr;
            callback = nullptr;
        }

        void setTitleHeaderPgm(const char *header) {
            titleHeaderPgm = header;
        }

        const char *getTitleHeaderPgm() {
            return titleHeaderPgm;
        }

        void setCallback(MenuCallbackFn titleCb) {
            callback = titleCb;
        }

        MenuCallbackFn getCallback() {
            return callback;
        }
    };

    extern RuntimeTitleMenuItem appTitleMenuItem;

    inline void setTitlePressedCallback(MenuCallbackFn cb) {
        appTitleMenuItem.setCallback(cb);
    }
}

#endif //TCMENU_RUNTIMETITLEMENUITEM_H
