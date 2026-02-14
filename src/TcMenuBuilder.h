
#ifndef TCLIBRARYDEV_TCMENUBUILDER_H
#define TCLIBRARYDEV_TCMENUBUILDER_H
#include "EepromAbstraction.h"
#include "MenuItems.h"
#include "RuntimeMenuItem.h"

class TcMenuBuilder;
struct RgbColor32;
class MenuManager;
class ScrollChoiceMenuItem;

#define INT_MENU_FLAG_READ 0x01
#define INT_MENU_FLAG_HIDE 0x02
#define INT_MENU_FLAG_LOCAL 0x04
#define INT_MENU_FLAG_SECURE 0x08

// This definition is used when dynamic eeprom storage is enabled to indicate save to rom.
#define ROM_SAVE 0x0020
// This definition is used when dynamic eeprom storage is enabled to indicate do not save to eeprom.
#define DONT_SAVE 0xFFFF

/**
 * This provides the extra flags for a menu item, such as its visibility and read only status. If you don't want to
 * provide any of these flags, you can use the NoMenuFlags constant.
 *
 * Examples:
 * @code
 * builder.actionItem(myId, "Action", ROM_SAVE, NoMenuFlags)
 * builder.actionItem(myId, "Action", ROM_SAVE, MenuFlags().readOnly().hide())
 * @endcode
 */
class MenuFlags {
    uint8_t flags = 0;
public:
    MenuFlags() = default;
    MenuFlags(const MenuFlags&) = default;
    MenuFlags& operator=(const MenuFlags&) = default;
    ~MenuFlags() = default;

    MenuFlags& readOnly() { flags |= INT_MENU_FLAG_READ; return *this; }
    MenuFlags& hide() { flags |= INT_MENU_FLAG_HIDE; return *this; }
    MenuFlags& localOnly() { flags |= INT_MENU_FLAG_LOCAL; return *this; }
    MenuFlags& securePin() { flags |= INT_MENU_FLAG_SECURE; return *this; }

    void setOnMenuItem(MenuItem* item) const;
};

/**
 * Used when you have no flags such as readOnly, hide, or localOnly to set on the item. If you need to set any flags
 * use MenuFlags().readOnly() for example.
 */
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

/**
 * @class AnalogItemBuilder
 * @brief A builder class for configuring and adding an analog menu item.
 *
 * This class facilitates the creation and customization of an analog menu item. It offers a fluent API to define
 * attributes such as offset, divisor, step size, maximum value, and unit for the analog menu item before finalizing
 * its addition to the parent menu structure.
 *
 * The builder modifies the `AnalogMenuInfo` properties and the associated `AnalogMenuItem` object
 * to define the behavior and display characteristics of the item within the menu. The configured
 * item is integrated into the menu hierarchy via the `commit` operation, returning control to the
 * parent `TcMenuBuilder`.
 *
 * @see https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/analog-menu-item/
 */
class AnalogItemBuilder {
private:
    AnalogMenuItem& item;
    AnalogMenuInfo& info;
    TcMenuBuilder& parentBuilder;
public:
    AnalogItemBuilder(AnalogMenuItem& item, AnalogMenuInfo& info,TcMenuBuilder& parentBuilder) : item(item), info(info), parentBuilder(parentBuilder) {}
    AnalogItemBuilder(const AnalogItemBuilder &) = default;

    /**
     * Set the offset of the analog item, the offset is added to the analog value before display. So if you set this
     * to be a negative number, the displayed value will be negative until the raw value hits offset.
     * @param offs the offset value to apply to the analog item
     * @return a reference to the current instance of AnalogItemBuilder for chaining additional configurations
     */
    AnalogItemBuilder& offset(int16_t offs) { info.offset = offs; return *this; }
    /**
     * Set the divisor of the analog item, I.E this is the fixed point divisor for the analog item. If you select for
     * example 2, this will divide the analog value by 2 before displaying it in halves.
     * @param divisor the divisor value
     * @return a reference to the current instance of AnalogItemBuilder for chaining additional configurations
     */
    AnalogItemBuilder& divisor(uint16_t divisor) { info.divisor = divisor == 0 ? 1 : divisor; return *this; }
    /**
     * An optional step size for the analog item, allowing for finer control over the displayed values.
     * @param step the step size value
     * @return a reference to the current instance of AnalogItemBuilder for chaining additional configurations
     */
    AnalogItemBuilder& step(int step) { item.setStep(step); return *this; }
    /**
     * The maximum value that can be represented, this is the RAW value before divisors or offsets are applied.
     * @param maxVal the maximum value that can be represented
     * @return a reference to the current instance of AnalogItemBuilder for chaining additional configurations
     */
    AnalogItemBuilder& maxValue(int maxVal) { info.maxValue = maxVal; return *this; }

