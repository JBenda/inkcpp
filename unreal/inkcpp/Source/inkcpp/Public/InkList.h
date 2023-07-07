#pragma once

#include "InkList.generated.h"

namespace ink::runtime {
	class list_interface;
	using list = list_interface*;
}

/**
 * Allowes reading and modifing ink lists.
 * @attention The 
 */
USTRUCT(BlueprintType)
struct INKCPP_API FInkList 
{
	GENERATED_BODY()

public:
	FInkList() {}
	FInkList(ink::runtime::list list) { list_data = list; }

	bool ContainsFlag(const FString& flag_name) const;

	bool ConstainsList(const FString& list_name) const;
	
	
private:
	friend struct FInkVar;
	ink::runtime::list GetData() const { return list_data; }
	/// @TODO


	ink::runtime::list list_data;
};