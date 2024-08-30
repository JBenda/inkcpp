# download inklecate for unreal plugin
install(CODE "
FetchContent_MakeAvailable(inklecate_windows inklecate_mac inklecate_linux)
if(NOT inklecate_windows_SOURCE_DIR)
message(WARNING \"failed to download inklecate for windows, \"
	\"the unreal plugin will be unable use a .ink file as asset directly\")
else()
set(INKLECATE_CMD_WIN \"Source/ThirdParty/inklecate/windows/inklecate.exe\")
endif()
if(NOT inklecate_mac_SOURCE_DIR)
	message(WARNING \"failed to download inklecate for MacOS, \"
		\"the unreal plugin will be unable use a .ink file as asset directly\")
else()
set(INKLECATE_CMD_MAC \"Source/ThirdParty/inklecate/mac/inklecate\")
endif()
if(NOT inklecate_linux_SOURCE_DIR)
	message(WARNING \"failed to download inklecate for linux, \"
		\"the unreal plugin will be unable use a .ink file as asset directly\")
else()
set(INKLECATE_CMD_LINUX \"Source/ThirdParty/inklecate/linux/inklecate\")
endif()
configure_file(
	\"${CMAKE_CURRENT_SOURCE_DIR}/inkcpp/Source/inkcpp_editor/Private/inklecate_cmd.cpp.in\"
	\"${CMAKE_CURRENT_BINARY_DIR}/inkcpp/Source/inkcpp_editor/Private/inklecate_cmd.cpp\"
)
" COMPONENT unreal EXCLUDE_FROM_ALL)
# Copy files into destination directory
install(DIRECTORY "inkcpp/" DESTINATION "inkcpp" COMPONENT unreal EXCLUDE_FROM_ALL
	PATTERN "*.in" EXCLUDE)
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/inkcpp" DESTINATION "." COMPONENT unreal EXCLUDE_FROM_ALL)
