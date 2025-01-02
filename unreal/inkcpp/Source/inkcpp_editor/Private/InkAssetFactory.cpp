/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "InkAssetFactory.h"

#include "EditorFramework/AssetImportData.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Interfaces/IPluginManager.h"
#include "Internationalization/Regex.h"

#include "InkAsset.h"
#include "ink/compiler.h"
#include "inklecate_cmd.cpp"

#include <sstream>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <unordered_set>
#include <cstdio>

UInkAssetFactory::UInkAssetFactory(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
    , object_ptr(this)
{
	// Add ink format
	Formats.Add(
	    FString(TEXT("json;"))
	    + NSLOCTEXT("UInkAssetFactory", "FormatInkJSON", "Ink JSON File").ToString()
	);
	Formats.Add(
	    FString(TEXT("ink;")) + NSLOCTEXT("UInkAssetFactory", "FormatInk", "Ink File").ToString()
	);

	// Set class
	SupportedClass    = UInkAsset::StaticClass();
	bCreateNew        = false;
	bAutomatedReimport = true;
	bForceShowDialog  = true;
	bEditorImport     = true;

	ImportPriority = 20;
}

UInkAssetFactory::~UInkAssetFactory() { FReimportManager::Instance()->UnregisterHandler(*this); }

/// @todo only finds first include match?
void TraversImports(
    UAssetImportData& AssetImportData, std::unordered_set<std::filesystem::path>& visited,
    std::filesystem::path filepath
)
{
	UE_LOG(
	    InkCpp, Display, TEXT("InkAsset Import: Traverse '%s'"), *FString(filepath.string().c_str())
	);
	if (visited.find(filepath) != visited.end()) {
		return;
	}
	int id = visited.size();
	visited.insert(filepath);
	AssetImportData.AddFileName(
	    FString(filepath.string().c_str()), id, id == 0 ? TEXT("MainFile") : TEXT("Include")
	);

	std::ifstream file(filepath);
	if (! file) {
		UE_LOG(
		    InkCpp, Warning, TEXT("Failed to open story file: %s"), *FString(filepath.string().c_str())
		);
		return;
	}
	std::stringstream file_data;
	file_data << file.rdbuf();
	FRegexMatcher matcher(
	    FRegexPattern(FString("(^|\n)[ \t]*INCLUDE[ \t]+(.*)"), ERegexPatternFlags{0}),
	    FString(file_data.str().c_str())
	);
	while (matcher.FindNext()) {
		std::filesystem::path match_file_path = filepath;
		match_file_path.replace_filename(TCHAR_TO_ANSI(*matcher.GetCaptureGroup(2)));
		TraversImports(AssetImportData, visited, match_file_path);
	}
}

UObject* UInkAssetFactory::FactoryCreateFile(
    UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename,
    const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled
)
{
	std::stringstream        output;
	std::stringstream        cmd{};
	const std::string        inklecate_cmd = get_inklecate_cmd();
	static const std::string ink_suffix{".ink"};
	try {
		using path            = std::filesystem::path;
		std::string cFilename = TCHAR_TO_ANSI(*Filename);
		path        story_path(cFilename, path::format::generic_format);
		story_path.make_preferred();
		bool use_temp_file = false;
		if (cFilename.size() > ink_suffix.size()
		    && std::equal(ink_suffix.rbegin(), ink_suffix.rend(), cFilename.rbegin())) {
			use_temp_file = true;
			if (inklecate_cmd.size() == 0) {
				UE_LOG(
				    InkCpp, Warning,
				    TEXT("Inklecate provided with the plugin, please import ink.json files")
				);
				return nullptr;
			}
			path path_bin(
			    TCHAR_TO_ANSI(*IPluginManager::Get().FindPlugin(TEXT("InkCPP"))->GetBaseDir()),
			    path::format::generic_format
			);
			path_bin.make_preferred();
			path_bin /= path(inklecate_cmd, path::format::generic_format).make_preferred();
			const char* filename = std::tmpnam(nullptr);
			if (filename == nullptr) {
				UE_LOG(InkCpp, Error, TEXT("Failed to create temporary file"));
				return nullptr;
			}
			cFilename = filename;
			path json_path(cFilename, path::format::generic_format);
			json_path.make_preferred();
			cmd
			    // if std::system start with a quote, the pair of qoute is removed, which leads to errors
			    // with pathes with spaces but if the quote is not the first symbol it works fine (a"b" is
			    // glued to ab from bash)
			    << path_bin.string()[0] << "\"" << (path_bin.string().c_str() + 1) << "\""
			    << " -o \"" << json_path.string() << "\" " << '"' << story_path.string() << "\" 2>&1";
			auto cmd_str = cmd.str();
			int  result  = std::system(cmd_str.c_str());
			if (result != 0) {
				UE_LOG(
				    InkCpp, Warning, TEXT("Inklecate failed with exit code %i, executed was: '%s'"), result,
				    *FString(cmd_str.c_str())
				);
				return nullptr;
			}
		}
		ink::compiler::run(cFilename.c_str(), output);
		if (use_temp_file) {
			std::filesystem::remove(cFilename);
		}

		// Create ink asset
		UInkAsset* asset = NewObject<UInkAsset>(InParent, InClass, InName, Flags);
		if (! asset->AssetImportData) {
			asset->AssetImportData = NewObject<UAssetImportData>(asset, UAssetImportData::StaticClass());
		}

		// Load it up
		std::string data = output.str();
		asset->CompiledStory.SetNum(data.length());
		FMemory::Memcpy(asset->CompiledStory.GetData(), data.c_str(), data.length());

		// Paths
		std::unordered_set<std::filesystem::path> visited{};
		TraversImports(*asset->AssetImportData, visited, story_path);

		// Not cancelled
		bOutOperationCanceled = false;

		// Return
		return asset;
	} catch (...) {
		// some kind of error?
		return nullptr;
	}

	return nullptr;
}

bool UInkAssetFactory::FactoryCanImport(const FString& Filename)
{
	const FString Extension = FPaths::GetExtension(Filename);
	return Extension == TEXT("json") || Extension == TEXT("ink");
}

bool UInkAssetFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
	// Can reimport story assets
	UInkAsset* InkAsset = Cast<UInkAsset>(Obj);
	if (InkAsset != nullptr && InkAsset->AssetImportData) {
		InkAsset->AssetImportData->ExtractFilenames(OutFilenames);
		return true;
	}
	return false;
}

void UInkAssetFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths)
{
	UE_LOG(InkCpp, Display, TEXT("SetReimportPaths"));
	UInkAsset* obj = Cast<UInkAsset>(Obj);
	if (obj && NewReimportPaths.Num() > 0) {
		for (size_t i = 0; i < NewReimportPaths.Num(); ++i) {
			obj->AssetImportData->UpdateFilenameOnly(NewReimportPaths[i], i);
		}
	}
}

int32 UInkAssetFactory::GetPriority() const { return ImportPriority; }

TObjectPtr<UObject>* UInkAssetFactory::GetFactoryObject() const
{
	return const_cast<TObjectPtr<UObject>*>(&object_ptr);
}

EReimportResult::Type UInkAssetFactory::Reimport(UObject* Obj)
{
	UE_LOG(InkCpp, Display, TEXT("Reimport started"));
	UInkAsset* InkAsset = Cast<UInkAsset>(Obj);
	if (! InkAsset || ! InkAsset->AssetImportData) {
		return EReimportResult::Failed;
	}

	const FString Filename = InkAsset->AssetImportData->GetFirstFilename();
	if (! Filename.Len() || IFileManager::Get().FileSize(*Filename) == INDEX_NONE) {
		return EReimportResult::Failed;
	}

	// Run the import again
	EReimportResult::Type Result      = EReimportResult::Failed;
	bool                  OutCanceled = false;

	if (ImportObject(
	        InkAsset->GetClass(), InkAsset->GetOuter(), *InkAsset->GetName(),
	        RF_Public | RF_Standalone, Filename, nullptr, OutCanceled
	    )
	    != nullptr) {
		/// TODO: add aditional pathes
		InkAsset->AssetImportData->Update(Filename);

		// Try to find the outer package so we can dirty it up
		if (InkAsset->GetOuter()) {
			InkAsset->GetOuter()->MarkPackageDirty();
		} else {
			InkAsset->MarkPackageDirty();
		}
		Result = EReimportResult::Succeeded;
	} else {
		Result = EReimportResult::Failed;
	}

	return Result;
}
