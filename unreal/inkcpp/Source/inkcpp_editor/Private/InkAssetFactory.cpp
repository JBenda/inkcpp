#include "InkAssetFactory.h"

#include "EditorFramework/AssetImportData.h"
#include "Misc/FileHelper.h"
#include "Interfaces/IPluginManager.h"

#include "InkAsset.h"
#include "ink/compiler.h"
#include "inklecate_cmd.cpp"

#include <sstream>
#include <cstdlib>

DECLARE_LOG_CATEGORY_EXTERN(InkCpp, Log, All);
DEFINE_LOG_CATEGORY(InkCpp);

UInkAssetFactory::UInkAssetFactory(const FObjectInitializer& ObjectInitializer)
	: UFactory(ObjectInitializer), FReimportHandler()
{
	// Add ink format
	Formats.Add(FString(TEXT("json;")) + NSLOCTEXT("UInkAssetFactory", "FormatInkJSON", "Ink JSON File").ToString());
	Formats.Add(FString(TEXT("ink;")) + NSLOCTEXT("UInkAssetFactory", "FormatInk", "Ink File").ToString());

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
	std::stringstream cmd;
	static const std::string inklecate_cmd{ INKLECATE_CMD };
	static const std::string ink_suffix{".ink"};
	try
	{
		std::string cFilename = TCHAR_TO_ANSI(*Filename);
		if (cFilename.size() > ink_suffix.size()
				&& std::equal(ink_suffix.rbegin(), ink_suffix.rend(), cFilename.rbegin()))
		{
			if(inklecate_cmd.size() == 0) {
				UE_LOG(InkCpp, Warning, TEXT("Inklecate provided with the plugin, please import ink.json files"));
				return nullptr;
			}
			cmd 
				<< TCHAR_TO_ANSI(*IPluginManager::Get().FindPlugin(TEXT("InkCPP"))->GetContentDir())
				<< inklecate_cmd << " -o " << cFilename << ".json " << cFilename;
			int result = std::system(cmd.str().c_str());
			if (result != 0) {
				UE_LOG(InkCpp, Warning, TEXT("Inklecate failed with exit code %i"), result);
				return nullptr;
			}
			cFilename += ".json";
		}
		ink::compiler::run(cFilename.c_str(), output);

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
