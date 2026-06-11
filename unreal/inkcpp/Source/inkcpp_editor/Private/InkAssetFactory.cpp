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
#include "Internationalization/Regex.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SHyperlink.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SWindow.h"
#include "ISettingsModule.h"
#include "Modules/ModuleManager.h"
#include "HAL/PlatformProcess.h"

#include "InkAsset.h"
#include "InkCppEditorSettings.h"
#include "ink/compiler.h"
#include "inklecate_cmd.cpp"
#include "version.h"

#include <sstream>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <unordered_set>
#include <cstdio>

// -------------------------------------------------------------------------
// Platform detection helpers
// -------------------------------------------------------------------------

namespace
{

/** Returns the platform name used by UE (Windows / Mac / Linux). */
FString GetCurrentPlatform()
{
#if PLATFORM_WINDOWS
	return TEXT("Windows");
#elif PLATFORM_MAC
	return TEXT("Mac");
#else
	return TEXT("Linux");
#endif
}

/**
 * Returns the inklecate download URL for the host platform.
 * The tag is derived from the InkVersion constant so this stays in sync
 * automatically when version.h is updated.
 */
FString GetInklecateDownloadUrl()
{
	// Map the ink JSON version to the nearest known inklecate release tag.
	// inklecate releases use a "v<major>.<minor>.<patch>" scheme that does NOT
	// directly mirror the ink JSON version number; update this string whenever
	// a new compatible inklecate release is published.
	const FString tag = TEXT("v1.1.1");

	const FString platform = GetCurrentPlatform();
	if (platform == TEXT("Windows")) {
		return FString::Printf(
		    TEXT("https://github.com/inkle/ink/releases/download/%s/inklecate_windows.zip"), *tag
		);
	} else if (platform == TEXT("Mac")) {
		return FString::Printf(
		    TEXT("https://github.com/inkle/ink/releases/download/%s/inklecate_mac.zip"), *tag
		);
	} else {
		return FString::Printf(
		    TEXT("https://github.com/inkle/ink/releases/download/%s/inklecate_linux.zip"), *tag
		);
	}
}

/** Returns the recommended path to place inklecate inside the project Content folder. */
FString GetRecommendedInklecatePath()
{
	const FString platform = GetCurrentPlatform();
	const FString ext      = (platform == TEXT("Windows")) ? TEXT(".exe") : TEXT("");
	// FPaths::ProjectContentDir() already ends with a separator
	return FPaths::ProjectContentDir() / TEXT("inklecate") / (TEXT("inklecate") + ext);
}

// -------------------------------------------------------------------------
// Tutorial / setup dialog
// -------------------------------------------------------------------------

/** Opens a modal dialog explaining how to install inklecate and link it in settings. */
void ShowInklecateSetupDialog()
{
	const FString downloadUrl     = GetInklecateDownloadUrl();
	const FString recommendedPath = GetRecommendedInklecatePath();
	const FString platform        = GetCurrentPlatform();
	const FString zipName         = FPaths::GetCleanFilename(downloadUrl);
	const FString binaryName
	    = (platform == TEXT("Windows")) ? TEXT("inklecate.exe") : TEXT("inklecate");
	const FString contentSubFolder = TEXT("Content/inklecate/");

	const FText titleText = NSLOCTEXT("InkCpp", "SetupTitle", "InkCPP: Inklecate Setup Required");

	const FText bodyText = FText::Format(
	    NSLOCTEXT(
	        "InkCpp", "SetupBody",
	        "To import .ink story files directly, InkCPP needs the inklecate compiler.\n\n"
	        "Quick setup steps:\n"
	        "  1. Download {0} for {1} using the link below.\n"
	        "  2. Unzip the archive into your project's Content folder:\n"
	        "         {2}\n"
	        "  3. Open Project Settings > Plugins > InkCPP and set\n"
	        "     \"Inklecate Executable Path\" to:\n"
	        "         {3}\n\n"
	    ),
	    FText::FromString(zipName), FText::FromString(platform),
	    FText::FromString(contentSubFolder + binaryName), FText::FromString(recommendedPath)
	);

	TSharedPtr<SWindow> DialogPtr;
	TSharedRef<SWindow> Dialog = SNew(SWindow)
	                                 .Title(titleText)
	                                 .SizingRule(ESizingRule::Autosized)
	                                 .IsTopmostWindow(true)
	                                 .SupportsMaximize(false)
	                                 .SupportsMinimize(false);
	DialogPtr = Dialog;

	TSharedRef<SWidget> Content = SNew(SBorder).Padding(FMargin(16.f)
	)[SNew(SVerticalBox)
	  + SVerticalBox::Slot().AutoHeight().Padding(
	      0.f, 0.f, 0.f, 12.f
	  )[SNew(STextBlock).Text(bodyText).AutoWrapText(true)]
	  + SVerticalBox::Slot().AutoHeight().Padding(
	      0.f, 0.f, 0.f, 12.f
	  )[SNew(SHyperlink)
	        .Text(FText::Format(
	            NSLOCTEXT("InkCpp", "DownloadLinkLabel", "Download: {0}"),
	            FText::FromString(downloadUrl)
	        ))
	        .OnNavigate_Lambda([downloadUrl]() {
		        FPlatformProcess::LaunchURL(*downloadUrl, nullptr, nullptr);
	        })]
	  + SVerticalBox::Slot().AutoHeight().HAlign(HAlign_Left
	  )[SNew(SHorizontalBox)
	    + SHorizontalBox::Slot().AutoWidth().Padding(
	        0.f, 0.f, 8.f, 0.f
	    )[SNew(SButton)
	          .Text(NSLOCTEXT("InkCpp", "OpenSettingsBtn", "Open Project Settings"))
	          .OnClicked_Lambda([DialogPtr]() {
		          if (ISettingsModule* SettingsModule
		              = FModuleManager::GetModulePtr<ISettingsModule>("Settings")) {
			          SettingsModule->ShowViewer(TEXT("Project"), TEXT("Plugins"), TEXT("InkCpp"));
		          }
		          if (DialogPtr.IsValid()) {
			          DialogPtr->RequestDestroyWindow();
		          }
		          return FReply::Handled();
	          })]
	    + SHorizontalBox::Slot().AutoWidth()[SNew(SButton)
	                                             .Text(NSLOCTEXT("InkCpp", "CloseBtn", "Close"))
	                                             .OnClicked_Lambda([DialogPtr]() {
		                                             if (DialogPtr.IsValid()) {
			                                             DialogPtr->RequestDestroyWindow();
		                                             }
		                                             return FReply::Handled();
	                                             })]]];

	Dialog->SetContent(Content);
	FSlateApplication::Get().AddModalWindow(Dialog, nullptr, false);
}


} // anonymous namespace

