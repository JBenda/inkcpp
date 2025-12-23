/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "header.h"
#include "version.h"

namespace ink::internal
{

bool header::verify() const
{
	if (endian() == endian_types::none) {
		inkFail("Header magic number was wrong!");
		return false;
	}

	if (endian() == endian_types::differ) {
		inkFail("Can't load content with different endian-ness!");
		return false;
	}

	if (ink_bin_version_number != InkBinVersion) {
		inkFail("InkCpp-version mismatch: file was compiled with different InkCpp-version!");
		return false;
	}

	return true;
}

} // namespace ink::internal
