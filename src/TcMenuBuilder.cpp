//
// Created by David Cherry on 22/01/2026.
//

#include "TcMenuBuilder.h"

#include "RemoteMenuItem.h"
#include "ScrollChoiceMenuItem.h"
#include "tcMenu.h"

void MenuFlags::setOnMenuItem(MenuItem* item) const {
    item->setReadOnly((flags & INT_MENU_FLAG_READ)!=0);
    item->setSecured((flags & INT_MENU_FLAG_SECURE)!=0);
    item->setLocalOnly((flags & INT_MENU_FLAG_LOCAL)!=0);
    if ((flags & INT_MENU_FLAG_HIDE)!=0) {
        item->setVisible(false);
    }
}

void TcMenuBuilder::putAtEndOfSub(MenuItem * toAdd) const {
    toAdd->setNext(nullptr);

    if (currentSub->getChild() == nullptr) {
        serlogF3(SER_TCMENU_INFO, "New child  ", currentSub->getId(), toAdd->getId());
        currentSub->setChild(toAdd);
        return;
    }

    int loopCount = 0;
    auto item = currentSub->getChild();
    while (item && item->getNext() && loopCount < 999) {
        ++loopCount;
        item = item->getNext();
    }
    if (item) {
        serlogF3(SER_TCMENU_INFO, "Append child  ", currentSub->getId(), toAdd->getId());
        item->setNext(toAdd);
    } else {
        serlogF3(SER_ERROR, "PutAtEnd failed  ", toAdd->getId(), loopCount);
    }
}

AllMenuInfoTypes* arrayOfItems[8];
size_t itemsAllocated = 0;

AllMenuInfoTypes* TcMenuBuilder::fillInAnyInfo(menuid_t id, const char *name, int eeprom, int maxVal, MenuCallbackFn callback_fn) {
    if (itemsAllocated >= BUILDER_ALLOCATER_TOTAL) {
        while (true) {
            serlogF2(SER_ERROR, "TcMB ovflw ", itemsAllocated);
            delay(100);
        }
    }

    if (itemsAllocated % BUILDER_ALLOCATER_SIZE == 0) {
        if (itemsAllocated == 0) memset(arrayOfItems, 0, sizeof(arrayOfItems));
        auto position = itemsAllocated / BUILDER_ALLOCATER_SIZE;
        arrayOfItems[position] = new AllMenuInfoTypes[BUILDER_ALLOCATER_SIZE];
    }
    auto reserve = &arrayOfItems[itemsAllocated / BUILDER_ALLOCATER_SIZE][itemsAllocated % BUILDER_ALLOCATER_SIZE];
    itemsAllocated++;
    reserve->anyInfo.id = id;
    strncpy(reserve->anyInfo.name, name, NAME_SIZE_T);
    reserve->anyInfo.name[NAME_SIZE_T-1] = 0;
    reserve->anyInfo.eepromAddr = eeprom;
    reserve->anyInfo.callback = callback_fn;
    reserve->anyInfo.maxValue = maxVal;
    return reserve;
}

TcMenuBuilder & TcMenuBuilder::appendCustomItem(MenuItem *itemToAdd) {
    putAtEndOfSub(itemToAdd);
    return *this;
}

TcMenuBuilder& TcMenuBuilder::endSub() {
    if (parent != nullptr) {
        return *parent;
    } else {
        return *this;
    }
}

AnalogItemBuilder & AnalogItemBuilder::unit(const char *unit) {
    if (unit == nullptr) {
            info.unitName[0] = '\0';
    } else {
        strncpy(info.unitName, unit, sizeof info.unitName);
        info.unitName[sizeof info.unitName - 1] = '\0';
    }
    return *this;
}

ScrollChoiceBuilder & ScrollChoiceBuilder::fromRamChoices(const char *fixedArray, int numItems, int fixedItemSize) {
    createdItem = new ScrollChoiceMenuItem(&info, initialValue, fixedArray, fixedItemSize, numItems);
    return *this;
}

ScrollChoiceBuilder & ScrollChoiceBuilder::fromRomChoices(EepromPosition choices, int numItems, int fixedItemSize) {
    createdItem = new ScrollChoiceMenuItem(&info, initialValue, choices, fixedItemSize, numItems);
    return *this;
}

ScrollChoiceBuilder & ScrollChoiceBuilder::ofCustomRtFunction(RuntimeRenderingFn rtRenderFn, int numItems) {
    createdItem = new ScrollChoiceMenuItem(&info, rtRenderFn, initialValue, numItems, nullptr, false);
    return *this;
}

ScrollChoiceBuilder & ScrollChoiceBuilder::cachingEepromValues() {
    if (createdItem != nullptr) {
        createdItem->cacheEepromValues();
    }
    return *this;
}

