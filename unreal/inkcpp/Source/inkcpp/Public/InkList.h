#pragma once

#include "InkList.generated.h"

namespace ink::runtime {
	class list_interface;
	using list = list_interface*;
}

USTRUCT(BlueprintType)
struct FListFlag {
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Ink")
	FString list_name;
	UPROPERTY(BlueprintReadOnly, Category = "Ink")
	FString flag_name;
};

/**
 * Allowes reading ink lists.
 */
UCLASS(Blueprintable, BlueprintType)
class INKCPP_API UInkList : public UObject
{
	GENERATED_BODY()

public:
	UInkList() {}
	UInkList(ink::runtime::list list) { list_data = list; }

	bool ContainsFlag(const FString& flag_name) const;

	UFUNCTION(BlueprintPure, Category = "Ink")
	/// returns true if a flag with the same spelling then the enum `value` is set in the list
	bool ContainsEnum(const FString& enumName, const uint8& value) const;

	UFUNCTION(BlueprintPure, Category = "Ink")
	/// returns all values of a list with the same name as the enum
	TArray<uint8> ElementsOf(const FString& enumName) const;

	UFUNCTION(BlueprintPure, Category = "Ink")
	/// returns all flag as string contained in the list from a list with the name `list_name`
	TArray<FString> ElementsOfAsString(const FString& list_name) const;

	UFUNCTION(BlueprintPure, Category = "Ink")
	/// returns all `list_name` `flag_name` toubples
	TArray<FListFlag> Elements() const;

	UFUNCTION(BlueprintPure, Category = "Ink")
	/// check if at least one value of the given list is included, OR the list is empty
	/// and assoziatet with the list
	bool ContainsList(const FString& name) const;
	
	
private:
	friend struct FInkVar;
	ink::runtime::list GetData() const { return list_data; }
	/// @TODO


	ink::runtime::list list_data;
};