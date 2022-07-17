#pragma once

#include "UObject/Object.h"

#include "InkAsset.generated.h"

UCLASS(hidecategories=Object)
class INKCPP_API UInkAsset : public UObject
{
	GENERATED_BODY()
	
public:
	UInkAsset();

	// Begin UObject
	virtual void Serialize(FStructuredArchive::FRecord Record) override;
	// End UObject

	TArray<char> CompiledStory;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, Instanced, Category = ImportSettings)
	class UAssetImportData* AssetImportData;

	// Begin UObject
	virtual void PostInitProperties() override;
	virtual void GetAssetRegistryTags(TArray<FAssetRegistryTag>& OutTags) const override;
	// End UObject
#endif
};