// -------------------------------------------------------------------------
// Include traversal
// -------------------------------------------------------------------------

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
	int id = ( int ) visited.size();
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
		match_file_path.replace_filename(TCHAR_TO_UTF8(*matcher.GetCaptureGroup(2)));
		TraversImports(AssetImportData, visited, match_file_path);
	}
}

// -------------------------------------------------------------------------
// Factory implementation
// -------------------------------------------------------------------------

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
	SupportedClass     = UInkAsset::StaticClass();
	bCreateNew         = false;
	bAutomatedReimport = true;
	bForceShowDialog   = true;
	bEditorImport      = true;

	ImportPriority = 20;
}

UInkAssetFactory::~UInkAssetFactory() { FReimportManager::Instance()->UnregisterHandler(*this); }

UObject* UInkAssetFactory::FactoryCreateFile(
    UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, const FString& Filename,
    const TCHAR* Parms, FFeedbackContext* Warn, bool& bOutOperationCanceled
)
{
	std::stringstream        output;
	static const std::string ink_suffix{".ink"};

	try {
		using path            = std::filesystem::path;
		std::string cFilename = TCHAR_TO_UTF8(*Filename);
		path        story_path(cFilename, path::format::generic_format);
		story_path.make_preferred();
		bool use_temp_file = false;

		if (cFilename.size() > ink_suffix.size()
		    && std::equal(ink_suffix.rbegin(), ink_suffix.rend(), cFilename.rbegin())) {
			// ---- .ink file: needs inklecate ----
			std::string inklecate_cmd = get_inklecate_cmd();

			if (inklecate_cmd.empty()) {
				// No path configured → show setup tutorial and abort
				UE_LOG(
				    InkCpp, Warning,
				    TEXT("InkCPP: No inklecate path configured. "
				         "Set it in Project Settings > Plugins > InkCPP, "
				         "or import a .ink.json file directly.")
				);
				ShowInklecateSetupDialog();
				bOutOperationCanceled = true;
				return nullptr;
			}

			// Verify the binary actually exists on disk
			// remove quotation marks
			if ((inklecate_cmd.front() == '"' && inklecate_cmd.back() == '"')
			    || (inklecate_cmd.front() == '\'' && inklecate_cmd.back() == '\'')) {
				inklecate_cmd.erase(inklecate_cmd.begin());
				inklecate_cmd.erase(inklecate_cmd.end() - 1);
			}
			if (! std::filesystem::path(inklecate_cmd).is_absolute()) {
				inklecate_cmd = std::string(TCHAR_TO_UTF8(*FPaths::ProjectDir())) + inklecate_cmd;
			}
			if (! std::filesystem::exists(inklecate_cmd)) {
				UE_LOG(
				    InkCpp, Warning,
				    TEXT("InkCPP: inklecate not found at '%s'. "
				         "Update the path in Project Settings > Plugins > InkCPP."),
				    *FString(inklecate_cmd.c_str())
				);
				ShowInklecateSetupDialog();
				bOutOperationCanceled = true;
				return nullptr;
			}

			// Build the inklecate invocation
			use_temp_file = true;
			char tmp_filename[L_tmpnam];
			if (tmpnam(tmp_filename) == 0) {
				UE_LOG(InkCpp, Error, TEXT("InkCPP: Failed to create a temporary file name."));
				return nullptr;
			}
			cFilename = tmp_filename;
			path json_path(cFilename, path::format::generic_format);
			json_path.make_preferred();

			std::stringstream cmd{};
			// Note: on Windows, if std::system()'s argument begins with '"', the outer
			// pair of quotes is stripped, breaking paths with spaces. Emitting the first
			// character outside the quoted section works around this.
			cmd << inklecate_cmd[0] << "\"" << (inklecate_cmd.c_str() + 1) << "\"" << " -o \""
			    << json_path.string() << "\"" << " \"" << story_path.string() << "\" 2>&1";
			const std::string cmd_str = cmd.str();
			int               result  = std::system(cmd_str.c_str());
			if (result != 0) {
				UE_LOG(
				    InkCpp, Warning, TEXT("InkCPP: inklecate failed (exit code %i). Command: '%s'"), result,
				    *FString(cmd_str.c_str())
				);
				return nullptr;
			}
		}

		// ---- JSON → binary compilation (in-process) ----
		ink::compiler::run(cFilename.c_str(), output);
		if (use_temp_file) {
			std::filesystem::remove(cFilename);
		}

		// Create ink asset
		UInkAsset* asset = NewObject<UInkAsset>(InParent, InClass, InName, Flags);
		if (! asset->AssetImportData) {
			asset->AssetImportData = NewObject<UAssetImportData>(asset, UAssetImportData::StaticClass());
		}

		// Load compiled binary into the asset
		std::string data = output.str();
		asset->CompiledStory.SetNum(data.length());
		FMemory::Memcpy(asset->CompiledStory.GetData(), data.c_str(), data.length());

		// Record source file paths for reimport
		std::unordered_set<std::filesystem::path> visited{};
		TraversImports(*asset->AssetImportData, visited, story_path);

		bOutOperationCanceled = false;
		return asset;
	} catch (...) {
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
		for (int32 i = 0; i < NewReimportPaths.Num(); ++i) {
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

	EReimportResult::Type Result      = EReimportResult::Failed;
	bool                  OutCanceled = false;

	if (ImportObject(
	        InkAsset->GetClass(), InkAsset->GetOuter(), *InkAsset->GetName(),
	        RF_Public | RF_Standalone, Filename, nullptr, OutCanceled
	    )
	    != nullptr) {
		/// @todo add additional paths
		InkAsset->AssetImportData->Update(Filename);

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
