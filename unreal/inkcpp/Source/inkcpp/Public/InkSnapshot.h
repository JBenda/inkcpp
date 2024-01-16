#pragma once

#include "InkSnapshot.generated.h"

/** A serelizable snapshot of a runtime state
 * @ingroup unreal
 */
USTRUCT(BlueprintType)
struct INKCPP_API FInkSnapshot
{
	GENERATED_BODY()
	FInkSnapshot() {}
	FInkSnapshot(const char* snap_data, size_t snap_len)
	: data(reinterpret_cast<const uint8*>(snap_data), snap_len)
	{}
	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "ink|SaveGame")
	TArray<uint8> data;
};
