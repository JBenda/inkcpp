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
	for (const FString& tag : Tags)
	{
		if (tag.StartsWith(prefix))
		{
			FString value = tag.RightChop(prefix.Len()).TrimStartAndEnd();
			for (int i = 0; i < Enum->NumEnums(); ++i) {
				FString enumStr = Enum->GetDisplayNameTextByIndex(i).ToString();
				if (enumStr.EndsWith(value)) {
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

const TArray<FString>& UTagList::GetTags() const {
	return Tags;
}
