// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TagList.generated.h"

/**
 * Helpful tag list
 */
UCLASS()
class INKCPP_API UTagList : public UObject
{
	GENERATED_BODY()

public:
	UTagList();

	/** Initializes the tag list from an array*/
	void Initialize(const TArray<FString>& tags);

	UFUNCTION(BlueprintPure, Category = "Ink")
	/** Checks if the tag is present in the array */
	bool Has(const FString& tag) const;

	UFUNCTION(BlueprintPure, Category = "Ink")
	bool HasEnum(const FString& enumName, uint8& value) const;

	/** Searches for a tag that begins with 'name:' and returns the string after the colon */
	UFUNCTION(BlueprintPure, Category = "Ink")
	FString GetValue(const FString& name) const;

private:
	TArray<FString> Tags;
};