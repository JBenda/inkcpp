// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TagList.generated.h"

/**
 * Helpful tag list
 * @ingroup unreal
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
	bool HasEnum(const UEnum* Eum, uint8& value) const;

	UFUNCTION(BlueprintPure, Category = "Ink")
	/** Searches for a tag that begins with 'name:' and returns the string after the colon 
	  * @retval "" if no value was found, or if there are nothing behind ':'
	  */
	FString GetValue(const FString& name) const;
	
	UFUNCTION(BlueprintPure, Category = "Ink")
	/** Searches for a tag thats begins with 'EnumName:' and returns the enum value corresponding to the text after the ':' 
	  * @attention If the enum appears multiple times, the first instance will be used
		* @retval true if a tag in the form "EnumName:FlagName" was found 
	  */
	bool GetEnum(const UEnum* Enum, uint8& value) const;

	UFUNCTION(BlueprintPure, Category = "Ink")
	const TArray<FString>& GetTags() const;

private:
	TArray<FString> Tags;
};
