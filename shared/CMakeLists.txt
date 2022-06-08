add_library(inkcpp_shared INTERFACE)
target_include_directories(inkcpp_shared INTERFACE public private)
FILE(GLOB PUBLIC_HEADERS "public/*")
# Install public files into the include directory
install(FILES ${PUBLIC_HEADERS}
	DESTINATION "include/inkcpp" COMPONENT lib EXCLUDE_FROM_ALL)

# Unreal installation
install(DIRECTORY "public/" DESTINATION "inkcpp/Source/shared/Public/" COMPONENT unreal EXCLUDE_FROM_ALL)
install(DIRECTORY "private/" DESTINATION "inkcpp/Source/shared/Private/" COMPONENT unreal EXCLUDE_FROM_ALL)
