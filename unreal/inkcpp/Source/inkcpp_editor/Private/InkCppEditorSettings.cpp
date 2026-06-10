/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "InkCppEditorSettings.h"

UInkCppEditorSettings::UInkCppEditorSettings()
{
	CategoryName = TEXT("Plugins");
	SectionName  = TEXT("InkCPP");
}

FName UInkCppEditorSettings::GetCategoryName() const { return TEXT("Plugins"); }

FText UInkCppEditorSettings::GetSectionText() const
{
	return NSLOCTEXT("InkCPP", "SettingsSectionText", "InkCPP");
}

FText UInkCppEditorSettings::GetSectionDescription() const
{
	return NSLOCTEXT(
	    "InkCPP", "SettingsSectionDescription",
	    "Settings for the InkCPP Unreal plugin. "
	    "Configure the path to the inklecate compiler used to import .ink story files."
	);
}

FString UInkCppEditorSettings::GetInklecatePath() const { return InklatePath.FilePath; }
