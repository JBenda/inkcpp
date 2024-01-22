if(${CMAKE_VERSION} VERSION_GREATER "3.24.0")
	cmake_policy(SET CMP0135 NEW)
endif()
include(FetchContent)
set(CMAKE_TLS_VERIFY true)
FetchContent_Declare(inklecate_mac
URL https://github.com/inkle/ink/releases/download/v1.1.1/inklecate_mac.zip
URL_HASH SHA256=c516402bca5fa249a7712e62591b048b137eba3098c53f9fb85a4253f9b9e2c0
SOURCE_DIR "inkcpp/Resources/inklecate/mac"
)
FetchContent_Declare(inklecate_windows
URL https://github.com/inkle/ink/releases/download/v1.1.1/inklecate_windows.zip
URL_HASH SHA256=6f317cb4c59bf1b31c6dd61e80c6a2287a1d8c241a703f0586f736ae00871aab
SOURCE_DIR "inkcpp/Resources/inklecate/windows"
)
FetchContent_Declare(inklecate_linux
URL https://github.com/inkle/ink/releases/download/v1.1.1/inklecate_linux.zip
URL_HASH SHA256=26f4e188e02536d6e99e73e71d9b13e2c2144187f1368a87e82fd5066176cff8
SOURCE_DIR "inkcpp/Resources/inklecate/linux"
)

set(FETCHCONTENT_QUIET OFF)

FetchContent_MakeAvailable(inklecate_windows)
if(NOT inklecate_windows_SOURCE_DIR)
message(WARNING "failed to download inklecate for windows, "
	"the unreal plugin will be unable use a .ink file as asset directly")
else()
set(INKLECATE_CMD_WIN "Resources/inklecate/windows/inklecate.exe")
endif()

FetchContent_MakeAvailable(inklecate_mac)
if(NOT inklecate_mac_SOURCE_DIR)
	message(WARNING "failed to download inklecate for MacOS, "
		"the unreal plugin will be unable use a .ink file as asset directly")
else()
set(INKLECATE_CMD_MAC "Resources/inklecate/mac/inklecate")
endif()

FetchContent_MakeAvailable(inklecate_linux)
if(NOT inklecate_linux_SOURCE_DIR)
	message(WARNING "failed to download inklecate for linux, "
		"the unreal plugin will be unable use a .ink file as asset directly")
else()
set(INKLECATE_CMD_LINUX "Resources/inklecate/linux/inklecate")
endif()

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/inkcpp/Source/inkcpp_editor/Private/inklecate_cmd.cpp.in"
	"${CMAKE_CURRENT_BINARY_DIR}/inkcpp/Source/inkcpp_editor/Private/inklecate_cmd.cpp"
)

# Copy files into destination directory
install(DIRECTORY "inkcpp/" DESTINATION "inkcpp" COMPONENT unreal EXCLUDE_FROM_ALL
	PATTERN "*.in" EXCLUDE)
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/inkcpp" DESTINATION "." COMPONENT unreal EXCLUDE_FROM_ALL)
