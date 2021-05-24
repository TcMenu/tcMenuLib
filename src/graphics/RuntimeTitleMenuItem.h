/*
 * Copyright (c) 2018 https://www.thecoderscorner.com (Nutricherry LTD).
 * This product is licensed under an Apache license, see the LICENSE file in the top-level directory.
 */

/**
 * @file RuntimeTitleMenuItem.h the menu item that is presented for the title
 */

#ifndef TCMENU_RUNTIMETITLEMENUITEM_H
#define TCMENU_RUNTIMETITLEMENUITEM_H

#include <BaseRenderers.h>

namespace tcgfx {

    /**
     * forward reference of the title rendering function
     * @param item menu item for title
     * @param mode rendering mode
     * @param buffer the buffer
     * @param bufferSize the buffers size
     * @return status code
     */
    int appTitleRenderingFn(RuntimeMenuItem *item, uint8_t, RenderFnMode mode, char *buffer, int bufferSize);

    /**
     * This menu item extension class handles the title row, for the root menu. It stores a header in program memory
     * and a possible callback function for when it is actioned.
     */
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

    /**
     * the global instance of the title menu item
     */
    extern RuntimeTitleMenuItem appTitleMenuItem;

    /**
     * Sets the callback that will be triggered when the title is clicked on.
     * @param cb the title click callback.
     */
    inline void setTitlePressedCallback(MenuCallbackFn cb) {
        appTitleMenuItem.setCallback(cb);
    }
}

#endif //TCMENU_RUNTIMETITLEMENUITEM_H
