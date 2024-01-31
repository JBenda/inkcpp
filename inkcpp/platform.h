/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
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
