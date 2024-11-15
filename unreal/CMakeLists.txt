# download inklecate for unreal plugin
set(INKCPP_UNREAL_TARGET_VERSION "5.5" CACHE STRING "Unreal engine version the plugin should target (e.g: 5.5)")
install(CODE "
include(FetchContent)
FetchContent_Populate(inklecate_mac
URL https://github.com/inkle/ink/releases/download/v1.1.1/inklecate_mac.zip
URL_HASH SHA256=c516402bca5fa249a7712e62591b048b137eba3098c53f9fb85a4253f9b9e2c0
SOURCE_DIR \"inklecate/mac\"
SUBBUILD_DIR \"${CMAKE_CURRENT_BINARY_DIR}/../_deps/inklecate_mac-subbuild\"
)
FetchContent_Populate(inklecate_windows
URL https://github.com/inkle/ink/releases/download/v1.1.1/inklecate_windows.zip
URL_HASH SHA256=6f317cb4c59bf1b31c6dd61e80c6a2287a1d8c241a703f0586f736ae00871aab
SOURCE_DIR \"inklecate/windows\"
SUBBUILD_DIR \"${CMAKE_CURRENT_BINARY_DIR}/../_deps/inklecate_windows-subbuild\"
)
FetchContent_Populate(inklecate_linux
URL https://github.com/inkle/ink/releases/download/v1.1.1/inklecate_linux.zip
URL_HASH SHA256=26f4e188e02536d6e99e73e71d9b13e2c2144187f1368a87e82fd5066176cff8
SOURCE_DIR \"inklecate/linux\"
SUBBUILD_DIR \"${CMAKE_CURRENT_BINARY_DIR}/../_deps/inklecate_linux-subbuild\"
)
set(FETCHCONTENT_QUIET OFF)
set(CMAKE_TLS_VERIFY true)
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
configure_file(
	\"${CMAKE_CURRENT_SOURCE_DIR}/inkcpp/inkcpp.uplugin.in\"
	\"${CMAKE_CURRENT_BINARY_DIR}/inkcpp/inkcpp.uplugin\"
)
" COMPONENT unreal EXCLUDE_FROM_ALL)
# Copy files into destination directory
install(DIRECTORY "inkcpp/" DESTINATION "inkcpp" COMPONENT unreal EXCLUDE_FROM_ALL
	PATTERN "*.in" EXCLUDE)
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/../inklecate" DESTINATION "inkcpp/Source/ThirdParty" COMPONENT unreal EXCLUDE_FROM_ALL)
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/inkcpp" DESTINATION "." COMPONENT unreal EXCLUDE_FROM_ALL)