    /**
     * Set the unit name for the analog item, this is displayed after the value.
     * @param unit the unit name to display
     * @return a reference to the current instance of AnalogItemBuilder for chaining additional configurations
     */
    AnalogItemBuilder& unit(const char* unit);

    /**
     * @brief Finalizes the configuration of the analog menu item and returns control to the parent menu builder.
     *
     * This method completes the building process of the analog menu item by finalizing its configuration to the
     * associated `AnalogMenuItem` and integrates it into the menu structure. It then returns control to the parent
     * `TcMenuBuilder` to allow for further menu items.
     *
     * @return A reference to the parent `TcMenuBuilder` instance.
     */
    TcMenuBuilder& endItem() const {return parentBuilder;}
};

class ScrollChoiceBuilder {
private:
    AnyMenuInfo& info;
    TcMenuBuilder& parentBuilder;
    uint16_t initialValue;
    MenuFlags menuFlags;
    ScrollChoiceMenuItem* createdItem = nullptr;
public:
    ScrollChoiceBuilder(AnyMenuInfo& info,TcMenuBuilder& parentBuilder, uint16_t initialValue, MenuFlags flags) : info(info), parentBuilder(parentBuilder), initialValue(initialValue), menuFlags(flags) {}
    ScrollChoiceBuilder(const ScrollChoiceBuilder &) = default;

    /**
     * @brief Configures the scroll choice menu item using a fixed array of choices stored in RAM.
     *
     * This method initializes and configures a `ScrollChoiceMenuItem` with a fixed array of choices
     * located in RAM.
     *
     * @note If you use this method, you must set an EEPROM in `menuMgr` otherwise the item will not function correctly.
     *
     * The array is of fixed length, meaning that each item takes a fixed size. Example with
     * fixedLen=8, numEntries=3:
     *
     * @code
     * choiceString = "Option A Option B Option C "
     *                 ^^^^^^^^ ^^^^^^^^ ^^^^^^^^
     *                 Entry 0  Entry 1  Entry 2
     *                 (index)  (index)  (index)
     *                    0        8        16
     * @endcode
     *
     * Each entry occupies exactly fixedLen characters. If the text is shorter than fixedLen,
     * it should be padded with spaces or zero terminated. Entry N starts at position (N * fixedLen).     *
     * List Docs - https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/list-menu-item/
     * ScrollChoice Docs - https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/scrollchoice-menu-item/
     *
     * @param fixedArray A pointer to a fixed array of choices stored in RAM.
     * @param numItems The total number of items available in the fixed array.
     * @param fixedItemSize The size (in bytes) of each individual choice in the array.
     * @return A reference to the current `ScrollChoiceBuilder` instance for method chaining.
     */
    ScrollChoiceBuilder& fromRamChoices(const char* fixedArray, int numItems, int fixedItemSize);

    /**
     * @brief Configures the scroll choice menu item using a fixed array of choices stored in EEPROM.
     *
     * This method initializes and configures a `ScrollChoiceMenuItem` with a fixed array of choices
     * located in EEPROM.
     *
     * If you use this method, you must set an EEPROM in `menuMgr` otherwise the item will not function correctly.
     *
     * The array is of fixed length, meaning that each item takes a fixed size. Example with
     * fixedLen=8, numEntries=3:
     *
     * @code
     * choiceString = "Option A Option B Option C "
     *                 ^^^^^^^^ ^^^^^^^^ ^^^^^^^^
     *                 Entry 0  Entry 1  Entry 2
     *                 (index)  (index)  (index)
     *                    0        8        16
     * @endcode
     *
     * Each entry occupies exactly fixedLen characters. If the text is shorter than fixedLen,
     * it should be padded with spaces or zero terminated. Entry N starts at position (N * fixedLen).
     *
     * List Docs - https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/list-menu-item/
     * ScrollChoice Docs - https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/scrollchoice-menu-item/
     *
     * @param eepromPosition offset into the eeprom of the fixed array
     * @param numItems The total number of items available in the fixed array.
     * @param fixedItemSize The size (in bytes) of each individual choice in the array.
     * @return A reference to the current `ScrollChoiceBuilder` instance for method chaining.
     */
    ScrollChoiceBuilder& fromRomChoices(EepromPosition eepromPosition, int numItems, int fixedItemSize);

