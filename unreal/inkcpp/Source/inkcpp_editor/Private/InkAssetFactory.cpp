#include "InkAssetFactory.h"

#include "EditorFramework/AssetImportData.h"
#include "Misc/FileHelper.h"

#include "InkAsset.h"
#include "ink/compiler.h"

#include <sstream>

UInkAssetFactory::UInkAssetFactory(const FObjectInitializer& ObjectInitializer)
	: UFactory(ObjectInitializer), FReimportHandler()
{
	// Add ink format
	Formats.Add(FString(TEXT("json;")) + NSLOCTEXT("UInkAssetFactory", "FormatInkJSON", "Ink JSON File").ToString());

	// Set class
	SupportedClass = UInkAsset::StaticClass();
	bCreateNew = false;
	bEditorImport = true;

	// Fuck data tables TODO - some criteria?
	ImportPriority = 99999;
}

UObject* UInkAssetFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	std::stringstream output;
	try
	{
		ink::compiler::run(TCHAR_TO_ANSI(*Filename), output);

		// Create ink asset
		UInkAsset* asset = NewObject<UInkAsset>(InParent, InClass, InName, Flags);

		// Load it up
		std::string data = output.str();
		asset->CompiledStory.SetNum(data.length());
		FMemory::Memcpy(asset->CompiledStory.GetData(), data.c_str(), data.length());

		// Paths
		asset->AssetImportData->Update(CurrentFilename);

		// Not cancelled
		bOutOperationCanceled = false;

		// Return
		return asset;
	}
	catch (...)
	{
		// some kind of error?
		return nullptr;
	}

	return nullptr;
}

bool UInkAssetFactory::FactoryCanImport(const FString& Filename)
{
	return true; // Fuck you Unreal
}

bool UInkAssetFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	// Can reimport story assets
	UInkAsset* InkAsset = Cast<UInkAsset>(Obj);
	if (InkAsset != nullptr && InkAsset->AssetImportData)
	{
		InkAsset->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}

	return false;
}

void UInkAssetFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UInkAsset* InkAsset = Cast<UInkAsset>(Obj);
	if (InkAsset && ensure(NewReimportPaths.Num() == 1))
	{
		InkAsset->AssetImportData->UpdateFilenameOnly(NewReimportPaths[0]);
	}
}

int32 UInkAssetFactory::GetPriority() const
{
	return ImportPriority;
}

const UObject* UInkAssetFactory::GetFactoryObject() const
{
	return this;
}

EReimportResult::Type UInkAssetFactory::Reimport(UObject* Obj)
{
	UInkAsset* InkAsset = Cast<UInkAsset>(Obj);
	if (!InkAsset)
		return EReimportResult::Failed;

	const FString Filename = InkAsset->AssetImportData->GetFirstFilename();
	if (!Filename.Len() || IFileManager::Get().FileSize(*Filename) == INDEX_NONE)
	{
		return EReimportResult::Failed;
	}

	// Run the import again
	EReimportResult::Type Result = EReimportResult::Failed;
	bool OutCanceled = false;

	if (ImportObject(InkAsset->GetClass(), InkAsset->GetOuter(), *InkAsset->GetName(), RF_Public | RF_Standalone, Filename, nullptr, OutCanceled) != nullptr)
	{
		InkAsset->AssetImportData->Update(Filename);

		// Try to find the outer package so we can dirty it up
		if (InkAsset->GetOuter())
		{
			InkAsset->GetOuter()->MarkPackageDirty();
		}
		else
		{
			InkAsset->MarkPackageDirty();
		}
		Result = EReimportResult::Succeeded;
	}
	else
	{
		Result = EReimportResult::Failed;
	}

	return Result;
}
