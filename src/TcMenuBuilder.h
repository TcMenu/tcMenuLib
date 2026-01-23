
#ifndef TCLIBRARYDEV_TCMENUBUILDER_H
#define TCLIBRARYDEV_TCMENUBUILDER_H
#include "EepromAbstraction.h"
#include "MenuItems.h"


class MenuManager;

#define INT_MENU_FLAG_READ 0x01
#define INT_MENU_FLAG_HIDE 0x02
#define INT_MENU_FLAG_LOCAL 0x04
#define INT_MENU_FLAG_SECURE 0x08

class MenuFlags {
    uint8_t flags;
public:
    MenuFlags() = default;
    ~MenuFlags() = default;

    MenuFlags& readOnly() { flags |= INT_MENU_FLAG_READ; return *this; }
    MenuFlags& hide() { flags |= INT_MENU_FLAG_HIDE; return *this; }
    MenuFlags& localOnly() { flags |= INT_MENU_FLAG_LOCAL; return *this; }
    MenuFlags& securePin() { flags |= INT_MENU_FLAG_SECURE; return *this; }

    void setOnMenuItem(MenuItem& item) const;
};

const MenuFlags NoMenuFlags;

class AnyInfoReserve {
    AllMenuInfoTypes info;
public:
    AnyInfoReserve() : info() {}
    AnyInfoReserve(const AnyInfoReserve&) = default;
    AnyInfoReserve& operator=(const AnyInfoReserve&) = default;
    ~AnyInfoReserve() = default;
    menuid_t getKey() const { return info.anyInfo.id; }
    bool isInUse() const { return info.anyInfo.id > 0;}
    AllMenuInfoTypes* getInfo() { return &info; }
};

class TcMenuBuilder {
private:
    MenuItem& localRoot;
public:
    TcMenuBuilder(MenuItem& root) : localRoot(root) {}
    ~TcMenuBuilder() = default;

    void putAtEndOfMenu(MenuItem * item);
    AnyInfoReserve* fillInAnyInfo(menuid_t id, const char *name, int eeprom, int maxVal, MenuCallbackFn callback_fn);

    TcMenuBuilder& floatItem(menuid_t id, const char *name, EepromPosition eepromPosition, uint16_t decimalPlaces, MenuFlags flags, float value = 0.0F, MenuCallbackFn callbackFn = nullptr);
    TcMenuBuilder& actionItem(menuid_t id, const char *name, MenuFlags flags, MenuCallbackFn callbackFn = nullptr);
    TcMenuBuilder& boolItem(menuid_t id, const char *name, EepromPosition eepromPosition, BooleanNaming naming, MenuFlags flags, bool initial = false, MenuCallbackFn callbackFn = nullptr);
    void build(MenuManager& mgr);
};


#endif //TCLIBRARYDEV_TCMENUBUILDER_H