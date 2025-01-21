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

target_compile_definitions(tcMenu
        PUBLIC BUILD_FOR_PICO_CMAKE=1 BUILD_PICO_FORCE_UART=1 IO_LOGGING_DEBUG=1
)

target_include_directories(tcMenu PUBLIC
        ${CMAKE_CURRENT_LIST_DIR}/../src
)

target_link_libraries(tcMenu PUBLIC pico_stdlib pico_sync IoAbstraction TaskManagerIO tcUnicodeHelper)