    ScrollChoiceBuilder& ofCustomRtFunction(RuntimeRenderingFn rtRenderFn, int numItems);

    /**
     * Reading from EEPROM is slow, you can optionally cache the values in RAM once loaded,
     * improving read performance.
     * @return A reference to the current `ScrollChoiceBuilder` instance for method chaining.
     */
    ScrollChoiceBuilder& cachingEepromValues();

    /**
     * @brief Finalizes the configuration of the scroll choice menu item and returns control to the parent menu builder.
     *
     * This method completes the building process of the menu item by finalizing its configuration to the
     * associated `ScrollChoiceMenuItem` and integrates it into the menu structure. It then returns control to
     * the parent `TcMenuBuilder` to allow for further menu items.
     *
     * @return A reference to the parent `TcMenuBuilder` instance.
     */
    TcMenuBuilder& endItem() const;
};

/**
 * @class TcMenuBuilder
 * @brief A builder class to create and configure a hierarchical menu structure.
 *
 * This class provides a fluent API to define and organize menu items such as actions, text items,
 * numeric items, submenus, and other specialized items. It maintains a tree-like structure of menu components,
 * where each menu may have submenus or other related items.
 *
 * The class ensures the correct organization of menu items within containers using a parent-child hierarchy.
 * It supports a wide range of menu item types, allowing for customization of behavior, storage, and rendering.
 *
 * @note Because menu creation happens at runtime, this builder cannot use PROGMEM storage for menu data,
 * resulting in increased RAM memory usage compared to compile-time menu definition approaches. All menu
 * structures, strings, and metadata must be allocated in RAM. For most menus, even on large AVR boards such as MEGA2560
 * the difference is negligible. However, you can still use the legacy static way if your case requires it. 
 * 
 */
class TcMenuBuilder {
public:
    explicit TcMenuBuilder(SubMenuItem* root, TcMenuBuilder* parent = nullptr) : currentSub(root), parent(parent) {}
    TcMenuBuilder(const TcMenuBuilder &) = default;
    TcMenuBuilder& operator=(const TcMenuBuilder &) = default;
    ~TcMenuBuilder() = default;

    /**
     * This is now the recommended way to store menu structures in EEPROM. It allows for dynamic
     * menu configurations without the need for compile-time menu definitions, which can be
     * beneficial for applications with changing menu requirements or frequent updates. If you use this mode to enable
     * eeprom storage for an item simply use "ROM_SAVE" or "DONT_SAVE" in the eeprom position.
     */
    TcMenuBuilder& usingDynamicEEPROMStorage();
    
    /**
     * @brief Adds a float menu item to the current menu structure.
     *
     * Float items in TcMenu are read only, for editable numeric items see "analogBuilder" and "largeNumberItem".
     *
     * This method configures and adds a floating-point menu item with specified properties, such as
     * the number of decimal places and an initial value. The float item is integrated within the
     * parent menu hierarchy during the build process. Additional behavior and display attributes
     * can be controlled using the provided flags and callback function.
     *
     * @see https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/float-menu-item/
     *
     * @param id The unique identifier for the menu item.
     * @param name The display name of the menu item.
     * @param eepromPosition for dynamic set to ROM_SAVE or DONT_SAVE, for legacy mode use an eeprom address.
     * @param decimalPlaces The number of decimal places to display for the float value.
     * @param flags Flags to control specific display and behavior properties of the menu item.
     * @param initial The initial float value of the menu item.
     * @param callbackFn A callback function that is invoked when the menu item is interacted with.
     * @return Reference to the current instance of TcMenuBuilder to allow chaining additional builders.
     */
    TcMenuBuilder& floatItem(menuid_t id, const char *name, EepromPosition eepromPosition, uint16_t decimalPlaces, MenuFlags flags, float initial = 0.0F, MenuCallbackFn callbackFn = nullptr);

    /**
     * @brief Adds an action menu item to the current menu structure.
     *
     * Action items are menu items that trigger a callback function when selected. They are typically used for
     * executing specific actions or commands within the menu system.
     *
     * @see https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/action-menu-item/
     *
     * @param id The unique identifier for the menu item.
     * @param name The display name of the menu item.
     * @param flags Flags to control specific display and behavior properties of the menu item.
     * @param callbackFn A callback function that is invoked when the menu item is interacted with.
     * @return Reference to the current instance of TcMenuBuilder to allow chaining additional builders.
     */
    TcMenuBuilder& actionItem(menuid_t id, const char *name, MenuFlags flags, MenuCallbackFn callbackFn = nullptr);

