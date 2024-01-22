#pragma once

#include "InkSnapshot.generated.h"

/** A serelizable snapshot of a runtime state
 * Can be used as variable in a USaveGame to be stored and reloaded
 * @ingroup unreal
 */
USTRUCT(BlueprintType)
struct INKCPP_API FInkSnapshot
{
	GENERATED_BODY()
	FInkSnapshot() {}

	/** @private */
	FInkSnapshot(const char* snap_data, size_t snap_len)
	    : data(reinterpret_cast<const uint8*>(snap_data), snap_len)
	{}
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "ink|SaveGame")
	/** Raw data used to restore runtime state.
	 *  not needed if a USaveGame is used.
	 */
	TArray<uint8> data;
};
