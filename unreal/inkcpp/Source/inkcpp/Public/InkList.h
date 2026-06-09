/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "CoreMinimal.h"

#include "InkList.generated.h"

namespace ink::runtime
{
class list_interface;
using list = list_interface*;
} // namespace ink::runtime

/** A single flag of a list.
 * @ingroup unreal
 */
USTRUCT(BlueprintType)

struct INKCPP_API FListFlag {
	GENERATED_BODY()
	UPROPERTY(BlueprintReadOnly, Category = "Ink")
	/** name of the list, the flag is part of */
	FString list_name;
	UPROPERTY(BlueprintReadOnly, Category = "Ink")
	/** name of the flag */
	FString flag_name;
};

/**
 * Allows reading ink lists.
 *
 * A UInkList wraps a pointer into runner-owned memory. It is only valid until
 * the runner advances (i.e. until the next getline() or choose()). After that
 * @ref IsValid() returns false and all accessor methods become no-ops that
 * return empty/default values. Always check @ref IsValid() before using a list
 * that was not obtained in the same Blueprint event.
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
	UInkList(ink::runtime::list list)
	    : list_data(list)
	{
	}

	/** @private */
	void SetList(ink::runtime::list list)
	{
		list_data = list;
		bValid    = true;
	}

	/** @private — called by UInkThread before advancing the runner */
	void Invalidate() { bValid = false; }

	UFUNCTION(BlueprintPure, Category = "Ink")

	/**
	 * Returns whether this list object still points to valid runner memory.
	 * A list becomes invalid once the runner advances past the line or variable
	 * read that produced it. Do not call any other methods on an invalid list.
	 *
	 * @blueprint
	 */
	bool IsValid() const { return bValid && list_data != nullptr; }

	UFUNCTION(BlueprintPure, Category = "Ink")
	/** Checks if a flag is contained (by name)
	 * @attention If the flag name is not unique please use the full flag name aka
	 * `list_name.flag_name`
	 * @retval false if the list is no longer valid
	 *
	 * @blueprint
	 */
	bool ContainsFlag(const FString& flag_name) const;

	UFUNCTION(BlueprintPure, Category = "Ink")
	/** checks if flag with the same spelling then the enum `value` is set in the list
	 * @retval true if flag is contained in list
	 * @retval false if the list is no longer valid
	 *
	 * @blueprint
	 */
	bool ContainsEnum(const UEnum* Enum, const uint8& value) const;

	UFUNCTION(BlueprintPure, Category = "Ink")
	/** returns all values of a list with the same name as the enum
	 * @retval empty array if the list is no longer valid
	 *
	 * @blueprint
	 */
	TArray<uint8> ElementsOf(const UEnum* Enum) const;

	UFUNCTION(BlueprintPure, Category = "Ink")
	/** returns all flag as string contained in the list from a list with the same name as the Enum
	 * @retval empty array if the list is no longer valid
	 *
	 * @blueprint
	 */
	TArray<FString> ElementsOfAsString(const UEnum* Enum) const;

	UFUNCTION(BlueprintPure, Category = "Ink")
	/** returns all `list_name` `flag_name` tuples
	 * @retval empty array if the list is no longer valid
	 *
	 * @blueprint
	 */
	TArray<FListFlag> Elements() const;

	UFUNCTION(BlueprintPure, Category = "Ink")
	/** check if at least one value of the given list is included, OR the list is empty
	 * and associated with the list
	 * @retval false if the list is no longer valid
	 *
	 * @blueprint
	 */
	bool ContainsList(const FString& ListName) const;


private:
	friend struct FInkVar;

	ink::runtime::list GetData() const { return list_data; }

	ink::runtime::list list_data = nullptr;
	bool               bValid    = true;
};