    /**
     * @brief Creates and configures a boolean menu item to be added to the menu structure.
     *
     * This method allows the user to define the attributes of a boolean menu item, including its identifier, display name,
     * EEPROM storage position, naming convention, flags, initial state, and callback function. The resulting menu item is
     * integrated into the menu system and linked to its parent container.
     *
     * @see https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/boolean-menu-item/
     *
     * @param id The unique identifier for the menu item.
     * @param name The display name of the menu item.
     * @param eepromPosition for dynamic set to ROM_SAVE or DONT_SAVE, for legacy mode use an eeprom address.
     * @param naming The naming convention to use for boolean states (e.g., ON/OFF, YES/NO).
     * @param flags The menu flags that define item properties and behaviors.
     * @param initial The initial state of the boolean item (true or false).
     * @param callbackFn The function to be called when the item's value changes.
     * @return A reference to the `TcMenuBuilder`, allowing for method chaining.
     */
    TcMenuBuilder& boolItem(menuid_t id, const char *name, EepromPosition eepromPosition, BooleanNaming naming, MenuFlags flags, bool initial = false, MenuCallbackFn callbackFn = nullptr);

    /**
     * @brief Creates and configures an analog menu item as part of the menu structure.
     *
     * This method initializes an analog menu item with provided attributes such as ID, name, EEPROM position,
     * flags, initial value, and a callback function. It adds the item to the current menu structure and returns
     * an AnalogItemBuilder for further configuration. The pattern for using this is:
     *
     * @code
     *      builder.analogBuilder(myId, "Analog1", ROM_SAVE, NoMenuFlags)
     *              .offset(10)
     *              .divisor(100)
     *              .step(1)
     *              .maxValue(100)
     *              .unit("V")
     *              .endItem() // <<== return back to building menu items
     *          boolItem(....)
     * @endcode
     *
     *  @see https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/analog-menu-item/
     *
     *
     * @param id              The unique identifier for the analog menu item.
     * @param name            The display name for the menu item.
     * @param eepromPosition for dynamic set to ROM_SAVE or DONT_SAVE, for legacy mode use an eeprom address.
     * @param flags           Configuration flags defining the item's behavior and properties.
     * @param initialValue    The initial value of the analog item.
     * @param callbackFn      The callback function invoked when the menu item value changes.
     * @return AnalogItemBuilder to configure the analog item, call "endItem()" once done to finalize and return to menu building.
     */
    AnalogItemBuilder analogBuilder(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags, uint16_t initialValue = 0, MenuCallbackFn callbackFn = nullptr);

    /**
     * @brief Adds an enumerated menu item to the menu structure.
     *
     * This method allows the creation of an enumerated menu item, where the user can select one value from
     * a predefined list of options. The options are represented as an array of string identifiers, providing
     * a set number of choices for the menu item.
     *
     * @see https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/enum-menu-item/
     *
     * In the example we first create an array globally (IE outside any function), then we use that array later to build
     * the menu item:
     *
     * @code
     * // 1. globally define a array of strings
     * const char enumArray[] = { "Hello", "World" };
     * // 2. use the array to create an item
     * tcMenuBuilder.enumItem(myId, "Greeting", myPositionInRom, enumArray, 2, 0, nullptr);
     * @endcode
     *
     * @param id The unique identifier for the menu item.
     * @param name The text label of the menu item, displayed in the menu interface.
     * @param eepromPosition for dynamic set to ROM_SAVE or DONT_SAVE, for legacy mode use an eeprom address.
     * @param enumEntries An array of strings representing the enumeration choices for the menu item.
     * @param numEntries The total number of entries in the enumeration array.
     * @param flags The configuration flags for the menu item, specifying additional behaviors or properties.
     * @param value The default or initial selected value of the enumerated menu item, represented as an index.
     * @param callbackFn The function to call when the menu item is selected or its value changes.
     * @return A reference to the current instance of TcMenuBuilder for chaining additional menu item declarations*/
    TcMenuBuilder& enumItem(menuid_t id, const char *name, EepromPosition eepromPosition, const char **enumEntries,
                            uint16_t numEntries, MenuFlags flags, uint16_t value = 0,
                            MenuCallbackFn callbackFn = nullptr);

