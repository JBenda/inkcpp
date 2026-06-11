/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "InkCppEditorSettings.h"

#include "Misc/Paths.h"

#include <string>

/**
 * Returns the path to the inklecate executable as a UTF-8 string.
 *
 * Resolution order:
 *  1. Value set in Project Settings > Plugins > InkCPP (InklcatePath).
 *  2. Empty string — caller interprets this as "inklecate not configured".
 *
 * A non-empty path does NOT guarantee the file exists; the caller is
 * responsible for checking existence and showing the setup dialog if needed.
 */
inline std::string get_inklecate_cmd()
{
	const UInkCppEditorSettings* Settings = GetDefault<UInkCppEditorSettings>();
	if (Settings) {
		FString ConfiguredPath = Settings->GetInklecatePath().TrimStartAndEnd();
		if (! ConfiguredPath.IsEmpty()) {
			// Convert any forward/backward slashes to the OS-preferred separator
			FPaths::NormalizeFilename(ConfiguredPath);
			return std::string(TCHAR_TO_UTF8(*ConfiguredPath));
		}
	}
	// No path configured – signal "not found" to the caller
	return std::string{};
}
