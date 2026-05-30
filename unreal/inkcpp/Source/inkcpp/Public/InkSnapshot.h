/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"

#include "InkSnapshot.generated.h"

/** A serializable snapshot of a runtime state
 * Can be used as variable in a USaveGame to be stored and reloaded
 * @ingroup unreal
 */
USTRUCT(BlueprintType)
struct INKCPP_API FInkSnapshot
{
	GENERATED_BODY()
	FInkSnapshot() : Migratable(false) {}

	/** @private */
	FInkSnapshot(const char* snap_data, size_t snap_len, bool migratable)
	    : data(reinterpret_cast<const uint8*>(snap_data), snap_len),
	      Migratable(migratable)
	{}
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "ink|SaveGame")
	/** Raw data used to restore runtime state.
	 *  not needed if a USaveGame is used.
	 */
	TArray<uint8> data;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "ink|SaveGame")
	/** Is true if the snapshot is migratable.
	 */
	bool Migratable;
};


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FInkMigratableSnapshotCompleted,
    const FInkSnapshot&, Snapshot
);

UCLASS(BlueprintType)
class INKCPP_API UInkMigratableSnapshotAsync : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:

    UPROPERTY(BlueprintAssignable)
    FInkMigratableSnapshotCompleted Completed;

    UFUNCTION(BlueprintCallable,
        meta = (BlueprintInternalUseOnly = "true"))
    static UInkMigratableSnapshotAsync* GetMigratableSnapshot(
        AInkRuntime* Runtime);

    virtual void Activate() override;

private:
    UPROPERTY()
    TObjectPtr<AInkRuntime> Runtime;

    void HandleResult(const FInkSnapshot& Snapshot);
};