    /**
     * @brief Creates a submenu item with the specified attributes and integrates it into the menu hierarchy.
     *
     * This method allows the creation of a submenu within the current menu structure by providing its unique ID, name, flags,
     * and callback function. A back menu item will also be added, this becomes the title for the submenu when it is on
     * display, and so it is the first item in the submenu.
     *
     * Once you call this method, you're in a new stacked MenuBuilder that is building for that submenu. To exit
     * this submenu you call `endSub` which takes you back to the previous level.
     *
     * @param id The unique identifier for the submenu item.
     * @param name The display name of the submenu item.
     * @param flags Flags that specify the attributes and behavior settings for the submenu item.
     * @param callbackFn A function pointer for the callback executed when the submenu item is activated.
     * @return A new instance of `TcMenuBuilder` that represents the new submenu item, allowing for further configuration or chaining operations.
     */
    TcMenuBuilder subMenu(menuid_t id, const char *name, MenuFlags flags, MenuCallbackFn callbackFn = nullptr);

    /**
     * @brief Adds a text menu item to the menu structure.
     *
     * This method creates and configures a text menu item with the specified parameters. The text item is used
     * to store and edit textual data in the menu. The method allocates the necessary resources and integrates
     * the created item with the menu hierarchy.
     *
     * https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/editabletext-menu-item/
     *
     * @param id The unique identifier of the menu item.
     * @param name The display name of the menu item.
     * @param eepromPosition for dynamic set to ROM_SAVE or DONT_SAVE, for legacy mode use an eeprom address.
     * @param textLength The maximum length of the text that the item can hold.
     * @param flags The configuration flags that define the item's behavior and state.
     * @param initial The initial text value assigned to the menu item.
     * @param callbackFn The callback function triggered when the item's value changes.
     * @return A reference to the current instance of `TcMenuBuilder` to enable method chaining.
     */
    TcMenuBuilder& textItem(menuid_t id, const char *name, EepromPosition eepromPosition, uint16_t textLength,
                            MenuFlags flags, const char *initial = "", MenuCallbackFn callbackFn = nullptr);

    /**
     * Advanced build option for override of the regular text component for advanced cases, for example editing values that
     * need customization such as editing hex values for example.
     * @param id the ID of the item
     * @param name the name of the item
     * @param eepromPosition for dynamic set to ROM_SAVE or DONT_SAVE, for legacy mode use an eeprom address.
     * @param textLength The length of the text to be edited.
     * @param renderFn The callback function that will customize the control. Consult documentation for details.
     * @param flags The configuration flags that define the item's behavior and state.
     * @param initial the initial value, optional.
     * @param callbackFn The callback function triggered when the item's value changes.
     * @return
     */
    TcMenuBuilder& textCustomRt(menuid_t id, const char *name, EepromPosition eepromPosition, uint16_t textLength,
                                RuntimeRenderingFn renderFn, MenuFlags flags, const char* initial = "", MenuCallbackFn callbackFn = nullptr);

    /**
     * Adds an IP address menu item to the menu structure. This item allows the user
     * to interact with and configure an IP address.
     *
     * @param id The unique identifier for this menu item.
     * @param name The display name for this menu item.
     * @param eepromPosition for dynamic set to ROM_SAVE or DONT_SAVE, for legacy mode use an eeprom address.
     * @param flags The configuration flags that define the item's behavior and state.
     * @param callbackFn An optional callback function triggered when the menu item is changed.
     * @return Reference to the TcMenuBuilder instance to allow for method chaining.
     */
    TcMenuBuilder& ipAddressItem(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags, MenuCallbackFn callbackFn = nullptr);

    /**
     * Adds an IP Address type menu item to the menu structure being built. The IP Address menu item allows users
     * to configure or display an IP address directly within the menu.
     *
     * @param id The unique identifier for the menu item.
     * @param name The display name for the menu item.
     * @param eepromPosition The EEPROM storage position for persisting the value, or -1 if not stored in EEPROM.
     * @param flags The flags specifying visibility, read-only status, and other properties of the menu item.
     * @param initial The initial value for the IP address storage.
     * @param callbackFn The callback function invoked when the menu item is selected or updated.
     * @return Reference to the current instance of TcMenuBuilder to allow method chaining.
     */
    TcMenuBuilder& ipAddressItem(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags,
                                 IpAddressStorage ipInitial, MenuCallbackFn callbackFn = nullptr);

