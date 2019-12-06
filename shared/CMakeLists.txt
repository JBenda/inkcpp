# Install public files into the include directory
install(DIRECTORY public DESTINATION include)

# Unreal installation
install(DIRECTORY "public/" DESTINATION "inkcpp/Source/shared/Public/" COMPONENT unreal)
install(DIRECTORY "private/" DESTINATION "inkcpp/Source/shared/Private/" COMPONENT unreal)