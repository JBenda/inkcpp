/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "InkAsset.h"

#include "Misc/FileHelper.h"
#if WITH_EDITORONLY_DATA
#include "EditorFramework/AssetImportData.h"
#endif

UInkAsset::UInkAsset()
	: Super()
{
}

void UInkAsset::Serialize(FStructuredArchive::FRecord Record)
{
	Super::Serialize(Record);
	
	// Write to archive?
	Record.GetUnderlyingArchive() << CompiledStory;
}

#if WITH_EDITORONLY_DATA
void UInkAsset::PostInitProperties()
{
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
	}

	Super::PostInitProperties();
}

void UInkAsset::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
	if (AssetImportData)
	{
		Context.AddTag(FAssetRegistryTag(
		    SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden
		));
	}

	Super::GetAssetRegistryTags(Context);
}
#endif
