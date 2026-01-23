//
// Created by David Cherry on 22/01/2026.
//

#include "TcMenuBuilder.h"

#include "tcMenu.h"

void MenuFlags::setOnMenuItem(MenuItem &item) const {
    item.setReadOnly((flags & INT_MENU_FLAG_READ)!=0);
    item.setSecured((flags & INT_MENU_FLAG_SECURE)!=0);
    item.setLocalOnly((flags & INT_MENU_FLAG_LOCAL)!=0);
    if ((flags & INT_MENU_FLAG_HIDE)!=0) {
        item.setVisible(false);
    }
}

BtreeList<menuid_t, AnyInfoReserve> infoAllocator;

void TcMenuBuilder::putAtEndOfMenu(MenuItem * toAdd) {
    int loopcount = 0;
    auto item = &localRoot;
    while (item && item->getNext() && loopcount < 100) {
        ++loopcount;
        item = item->getNext();
    }
    if (item) {
        item->setNext(toAdd);
    }
}

AnyInfoReserve* TcMenuBuilder::fillInAnyInfo(menuid_t id, const char *name, int eeprom, int maxVal, MenuCallbackFn callback_fn) {
    AnyInfoReserve reserve;
    reserve.getInfo()->anyInfo.id = id;
    strncpy(reserve.getInfo()->anyInfo.name, name, NAME_SIZE_T);
    reserve.getInfo()->anyInfo.eepromAddr = eepromPosition;
    reserve.getInfo()->anyInfo.callback = callbackFn;
    reserve.getInfo()->anyInfo.maxValue = maxVal;
    infoAllocator.add(reserve);
    return infoAllocator.getByKey(id);
}

TcMenuBuilder & TcMenuBuilder::actionItem(menuid_t id, const char *name, MenuFlags flags, MenuCallbackFn callbackFn) {
    AnyInfoReserve* reserve = fillInAnyInfo(id, name, 0xFFFF, 0, callbackFn);
    auto item = new FloatMenuItem(&reserve.getInfo()->floatInfo, nullptr,  false);
    flags.setOnMenuItem(*item);
    putAtEndOfMenu(item);
    return *this;
}

TcMenuBuilder& TcMenuBuilder::floatItem(menuid_t id, const char *name, EepromPosition eepromPosition, uint16_t decimalPlaces, MenuFlags flags, MenuCallbackFn callbackFn) {
    AnyInfoReserve* reserve = fillInAnyInfo(id, name, 0xFFFF, decimalPlaces, );
    auto item = new FloatMenuItem(&reserve.getInfo()->floatInfo, nullptr,  false);
    flags.setOnMenuItem(*item);
    putAtEndOfMenu(item);
    return *this;
}

MenuFlags flags = MenuFlags().localOnly();

MenuItem item(MENUTYPE_FLOAT_VALUE, nullptr, nullptr, false);
TcMenuBuilder builder(item);

void f() {
    builder.floatItem(1, "test", 0, 2, MenuFlags().localOnly())
        .floatItem(1, "test", 0, 2, NoMenuFlags);
}