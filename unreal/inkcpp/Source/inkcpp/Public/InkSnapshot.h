#pragma once

#include "InkSnapshot.generated.h"

USTRUCT(BlueprintType)
struct INKCPP_API FInkSnapshot
{
	GENERATED_BODY()
	FInkSnapshot() {}
	FInkSnapshot(const char* snap_data, size_t snap_len)
	: data(snap_data, snap_len)
	{}
	TArray<char> data;
};