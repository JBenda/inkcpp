/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "UObject/Object.h"

#include "InkAsset.generated.h"

/** Assets contanining a InkCPP .bin.
 * Asset can be constructed from a `.ink.json` file outputed from inky or inklecate.
 * And generall also directly from a `.ink` file (it may fail if the shipped version of inklecate is
 * incopatible with your system).
 *
 * @todo Please note that reimport does not work properly if your ink file has includes.
 * Since the reimport only watches the main file for changes.
 * @ingroup unreal
 */
UCLASS(hidecategories=Object)
class INKCPP_API UInkAsset : public UObject
{
	GENERATED_BODY()
	
public:
	UInkAsset();

	// Begin UObject
	/** @private */
	virtual void Serialize(FStructuredArchive::FRecord Record) override;
	// End UObject

	/** @private */
	TArray<char> CompiledStory;

#if WITH_EDITORONLY_DATA
	/** @private */
	UPROPERTY(VisibleAnywhere, Instanced, Category = ImportSettings)
	class UAssetImportData* AssetImportData;

	// Begin UObject
	/** @private */
	virtual void PostInitProperties() override;
	/** @private */
	virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const override;
	// End UObject
#endif
};
