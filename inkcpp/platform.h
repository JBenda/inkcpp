#pragma once

#include "config.h"

#ifdef INK_ENABLE_STL
#include <string>
#include <sstream>
#include <fstream>
#endif
#ifdef INK_ENABLE_CSTD
#include <string.h>
#include <stdlib.h>
#endif
#ifdef INK_ENABLE_UNREAL
#include "Containers/UnrealString.h"
#endif