    /**
     * Advanced construction/build option. Adds a custom IP address menu item to the menu using the provided parameters.
     * This method allows customization of properties such as the menu ID, display name, EEPROM storage position, flags,
     * initial IP address, and an optional
     * callback function.
     *
     * @param id The unique identifier for the menu item.
     * @param name The display name of the menu item.
     * @param eepromPosition The EEPROM storage position for the value of this item.
     * @param flags Additional menu flags controlling visibility, read-only status, etc.
     * @param renderFn The callback function that will customize the control. Consult documentation for details.
     * @param ipInitial The initial value for the IP address to be displayed or stored.
     * @param callbackFn (Optional) A callback function invoked when the menu item is interacted with. Defaults to nullptr if not provided.
     * @return A reference to the TcMenuBuilder for further modification or chaining of method calls.
     */
    TcMenuBuilder& ipAddressCustomRt(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags,
                                     RuntimeRenderingFn renderFn, IpAddressStorage ipInitial, MenuCallbackFn callbackFn = nullptr);

    TcMenuBuilder& timeItem(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags, MultiEditWireType timeFormat,
                            const TimeStorage& timeStorage, MenuCallbackFn callbackFn = nullptr);

    TcMenuBuilder& timeItem(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags, MultiEditWireType timeFormat,
                            MenuCallbackFn callbackFn = nullptr);

    TcMenuBuilder& timeItemCustomRt(menuid_t id, const char *name, EepromPosition eepromPosition, const TimeStorage& timeStorage,
                                    RuntimeRenderingFn renderFn, MenuFlags flags, MultiEditWireType timeFormat, MenuCallbackFn callbackFn = nullptr);

    TcMenuBuilder& dateItem(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags, DateStorage initial,
                            MenuCallbackFn callbackFn = nullptr);

    TcMenuBuilder& dateItem(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags, MenuCallbackFn callbackFn = nullptr);

    TcMenuBuilder& dateItemCustomRt(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags, DateStorage initial,
                        RuntimeRenderingFn renderFn, MenuCallbackFn callbackFn = nullptr);

    /**
     * @brief Creates and preconfigures a ScrollChoiceBuilder to define a scrollable choice menu item.
     *
     * This method facilitates the setup of a scrollable choice menu item that allows the user to select
     * a value from a list of predefined choices. The ScrollChoiceBuilder returned by this method enables
     * further customization before the item is finalized and integrated into the menu structure. When you've configured
     * it make sure you call `endItem()` to finalize the menu item and add it to the menu structure.
     *
     * @code
     *  builder.scrollChoiceBuilder(myId, "Choice Menu", myRomLocation, NoMenuFlags)
     *      .fromRomChoices(romArrayLocation, numItems, fixedItemSize);
     *      .cachingEepromValues()
     *      .endItem(); <<== return back to building menu items
     *    .boolItem(...)
     * @endcode
     *
     * @param id The unique identifier for the menu item.
     * @param name The display name for the menu item.
     * @param eepromPosition for dynamic set to ROM_SAVE or DONT_SAVE, for legacy mode use an eeprom address.
     * @param flags Flag options defining the item's behavior and characteristics.
     * @param initial The index of the initial choice to display (default is 0).
     * @param callbackFn The callback function to be triggered on value changes (default is nullptr).
     * @return ScrollChoiceBuilder to configure the item, call "commit()" once done to finalize and return to menu building.
     */
    ScrollChoiceBuilder scrollChoiceBuilder(menuid_t id, const char *name, EepromPosition eepromPosition, MenuFlags flags, uint16_t initial = 0, MenuCallbackFn callbackFn = nullptr);

    /**
     * @brief Configures and adds a 32-bit RGB menu item to the menu structure.
     *
     * This method enables the creation of a menu item that represents a 32-bit RGB color value. The color can optionally
     * include an alpha channel for transparency. The item properties, such as identifier, display name, EEPROM storage position,
     * and associated flags, are defined during configuration. The color item's state can be modified or retrieved via callback functions.
     *
     * @param id The unique menu identifier for this item.
     * @param name The display name of the menu item.
     * @param eepromPosition for dynamic set to ROM_SAVE or DONT_SAVE, for legacy mode use an eeprom address.
     * @param alphaChannel Indicates if an alpha channel (transparency) is included in the RGB color representation.
     * @param flags Custom attributes or behaviors associated with the menu item, defined using the MenuFlags type.
     * @param callbackFn A function callback invoked during menu value changes or actions.
     * @return A reference to the current instance of the `TcMenuBuilder` for further configuration.
     */
    TcMenuBuilder& rgb32Item(menuid_t id, const char *name, EepromPosition eepromPosition, bool alphaChannel,
                             MenuFlags flags, MenuCallbackFn callbackFn = nullptr);

