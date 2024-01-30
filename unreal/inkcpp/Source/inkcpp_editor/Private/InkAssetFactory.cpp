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

#include "InkAsset.h"
#include "ink/compiler.h"
#include "inklecate_cmd.cpp"

#include <sstream>
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <cstdio>
#include <cctype>
#include <cstring>

UInkAssetFactory::UInkAssetFactory(const FObjectInitializer& ObjectInitializer)
	: UFactory(ObjectInitializer), FReimportHandler(), object_ptr(this)
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

void find_includes(std::filesystem::path file_path, std::vector<std::filesystem::path>& files) {
	using path = std::filesystem::path;
	files.push_back(file_path);
	size_t curr_size = files.size();
	{
		path dir_path = file_path;
		dir_path = file_path.remove_filename();
		file_path.make_preferred();
		std::ifstream file(file_path);
		if (!file) {
			UE_LOG(InkCpp, Error, TEXT("Failed To inspect file '%s', maybe not all dependencies for the asset were listed"), ANSI_TO_TCHAR(file_path.string().c_str()));
			return;
		}
		
		std::string line;
		while(std::getline(file, line)) {
			size_t i = 0;
			// remove leading spaces
			while(std::isspace(static_cast<unsigned char>(line[i]))) { ++i; }
			// check for "INCLUDE"
			if(strncmp("INCLUDE", line.c_str() + i, 7) != 0) {
				continue;
			}
			i += 7;
			size_t end = i;
			// remove spaces (but at least one)
			while(std::isspace(static_cast<unsigned char>(line[i]))) { ++i; }
			if (end == i) { continue; }

			path new_file_path(line.c_str() + i);
			if (new_file_path.is_relative()) {
				new_file_path = dir_path / new_file_path;
			}
			files.push_back(new_file_path);
		}
	}
	size_t end = files.size();
	for(size_t i = curr_size; i < end; ++i) {
		find_includes(files[i], files);
	}
}

UObject* UInkAssetFactory::FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename, const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled)
{
	using path = std::filesystem::path;
	std::stringstream output;
	std::stringstream cmd{};
	const std::string        inklecate_cmd = get_inklecate_cmd();
	static const std::string ink_suffix{".ink"};
	std::vector<path> story_files;
	try
	{
		std::string cFilename = TCHAR_TO_ANSI(*Filename);
		bool use_temp_file = false;
		if (cFilename.size() > ink_suffix.size()
				&& std::equal(ink_suffix.rbegin(), ink_suffix.rend(), cFilename.rbegin()))
		{
			use_temp_file = true;
			if(inklecate_cmd.size() == 0) {
				UE_LOG(InkCpp, Warning, TEXT("Inklecate provided with the plugin, please import ink.json files"));
				return nullptr;
			}
			using path = std::filesystem::path;
			path path_bin(TCHAR_TO_ANSI(*IPluginManager::Get().FindPlugin(TEXT("InkCPP"))->GetBaseDir()), path::format::generic_format);
			path_bin.make_preferred();
			path_bin /= path(inklecate_cmd, path::format::generic_format).make_preferred();

			path story_path(cFilename, path::format::generic_format);
			find_includes(story_path, story_files);
			story_path.make_preferred();

			const char* filename = std::tmpnam(nullptr);
			if(filename == nullptr) {
				UE_LOG(InkCpp, Error, TEXT("Failed to create temporary file"));
				return nullptr;
			}
			cFilename = filename;
			path json_path(cFilename, path::format::generic_format);
			json_path.make_preferred();
			cmd 
				// if std::system start with a quote, the pair of qoute is removed, which leads to errors with pathes with spaces
				// but if the quote is not the first symbol it works fine (a"b" is glued to ab from bash)
				<< path_bin.string()[0] << "\"" << (path_bin.string().c_str() + 1) << "\""
				<< " -o \"" << json_path.string() << "\" "
				<< '"' << story_path.string() << "\" 2>&1";
			auto cmd_str = cmd.str();
			int result = std::system(cmd_str.c_str());
			if (result != 0) {
				UE_LOG(InkCpp, Warning, TEXT("Inklecate failed with exit code %i, executed was: '%s'"), result, *FString(cmd_str.c_str()));
				return nullptr;
			}
		}
		ink::compiler::run(cFilename.c_str(), output);
		if(use_temp_file) {
			std::filesystem::remove(cFilename);
		}

		// Create ink asset
		UInkAsset* asset = NewObject<UInkAsset>(InParent, InClass, InName, Flags);

		// Load it up
		std::string data = output.str();
		asset->CompiledStory.SetNum(data.length());
		FMemory::Memcpy(asset->CompiledStory.GetData(), data.c_str(), data.length());

		// Paths
		TArray<FAssetImportInfo::FSourceFile> source_files;
		for(auto& src_path : story_files) {
			src_path.make_preferred();
			source_files.Add(FAssetImportInfo::FSourceFile(
				asset->AssetImportData->SanitizeImportFilename(FString(ANSI_TO_TCHAR(src_path.string().c_str()))),
				FDateTime::UtcNow(),
				FMD5Hash(),
				FString(TEXT("Ink story file"))
			));
		}
		if (! source_files.Num()) {
			asset->AssetImportData->SetSourceFiles(std::move(source_files));
		}
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
	return Filename.EndsWith(TEXT(".ink.json"), ESearchCase::IgnoreCase)
			|| Filename.EndsWith(TEXT(".ink"), ESearchCase::IgnoreCase)
			|| Filename.EndsWith(TEXT(".json"), ESearchCase::IgnoreCase);
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

TObjectPtr<UObject>* UInkAssetFactory::GetFactoryObject() const
{
	return const_cast<TObjectPtr<UObject>*>(&object_ptr);
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
	// TODO: set to true?
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
