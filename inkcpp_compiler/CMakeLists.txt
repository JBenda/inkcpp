add_library(inkcpp_compiler compiler.cpp binary_stream.h binary_stream.cpp json.hpp)

# Make sure the include directory is included 
target_include_directories(inkcpp_compiler
    PUBLIC include
    PUBLIC ../shared/public
    PRIVATE ../shared/private
    )

# Make sure this project and all dependencies use the C++17 standard
target_compile_features(inkcpp_compiler PUBLIC cxx_std_17)

# Default installation
install(TARGETS inkcpp_compiler DESTINATION lib)