    /**
     * @brief Adds an RGB32 menu item to the current menu being built.
     *
     * This method allows for the creation and integration of a 32-bit RGB color menu item into
     * the menu hierarchy. The item supports optional alpha channel functionality and can be
     * initialized with a provided starting color value. Configurable flags and a callback function
     * define the behavior and interactions of the item. The method uses the fluent interface pattern,
     * returning a reference to the current `TcMenuBuilder` instance for chaining further menu additions.
     *
     * @param id The unique identifier for the RGB32 menu item.
     * @param name The display name of the menu item.
     * @param eepromPosition for dynamic set to ROM_SAVE or DONT_SAVE, for legacy mode use an eeprom address.
     * @param alphaChannel Boolean flag indicating whether the alpha channel is supported.
     * @param flags Additional configuration flags for the menu item.
     * @param initial The initial color value of type `RgbColor32` for the menu item.
     * @param callbackFn A function pointer for the menu item callback, invoked on user interaction.
     * @return A reference to the current `TcMenuBuilder` instance to allow for method chaining.
     */
    TcMenuBuilder& rgb32Item(menuid_t id, const char *name, EepromPosition eepromPosition, bool alphaChannel,
                             MenuFlags flags, const RgbColor32& initial, MenuCallbackFn callbackFn = nullptr);

    /**
     * Advanced construction/build case for RGB items where you need to override the menu in a custom way. This is
     * normally used when you want to customize the rendering or behavior of the RGB menu item beyond the standard options.
     *
     * @param id The unique identifier for the RGB32 menu item.
     * @param name The display name of the menu item.
     * @param eepromPosition for dynamic set to ROM_SAVE or DONT_SAVE, for legacy mode use an eeprom address.
     * @param alphaChannel Boolean flag indicating whether the alpha channel is supported.
     * @param renderFn The custom rendering function for the RGB menu item. Consult the documentation
     * @param flags Additional configuration flags for the menu item.
     * @param initial The initial color value of type `RgbColor32` for the menu item.
     * @param callbackFn A function pointer for the menu item callback, invoked on user interaction.
     * @return A reference to the current `TcMenuBuilder` instance to allow for method chaining.
     */
    TcMenuBuilder& rgb32CustomRt(menuid_t id, const char *name, EepromPosition eepromPosition, bool alphaChannel,
                                 RuntimeRenderingFn renderFn, MenuFlags flags, const RgbColor32& initial, MenuCallbackFn callbackFn = nullptr);

    /**
     * @brief Adds a list menu item to the menu structure with content stored in RAM.
     *
     * This method creates a runtime-configurable list menu item, where the items in the list and their metadata
     * are stored in RAM. The list menu item is added as a child to the current submenu.
     * List Docs - https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/list-menu-item/

     * @param id The unique identifier for the menu item.
     * @param name The name of the menu item, displayed in the menu.
     * @param numberOfRows The total number of rows (entries) in the list.
     * @param arrayOfItems An array of C-strings containing the list entries. These must be stored in RAM.
     * @param flags Additional configuration flags for the menu item.
     * @param callbackFn A callback function invoked when interaction with the menu item occurs.
     * @return A reference to the current TcMenuBuilder, allowing fluent-style chaining.
     */
    TcMenuBuilder& listItemRam(menuid_t id, const char *name, uint16_t numberOfRows, const char** arrayOfItems, MenuFlags flags, MenuCallbackFn callbackFn = nullptr);

    /**
    * @brief Adds a list menu item to the menu structure with content stored in FLASH.
     *
     * This method creates a runtime-configurable list menu item, where the items in the list items are stored in FLASH.
     * The list menu item is added as a child to the current submenu.
     * List Docs - https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/list-menu-item/
     *
     * @param id The unique identifier for the menu item.
     * @param name The name of the menu item, displayed in the menu.
     * @param numberOfRows The total number of rows (entries) in the list.
     * @param arrayOfItems An array of C-strings containing the list entries. These must be stored in FLASH.
     * @param flags Additional configuration flags for the menu item.
     * @param callbackFn A callback function invoked when interaction with the menu item occurs.
     * @return A reference to the current TcMenuBuilder, allowing fluent-style chaining.
     */
    TcMenuBuilder& listItemFlash(menuid_t id, const char *name, uint16_t numberOfRows, const char** arrayOfItems, MenuFlags flags, MenuCallbackFn callbackFn = nullptr);

