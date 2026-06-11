/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "Engine/DeveloperSettings.h"
#include "InkCppEditorSettings.generated.h"

/**
 * Editor settings for the InkCPP plugin.
 *
 * Accessible via Edit > Project Settings > Plugins > InkCPP.
 *
 * @ingroup unreal
 */
UCLASS(config = Editor, DefaultConfig)

class INKCPP_EDITOR_API UInkCppEditorSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UInkCppEditorSettings();

	// ~Begin UDeveloperSettings
	virtual FName GetContainerName() const override;
	virtual FName GetCategoryName() const override;
	virtual FName GetSectionName() const override;
	virtual FText GetSectionText() const override;
	virtual FText GetSectionDescription() const override;
	// ~End UDeveloperSettings

	/**
	 * Full path to the inklecate executable used to compile .ink files.
	 *
	 * Leave empty to search the system PATH. If no executable is found when importing a .ink file
	 * a setup guide will be shown.
	 *
	 * Recommended location: place the platform-specific inklecate binary inside your project's
	 * Content folder so it travels with the project, e.g.
	 *   Windows : Content/inklecate/inklecate.exe
	 *   Mac/Linux: Content/inklecate/inklecate
	 *
	 * The correct version for this build of InkCPP can be downloaded from:
	 *   https://github.com/inkle/ink/releases
	 */
	UPROPERTY(
	    config, EditAnywhere, Category = "Ink",
	    meta
	    = (DisplayName = "Inklecate Executable Path",
	       ToolTip = "Absolute path to the inklecate executable.", RelativeToGameDir = false)
	)
	FFilePath InklcatePath;

	/** Returns the configured path string, or an empty string if not set. */
	FString GetInklecatePath() const;
};