TcMenuBuilder& ScrollChoiceBuilder::endItem() const {
    if (createdItem == nullptr) {
        serlogF(SER_ERROR, "ScrollChoiceBuilder::commit null!");
    } else {
        menuFlags.setOnMenuItem(createdItem);
        parentBuilder.putAtEndOfSub(createdItem);
    }
    return parentBuilder;
}

TcMenuBuilder& TcMenuBuilder::usingDynamicEEPROMStorage() {
    setEepromStorageMode(TC_STORE_ROM_DYNAMIC);
    return *this;
}

TcMenuBuilder& TcMenuBuilder::floatItem(menuid_t id, const char *name, EepromPosition eepromPosition, uint16_t decimalPlaces,
                                        MenuFlags flags, float initial, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, 0xFFFF, decimalPlaces, callbackFn);
    auto item = new FloatMenuItem(&reserve->floatInfo, nullptr,  false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    item->setFloatValue(initial);
    return *this;
}

TcMenuBuilder& TcMenuBuilder::actionItem(menuid_t id, const char *name, MenuFlags flags, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, 0xFFFF, 0, callbackFn);
    auto item = new ActionMenuItem(&reserve->anyInfo, nullptr,  false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder& TcMenuBuilder::boolItem(menuid_t id, const char *name, EepromPosition eepromPosition, BooleanNaming naming,
                                       MenuFlags flags, bool initial, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, eepromPosition, 1, callbackFn);
    reserve->booleanInfo.naming = naming;
    auto item = new BooleanMenuItem(&reserve->booleanInfo, initial, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

AnalogItemBuilder TcMenuBuilder::analogBuilder(menuid_t id, const char *name, EepromPosition eepromPosition,
    MenuFlags flags, uint16_t initialValue, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, eepromPosition, 0, callbackFn);
    auto item = new AnalogMenuItem(&reserve->analogInfo, initialValue, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return AnalogItemBuilder(*item, reserve->analogInfo, *this);
}

TcMenuBuilder & TcMenuBuilder::enumItem(menuid_t id, const char *name, EepromPosition eepromPosition,
    const char **enumEntries, uint16_t numEntries, MenuFlags flags, uint16_t value, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, eepromPosition, numEntries - 1, callbackFn);
    reserve->enumInfo.menuItems = enumEntries;
    auto item = new EnumMenuItem(&reserve->enumInfo, value, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder& TcMenuBuilder::ipAddressItem(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags, IpAddressStorage initial, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, eepromPosition, 0, callbackFn);
    auto item = new IpAddressMenuItem(&reserve->anyInfo, initial, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder & TcMenuBuilder::timeItem(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags, MultiEditWireType timeFormat,
    const TimeStorage &timeStorage, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, eepromPosition, 0, callbackFn);
    auto item = new TimeFormattedMenuItem(&reserve->anyInfo, timeStorage, timeFormat, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder & TcMenuBuilder::timeItem(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags,
    MultiEditWireType timeFormat, MenuCallbackFn callbackFn) {
    return timeItem(id, name, eepromPosition, flags, timeFormat, TimeStorage(12, 0), callbackFn);
}

TcMenuBuilder & TcMenuBuilder::dateItem(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags,
    DateStorage initial, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, eepromPosition, 0, callbackFn);
    auto item = new DateFormattedMenuItem(&reserve->anyInfo, initial, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder & TcMenuBuilder::dateItem(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags, MenuCallbackFn callbackFn) {
    return dateItem(id, name, eepromPosition, flags, DateStorage(1, 1, 2000), callbackFn);
}

ScrollChoiceBuilder TcMenuBuilder::scrollChoiceBuilder(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags, uint16_t initialValue, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, eepromPosition, 0, callbackFn);
    return ScrollChoiceBuilder(reserve->anyInfo, *this, initialValue, flags);
}

TcMenuBuilder & TcMenuBuilder::rgb32Item(menuid_t id, const char *name, EepromPosition eepromPosition,
                                         bool alphaChannel, MenuFlags flags, MenuCallbackFn callbackFn) {
    return rgb32Item(id, name, eepromPosition, alphaChannel, flags, RgbColor32(0, 0, 0), callbackFn);
}

TcMenuBuilder & TcMenuBuilder::rgb32Item(menuid_t id, const char *name, EepromPosition eepromPosition, bool alphaChannel, MenuFlags flags, const RgbColor32& initial, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, eepromPosition, 3, callbackFn);
    auto item = new Rgb32MenuItem(&reserve->anyInfo, initial, alphaChannel, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder& TcMenuBuilder::rgb32CustomRt(menuid_t id, const char *name, EepromPosition eepromPosition, bool alphaChannel,
                                 RuntimeRenderingFn renderFn, MenuFlags flags, const RgbColor32& initial, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, eepromPosition, 3, callbackFn);
    auto item = new Rgb32MenuItem(&reserve->anyInfo, renderFn, initial, alphaChannel, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder& TcMenuBuilder::largeNumberRtCustom(menuid_t id, const char *name, EepromPosition eepromPosition, const LargeFixedNumber& num,
                                                  bool allowNegative, RuntimeRenderingFn renderFn, MenuFlags flags, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, eepromPosition, 0, callbackFn);
    auto item = new EditableLargeNumberMenuItem(&reserve->anyInfo, renderFn, num, allowNegative, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder& TcMenuBuilder::largeNumberItem(menuid_t id, const char *name, EepromPosition eepromPosition, const LargeFixedNumber& num, bool allowNegative, MenuFlags flags, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, eepromPosition, 0, callbackFn);
    auto item = new EditableLargeNumberMenuItem(&reserve->anyInfo, num, allowNegative, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder & TcMenuBuilder::listItemRam(menuid_t id, const char *name, uint16_t numberOfRows, const char** arrayOfItems, MenuFlags flags, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, 0xFFFF, numberOfRows, callbackFn);
    auto item = new ListRuntimeMenuItem(&reserve->anyInfo, numberOfRows, arrayOfItems, ListRuntimeMenuItem::RAM_ARRAY, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder & TcMenuBuilder::listItemFlash(menuid_t id, const char *name, uint16_t numberOfRows, const char **arrayOfItems, MenuFlags flags, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, 0xFFFF, numberOfRows, callbackFn);
    auto item = new ListRuntimeMenuItem(&reserve->anyInfo, numberOfRows, arrayOfItems, ListRuntimeMenuItem::FLASH_ARRAY, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder & TcMenuBuilder::listItemRtCustom(menuid_t id, const char* name, uint16_t numberOfRows, RuntimeRenderingFn rtRenderFn, MenuFlags flags, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, 0xFFFF, numberOfRows, callbackFn);
    auto item = new ListRuntimeMenuItem(&reserve->anyInfo, numberOfRows, rtRenderFn, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder & TcMenuBuilder::eepromAuthenticationItem(menuid_t id, const char *name, MenuFlags flags,MenuCallbackFn onAuthChanged) {
    auto item = new EepromAuthenticationInfoMenuItem(name, onAuthChanged, id, nullptr);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder& TcMenuBuilder::remoteConnectivityMonitor(menuid_t id, const char* name, MenuFlags flags) {
    auto item = new RemoteMenuItem(name, id, nullptr);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder& TcMenuBuilder::ipAddressItem(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags, MenuCallbackFn callbackFn) {
    return ipAddressItem(id, name, eepromPosition, flags, IpAddressStorage(127, 0, 0, 1), callbackFn);
}

TcMenuBuilder& TcMenuBuilder::ipAddressCustomRt(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags,
                                 RuntimeRenderingFn renderFn, IpAddressStorage ipInitial, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, eepromPosition, 3, callbackFn);
    auto item = new IpAddressMenuItem(&reserve->anyInfo, renderFn, ipInitial, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder& TcMenuBuilder::dateItemCustomRt(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags, DateStorage initial,
                    RuntimeRenderingFn renderFn, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, eepromPosition, 3, callbackFn);
    auto item = new DateFormattedMenuItem(&reserve->anyInfo, renderFn, initial, id, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder& TcMenuBuilder::timeItemCustomRt(menuid_t id, const char *name, EepromPosition eepromPosition, const TimeStorage& timeStorage,
                                RuntimeRenderingFn renderFn, MenuFlags flags, MultiEditWireType timeFormat, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, eepromPosition, 3, callbackFn);
    auto item = new TimeFormattedMenuItem(&reserve->anyInfo, renderFn, timeStorage, timeFormat, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder& TcMenuBuilder::textItem(menuid_t id, const char *name, EepromPosition eepromPosition, uint16_t textLength,
                        MenuFlags flags, const char *initial, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, eepromPosition, textLength, callbackFn);
    auto item = new TextMenuItem(&reserve->anyInfo, initial, textLength, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder& TcMenuBuilder::textCustomRt(menuid_t id, const char *name, EepromPosition eepromPosition,
                                           uint16_t textLength, RuntimeRenderingFn renderFn, MenuFlags flags,
                                           const char *initial, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, eepromPosition, textLength, callbackFn);
    auto item = new TextMenuItem(&reserve->anyInfo, renderFn, initial, textLength, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    return *this;
}

TcMenuBuilder TcMenuBuilder::subMenu(menuid_t id, const char *name, MenuFlags flags, MenuCallbackFn callbackFn) {
    auto* reserve = fillInAnyInfo(id, name, 0xFFFF, 0, callbackFn);
    auto item = new SubMenuItem(&reserve->subInfo, nullptr, nullptr, false);
    flags.setOnMenuItem(item);
    putAtEndOfSub(item);
    auto back = new BackMenuItem(&reserve->subInfo, nullptr, false);
    auto newBuilder = TcMenuBuilder(item, this);
    newBuilder.putAtEndOfSub(back);
    return newBuilder;
}