    /**
     * @brief Adds a runtime custom list item to the menu structure.
     *
     * This method allows the addition of a list item that uses a custom runtime rendering function
     * to dynamically determine its content. The item is read-only by default and can display multiple rows of data.
     * List Docs - https://tcmenu.github.io/documentation/arduino-libraries/tc-menu/menu-item-types/list-menu-item/
     *
     * @param id The unique ID of the menu item.
     * @param name The name (or label) of the menu item.
     * @param numberOfRows The number of rows that the list item can display.
     * @param flags Additional configuration flags for the menu item.
     * @param rtRenderFn The function used to render the list item's contents dynamically at runtime.
     * @param callbackFn The callback function that is invoked when the item is interacted with.
     * @return A reference to the current `TcMenuBuilder` instance, allowing method chaining.
     */
    TcMenuBuilder& listItemRtCustom(menuid_t id, const char *name, uint16_t numberOfRows, RuntimeRenderingFn rtRenderFn, MenuFlags flags, MenuCallbackFn callbackFn = nullptr);

    /**
     * Add an item to the menu that allows for authentication using EEPROM storage. This item is read-only and displays
     * authentication status. When the authentication status changes, the provided callback function is invoked.
     *
     * @param id The unique ID of the menu item.
     * @param name The name (or label) of the menu item.
     * @param flags Menu flags to configure the item's behavior.
     * @param onAuthChanged Optional callback function invoked when authentication status changes.
     * @return A reference to the current `TcMenuBuilder` instance, allowing method chaining.
     */
    TcMenuBuilder& eepromAuthenticationItem(menuid_t id, const char* name, MenuFlags flags, MenuCallbackFn onAuthChanged = nullptr);

    /**
     * Add an item to the menu that provides remote connectivity monitoring. This item is read-only and displays
     * connectivity status.
     *
     * @param id The unique ID of the menu item.
     * @param name The name (or label) of the menu item.
     * @param flags Menu flags to configure the item's behavior.
     * @return A reference to the current `TcMenuBuilder` instance, allowing method chaining.
     */
    TcMenuBuilder& remoteConnectivityMonitor(menuid_t id, const char* name, MenuFlags flags);

    /**
     * Add an item that you've created manually, such as a custom item outside the scope of this builder. For example, if
     * you had used the traditional static method for some complex items, you could add them using this method.
     * @param itemToAdd the item to append to the menu hierarchy. The item must not be deallocated after addition!
     * @return A reference to the current TcMenuBuilder instance for method chaining.
     */
    TcMenuBuilder& appendCustomItem(MenuItem* itemToAdd);

    /**
     * @brief Ends the current submenu context and returns to the parent menu.
     *
     * This method exits the current submenu level and transitions back to the parent menu context.
     * If the current menu has no parent, it returns a reference to the root TcMenuBuilder instance.
     * This allows chaining and continuation of the menu-building process.
     *
     * @return A reference to the parent TcMenuBuilder instance, or the root builder instance if no parent exists.
     */
    TcMenuBuilder& endSub();

    /**
     * @brief Fills in an AnyInfoReserve structure with the provided menu item information.
     *
     * This method reserves and populates an AnyInfoReserve object with the specified menu item parameters.
     * It is typically used internally by the builder to prepare menu item metadata before creating and registering
     * a new menu item in the hierarchy. It is probably only useful along with custom menu item creation.
     *
     * If you're creating custom menu items, then you can use this method to set up the
     * necessary `info` metadata for your custom item before adding it to the menu hierarchy.
     *
     * @param id The unique identifier for the menu item.
     * @param name The display name of the menu item.
     * @param eeprom The EEPROM position for persisting the menu item's state, or -1 if not using EEPROM.
     * @param maxVal The maximum value for the menu item (interpretation depends on item type).
     * @param callback_fn A callback function that will be triggered when the menu item is interacted with, or nullptr if no callback is needed.
     * @return A pointer to an AnyInfoReserve object containing the populated menu item information.
     */
    AnyInfoReserve* fillInAnyInfo(menuid_t id, const char *name, int eeprom, int maxVal, MenuCallbackFn callback_fn);

    void putAtEndOfSub(MenuItem * toAdd) const;

    MenuItem * getRootItem() const {
        return currentSub->getChild();
    }

private:
    SubMenuItem* currentSub;
    TcMenuBuilder* parent;

};

#endif //TCLIBRARYDEV_TCMENUBUILDER_H
