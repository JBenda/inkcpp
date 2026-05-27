/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "InkList.h"
#include "inkcpp.h"
#include <string>
#include "ink/list.h"

bool UInkList::ContainsFlag(const FString& flag_name) const
{
	if (! ensureMsgf(IsValid(), TEXT("UInkList::ContainsFlag called on an invalid (stale) list")))
		return false;
	return list_data->contains(TCHAR_TO_UTF8(*flag_name));
}

bool UInkList::ContainsEnum(const UEnum* Enum, const uint8& value) const
{
	if (! ensureMsgf(IsValid(), TEXT("UInkList::ContainsEnum called on an invalid (stale) list")))
		return false;
	if (! Enum) {
		UE_LOG(
		    InkCpp, Warning,
		    TEXT("No Enum provided for ContainsEnum; therefore ContainsEnum has failed!")
		);
		return false;
	}

	return list_data->contains(TCHAR_TO_UTF8(*Enum->GetDisplayNameTextByValue(value).ToString()));
}

TArray<uint8> UInkList::ElementsOf(const UEnum* Enum) const
{
	TArray<uint8> ret;
	if (! ensureMsgf(IsValid(), TEXT("UInkList::ElementsOf called on an invalid (stale) list")))
		return ret;
	if (! Enum) {
		UE_LOG(InkCpp, Warning, TEXT("Failed to provide enum for elements of!"));
		return ret;
	}
	FString enumName = Enum->GetFName().ToString();

	int         num = Enum->NumEnums();
	std::string str(TCHAR_TO_UTF8(*enumName));
	for (auto itr = list_data->begin(str.c_str()); itr != list_data->end(); ++itr) {
		bool          hit = false;
		const FString flag(UTF8_TO_TCHAR((*itr).flag_name));
		for (int i = 0; i < num; ++i) {
			FString enumStr = Enum->GetDisplayNameTextByIndex(i).ToString();
			if (enumStr.EndsWith(flag)) {
				ret.Add(Enum->GetValueByIndex(i));
				hit = true;
				break;
			}
		}
		if (! hit) {
			UE_LOG(
			    InkCpp, Warning, TEXT("Failed to find list value '%s' in enum '%s'"), *flag, *enumName
			);
		}
	}

	return ret;
}

TArray<FString> UInkList::ElementsOfAsString(const UEnum* Enum) const
{
	TArray<FString> ret;
	if (! ensureMsgf(
	        IsValid(), TEXT("UInkList::ElementsOfAsString called on an invalid (stale) list")
	    ))
		return ret;

	FString EnumName = Enum->GetFName().ToString();
	for (auto itr = list_data->begin(TCHAR_TO_UTF8(*EnumName)); itr != list_data->end(); ++itr) {
		ret.Add(FString((*itr).flag_name));
	}
	return ret;
}

TArray<FListFlag> UInkList::Elements() const
{
	TArray<FListFlag> ret;
	if (! ensureMsgf(IsValid(), TEXT("UInkList::Elements called on an invalid (stale) list")))
		return ret;
	for (auto itr = list_data->begin(); itr != list_data->end(); ++itr) {
		ret.Add(FListFlag{
		    .list_name = FString((*itr).list_name),
		    .flag_name = FString((*itr).flag_name),
		});
	}
	return ret;
}

bool UInkList::ContainsList(const FString& name) const
{
	if (! ensureMsgf(IsValid(), TEXT("UInkList::ContainsList called on an invalid (stale) list")))
		return false;
	return list_data->begin(TCHAR_TO_UTF8(*name)) != list_data->end();
}
