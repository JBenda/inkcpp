/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to 
 * https://github.com/JBenda/inkcpp for full license details. 
 */
#pragma once

#include "InkList.generated.h"

namespace ink::runtime {
	class list_interface;
	using list = list_interface*;
}

/** A single flag of a list.
 * @ingroup unreal
 */
USTRUCT(BlueprintType)
struct FListFlag {
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Ink")
	/** name of the list, the flag is part of */
	FString list_name;
	UPROPERTY(BlueprintReadOnly, Category = "Ink")
	/** name of the flag */
	FString flag_name;
};

/**
 * Allowes reading ink lists.
 * @ingroup unreal
 */
UCLASS(Blueprintable, BlueprintType)
class INKCPP_API UInkList : public UObject
{
	GENERATED_BODY()

public:
	/** @private */
	UInkList() {}

	/** @private */
	UInkList(ink::runtime::list list) { list_data = list; }

	/** @private */
	void SetList(ink::runtime::list list) { list_data = list; }

	UFUNCTION(BlueprintPure, Category = "Ink")
	/** Checks if a flag is contained (by name)
	 * @attention If the flag name is not unique please use the full flag name aka
	 * `list_name.flag_name`
	 *
	 * @blueprint
	 */
	bool ContainsFlag(const FString& flag_name) const;

	UFUNCTION(BlueprintPure, Category = "Ink")
	/** checks if flag with the same spelling then the enum `value` is set in the list
	 * @retval true if flag is contained in list
	 *
	 * @blueprint
	 */
	bool ContainsEnum(const UEnum* Enum, const uint8& value) const;

	UFUNCTION(BlueprintPure, Category = "Ink")
	/** returns all values of a list with the same name as the enum
	 *
	 * @blueprint
	 */
	TArray<uint8> ElementsOf(const UEnum* Enum) const;

	UFUNCTION(BlueprintPure, Category = "Ink")
	/** returns all flag as string contained in the list from a list with the same name as the Enum`
	 *
	 * @blueprint
	 */
	TArray<FString> ElementsOfAsString(const UEnum* Enum) const;

	UFUNCTION(BlueprintPure, Category = "Ink")
	/** returns all `list_name` `flag_name` tuples
	 *
	 * @blueprint
	 */
	TArray<FListFlag> Elements() const;

	UFUNCTION(BlueprintPure, Category = "Ink")
	/** check if at least one value of the given list is included, OR the list is empty
	 * and assoziatet with the list
	 *
	 * @blueprint
	 */
	bool ContainsList(const FString& ListName) const;


private:
	friend struct FInkVar;

	ink::runtime::list GetData() const { return list_data; }

	ink::runtime::list list_data;
};
