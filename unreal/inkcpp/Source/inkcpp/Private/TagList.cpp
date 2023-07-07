// Fill out your copyright notice in the Description page of Project Settings.

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

bool UTagList::HasEnum(const FString& enumName, uint8& value) const
{
	UEnum* Enum = FindObjectSafe<UEnum>(nullptr, *enumName, true);
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
			FString enumStr = Enum->GetNameStringByIndex(i);

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
