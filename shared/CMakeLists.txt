add_library(inkcpp_shared INTERFACE)

target_include_directories(inkcpp_shared
	INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/public>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/private>
	$<INSTALL_INTERFACE:inkcpp>
)
FILE(GLOB PUBLIC_HEADERS "public/*")
set_target_properties(inkcpp_shared PROPERTIES PUBLIC_HEADER "${PUBLIC_HEADERS}")

# Unreal installation
install(DIRECTORY "public/" DESTINATION "inkcpp/Source/shared/Public/" COMPONENT unreal EXCLUDE_FROM_ALL)
install(DIRECTORY "private/" DESTINATION "inkcpp/Source/shared/Private/" COMPONENT unreal EXCLUDE_FROM_ALL)
