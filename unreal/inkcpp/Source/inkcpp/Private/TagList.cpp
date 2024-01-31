/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "TagList.h"

UTagList::UTagList()
{
}

void UTagList::Initialize(const TArray<FString>& tags)
{
	// Copy construct
	Tags = TArray<FString>(tags);
}

bool UTagList::Has(const FString& tag) const
{
	for (const FString& t : Tags)
	{
		if (t.Equals(tag))
			return true;
	}

	return false;
}

bool UTagList::HasEnum(const UEnum* Enum, uint8& value) const
{
	if (!Enum)
	{
		value = 0;
		return false;
	}

	// Iterate tags
	int num = Enum->NumEnums();
	for (const FString& tag : Tags)
	{
		// Iterate enum values
		for (int i = 0; i < num; i++)
		{
			FString enumStr = Enum->GetDisplayNameTextByIndex(i).ToString();

			// Instead of string comparison, check if they share a suffix (most enums have weird prefixes)
			if (enumStr.EndsWith(tag))
			{
				value = Enum->GetValueByIndex(i);
				return true;
			}
		}
	}

	value = 0;
	return false;
}

FString UTagList::GetValue(const FString& name) const
{
	for (const FString& tag : Tags)
	{
		if (tag.StartsWith(name + ":"))
		{
			return tag.RightChop(name.Len() + 1).TrimStartAndEnd();
		}
	}

	return FString();
}

bool UTagList::GetEnum(const UEnum* Enum, uint8& value) const
{
	FString prefix = Enum->GetFName().ToString() + ":";
	for (const FString& tag : Tags) {
		if (tag.StartsWith(prefix)) {
			FString tag_value = tag.RightChop(prefix.Len()).TrimStartAndEnd();
			for (int i = 0; i < Enum->NumEnums(); ++i) {
				FString enumStr = Enum->GetDisplayNameTextByIndex(i).ToString();
				if (enumStr.EndsWith(tag_value)) {
					value = Enum->GetValueByIndex(i);
					return true;
				}
			}

			value = 0;
			return false;
		}
	}
	value = 0;
	return false;
}

const TArray<FString>& UTagList::GetTags() const { return Tags; }
