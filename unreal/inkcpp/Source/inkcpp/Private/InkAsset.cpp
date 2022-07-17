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

void UInkAsset::GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const
{
	if (AssetImportData)
	{
		OutTags.Add(FAssetRegistryTag(SourceFileTagName(), AssetImportData->GetSourceData().ToJson(), FAssetRegistryTag::TT_Hidden));
	}

	Super::GetAssetRegistryTags(OutTags);
}
#endif