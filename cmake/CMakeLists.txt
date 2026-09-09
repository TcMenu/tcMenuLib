cmake_minimum_required(VERSION 3.13)

add_library(tcMenu 
        ../src/BaseDialog.cpp
        ../src/BaseRenderers.cpp
        ../src/EditableLargeNumberMenuItem.cpp
        ../src/EepromItemStorage.cpp
        ../src/MenuHistoryNavigator.cpp
        ../src/MenuItems.cpp
        ../src/MenuIterator.cpp
        ../src/MessageProcessors.cpp
        ../src/RemoteAuthentication.cpp
        ../src/RemoteConnector.cpp
        ../src/RemoteMenuItem.cpp
        ../src/RuntimeMenuItem.cpp
        ../src/ScrollChoiceMenuItem.cpp
        ../src/SecuredMenuPopup.cpp
        ../src/tcMenu.cpp
        ../src/tcMenuKeyboard.cpp
        ../src/tcUtil.cpp
        ../src/TcMenuBuilder.cpp
        ../src/extras/DrawableDashboard.cpp
        ../src/extras/DrawableTouchCalibrator.cpp
        ../src/extras/MenuItemDelegate.cpp
        ../src/extras/TwoButtonSwitchEncoder.cpp
        ../src/graphics/BaseGraphicalRenderer.cpp
        ../src/graphics/CardLayoutPanel.cpp
        ../src/graphics/DeviceDrawable.cpp
        ../src/graphics/DeviceDrawableHelper.cpp
        ../src/graphics/DialogRuntimeEditor.cpp
        ../src/graphics/GfxMenuConfig.cpp
        ../src/graphics/GraphicsDeviceRenderer.cpp
        ../src/graphics/MenuTouchScreenEncoder.cpp
        ../src/graphics/RuntimeTitleMenuItem.cpp
        ../src/graphics/TcDrawableButton.cpp
        ../src/graphics/TcThemeBuilder.cpp
        ../src/remote/BaseBufferedRemoteTransport.cpp
        ../src/remote/BaseRemoteComponents.cpp
)

if (BUILD_FOR_STM32CUBE)
    target_compile_definitions(tcMenu
            PUBLIC BUILD_FOR_STM32CUBE_CMAKE=1 IO_LOGGING_DEBUG=1
    )
elseif (BUILD_NATIVE_TESTS)
    target_compile_definitions(tcMenu
            PUBLIC BUILD_FOR_NATIVE_PLATFORM=1 IO_LOGGING_DEBUG=1
    )
else ()
    target_compile_definitions(tcMenu
            PUBLIC BUILD_FOR_PICO_CMAKE=1 BUILD_PICO_FORCE_UART=1 IO_LOGGING_DEBUG=1
    )
endif ()

target_include_directories(tcMenu PUBLIC
        ../src
)

target_link_libraries(tcMenu PUBLIC IoAbstraction TaskManagerIO tcUnicodeHelper)

# only bring in pico libs when compiling for PicoSDK
if(TARGET pico_stdlib AND TARGET pico_sync)
    message("Build is pico, adding libs")
    target_link_libraries(TaskManagerIO PUBLIC pico_stdlib pico_sync)
endif()

if (BUILD_NATIVE_TESTS)
    get_unity_if_needed()
    add_executable(test_tccore_items
            ../test/test_core_items/test_main.cpp
            ../test/test_core_items/CoreMenuItemTests.cpp
            ../test/test_core_items/DateTimeItemTestCases.cpp
            ../test/test_core_items/LargeNumberItemTests.cpp
            ../test/test_core_items/RuntimeItemTestCases.cpp
            ../test/test_core_items/ScrollChoiceMenuItemTests.cpp
            ../test/test_core_items/ValueItemTestCases.cpp
    )

    add_executable(test_menu_manager
            ../test/test_menumgr/test_main.cpp
            ../test/test_menumgr/authenticationTests.cpp
            ../test/test_menumgr/menuManagerTests.cpp
            ../test/test_menumgr/tcMenuCoreTests.cpp
            ../test/test_menumgr/menuNavigatorTests.cpp
    )

    add_executable(test_tcmenu_render
            ../test/test_rendering/test_main.cpp
            ../test/test_rendering/baseDialogTests.cpp
            ../test/test_rendering/CoreRendererTests.cpp
    )

    target_link_libraries(test_tccore_items PRIVATE unity TcMenuLog TaskManagerIO IoAbstraction tcMenu)
    target_link_libraries(test_menu_manager PRIVATE unity TcMenuLog TaskManagerIO IoAbstraction tcMenu)
    target_link_libraries(test_tcmenu_render PRIVATE unity TcMenuLog TaskManagerIO IoAbstraction tcMenu)
    add_test(NAME test_tccore_items COMMAND test_tccore_items)
    add_test(NAME test_menu_manager COMMAND test_menu_manager)
    add_test(NAME test_tcmenu_render COMMAND test_tcmenu_render)
endif ()
