/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "inkcpp_editor.h"

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include <AssetToolsModule.h>
#include "EditorFramework/AssetImportData.h"
#include "InkAsset.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

class FInkAssetActions : public IAssetTypeActions
{
public:
	// IAssetTypeActions interface
	virtual FText GetName() const override { return LOCTEXT("FInkAssetActions", "InkAsset"); }

	virtual FColor GetTypeColor() const override { return FColor::Red; }

	virtual UClass* GetSupportedClass() const override { return UInkAsset::StaticClass(); }

	virtual uint32 GetCategories() override { return EAssetTypeCategories::Media; }

	virtual bool IsImportedAsset() const override { return true; }

	virtual void GetResolvedSourceFilePaths(
	    const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths
	) const override
	{
		for (auto& Asset : TypeAssets) {
			const auto InkAsset = Cast<UInkAsset>(Asset);
			if (InkAsset->AssetImportData) {
				InkAsset->AssetImportData->ExtractFilenames(OutSourceFilePaths);
			}
		}
	}

	bool ShouldFindEditorForAsset() const override { return false; }

	// Inherited via IAssetTypeActions
	void OpenAssetEditor(
	    const TArray<UObject*>& InObjects, TSharedPtr<IToolkitHost> EditWithinLevelEditor
	) override
	{
	}

	void OpenAssetEditor(
	    const TArray<UObject*>& InObjects, const EAssetTypeActivationOpenedMethod OpenedMethod,
	    TSharedPtr<IToolkitHost> EditWithinLevelEditor
	) override
	{
	}

	bool AssetsActivatedOverride(
	    const TArray<UObject*>& InObjects, EAssetTypeActivationMethod::Type ActivationType
	) override
	{
		return false;
	}

	bool CanRename(const FAssetData& InAsset, FText* OutErrorMsg) const override { return true; }

	bool CanDuplicate(const FAssetData& InAsset, FText* OutErrorMsg) const override { return false; }

	TArray<FAssetData> GetValidAssetsForPreviewOrEdit(
	    TArrayView<const FAssetData> InAssetDatas, bool bIsPreview
	) override
	{
		return TArray<FAssetData>();
	}

	bool CanFilter() override { return false; }

	FName GetFilterName() const override { return FName(); }

	bool CanLocalize() const override { return false; }

	bool CanMerge() const override { return false; }

	void Merge(UObject* InObject) override {}

	void Merge(
	    UObject* BaseAsset, UObject* RemoteAsset, UObject* LocalAsset,
	    const FOnMergeResolved& ResolutionCallback
	) override
	{
	}

	FString GetObjectDisplayName(UObject* Object) const override { return FString("InkAsset"); }

	const TArray<FText>& GetSubMenus() const override
	{
		// TODO: insert return statement here
		static TArray<FText> SubMenus;
		return SubMenus;
	}

	bool ShouldForceWorldCentric() override { return false; }

	void PerformAssetDiff(
	    UObject* OldAsset, UObject* NewAsset, const FRevisionInfo& OldRevision,
	    const FRevisionInfo& NewRevision
	) const override
	{
	}

	UThumbnailInfo* GetThumbnailInfo(UObject* Asset) const override { return nullptr; }

	EThumbnailPrimType GetDefaultThumbnailPrimitiveType(UObject* Asset) const override
	{
		return EThumbnailPrimType();
	}

	TSharedPtr<class SWidget> GetThumbnailOverlay(const FAssetData& AssetData) const override
	{
		return TSharedPtr<class SWidget>();
	}

	FText GetAssetDescription(const FAssetData& AssetData) const override
	{
		return LOCTEXT("InkAssetDescription", "In Inkle story compiled and ready to use with InkCPP");
	}

	void GetSourceFileLabels(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFileLabels)
	    const override
	{
		for (const UObject* obj : TypeAssets) {
			const UInkAsset* InkAsset = Cast<UInkAsset>(obj);
			if (InkAsset != nullptr && InkAsset->AssetImportData) {
				InkAsset->AssetImportData->ExtractDisplayLabels(OutSourceFileLabels);
			}
		}
	}

	void BuildBackendFilter(FARFilter& InFilter) override {}

	FText GetDisplayNameFromAssetData(const FAssetData& AssetData) const override { return FText(); }

	FTopLevelAssetPath GetClassPathName() const override
	{
		return GetSupportedClass()->GetClassPathName();
	}

	bool SupportsOpenedMethod(const EAssetTypeActivationOpenedMethod OpenedMethod) const override
	{
		return false;
	}

	const FSlateBrush*
	    GetThumbnailBrush(const FAssetData& InAssetData, const FName InClassName) const override
	{
		return nullptr;
	}

	const FSlateBrush*
	    GetIconBrush(const FAssetData& InAssetData, const FName InClassName) const override
	{
		return nullptr;
	}

	// End of IAssetTypeActions interface
};

class FInkAssetModuleImpl : public FDefaultModuleImpl
{
public:
	virtual void StartupModule() override
	{
		// Register asset types
		IAssetTools& AssetTools
		    = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

		SpriteSheetImportAssetTypeActions = MakeShareable(new FInkAssetActions);
		AssetTools.RegisterAssetTypeActions(SpriteSheetImportAssetTypeActions.ToSharedRef());
	}

	virtual void ShutdownModule() override
	{
		if (FModuleManager::Get().IsModuleLoaded("AssetTools")) {
			IAssetTools& AssetTools
			    = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
			if (SpriteSheetImportAssetTypeActions.IsValid()) {
				AssetTools.UnregisterAssetTypeActions(SpriteSheetImportAssetTypeActions.ToSharedRef());
			}
		}
	}

private:
	TSharedPtr<IAssetTypeActions> SpriteSheetImportAssetTypeActions;
};

//////////////////////////////////////////////////////////////////////////

IMPLEMENT_MODULE(FInkAssetModuleImpl, inkcpp_editor);
DEFINE_LOG_CATEGORY(InkCpp);
