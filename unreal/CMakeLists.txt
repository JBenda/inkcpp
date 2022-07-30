
include(FetchContent)
set(CMAKE_TLS_VERIFY true)
FetchContent_Declare(inklecate_mac
URL https://github.com/inkle/ink/releases/download/v1.0.0/inklecate_mac.zip
URL_HASH SHA256=B6F4DD1F95C180637CE193DBB5FA6D59AEAFE49A2121A05B7822E6CBBAA6931F
SOURCE_DIR "inkcpp/Resources/inklecate/mac"
)
FetchContent_Declare(inklecate_windows
URL https://github.com/inkle/ink/releases/download/v1.0.0/inklecate_windows.zip
URL_HASH SHA256=A93A5E03109FBD92C636374132253D6D1AEF62B3E060A7FD8CAB97DA883A8327
SOURCE_DIR "inkcpp/Resources/inklecate/windows"
)
FetchContent_Declare(inklecate_linux
URL https://github.com/inkle/ink/releases/download/v1.0.0/inklecate_linux.zip
URL_HASH SHA256=6E17DB766222998BA0AE5A5DA9857E34896E683B9EC42FAD528C3F8BEA7398EA
SOURCE_DIR "inkcpp/Resources/inklecate/linux"
)
set(INKLECATE_CMD "")
if(WIN32)
	FetchContent_MakeAvailable(inklecate_windows)
	if(NOT inklecate_windows_SOURCE_DIR)
	message(WARNING "failed to downloawd inklecate for windows, "
		"the unreal plugin will be unable use a .ink file as asset directly")
	else()
	set(INKLECATE_CMD "Resources/inklecate/windows/inklecate.exe")
	endif()
elseif(APPLE)
	FetchContent_MakeAvailable(inklecate_mac)
	if(NOT inklecate_mac_SOURCE_DIRE)
		message(WARNING "failed to downloawd inklecate for MacOS, "
			"the unreal plugin will be unable use a .ink file as asset directly")
	else()
	set(INKLECATE_CMD "Resources/inklecate/mac/inklecate")
	endif()
elseif(UNIX)
	FetchContent_MakeAvailable(inklecate_linux)
	if(NOT inklecate_linux_SOURCE_DIR)
		message(WARNING "failed to downloawd inklecate for linux, "
			"the unreal plugin will be unable use a .ink file as asset directly")
	else()
	set(INKLECATE_CMD "Resources/inklecate/linux/inklecate")
	endif()
else()
message(WARNING "unable to determine OS -> unreal component cant compile ink files directly"
	" to fix this may define WIN32/APPLE or UNIX manually and run again")
endif()

configure_file(
	"${CMAKE_CURRENT_SOURCE_DIR}/inkcpp/Source/inkcpp_editor/Private/inklecate_cmd.cpp.in"
	"${CMAKE_CURRENT_BINARY_DIR}/inkcpp/Source/inkcpp_editor/Private/inklecate_cmd.cpp"
)

# Copy files into destination directory
install(DIRECTORY "inkcpp/" DESTINATION "inkcpp" COMPONENT unreal EXCLUDE_FROM_ALL
	PATTERN "*.in" EXCLUDE)
install(DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/inkcpp/" DESTINATION "inkcpp" COMPONENT unreal EXCLUDE_FROM_ALL)
