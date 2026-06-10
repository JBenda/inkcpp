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

struct INKCPP_API FInkSnapshot {
	GENERATED_BODY()

	FInkSnapshot()
	    : Migratable(false)
	{
	}

	/** @private */
	FInkSnapshot(const char* snap_data, size_t snap_len, bool migratable)
	    : data(reinterpret_cast<const uint8*>(snap_data), snap_len)
	    , Migratable(migratable)
	{
	}
	UPROPERTY(BlueprintReadWrite, SaveGame, Category = "Ink|SaveGame")
	/** Raw data used to restore runtime state.
	 *  not needed if a USaveGame is used.
	 */
	TArray<uint8> data;

	UPROPERTY(BlueprintReadOnly, SaveGame, Category = "Ink|SaveGame")
	/** Is true if the snapshot is migratable.
	 */
	bool Migratable;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(
    FInkMigratableSnapshotCompleted, const FInkSnapshot&, Snapshot
);

/** A helper class to create migratable snapshots.
 * creating an instance with ::GetMigratableSnapshot() will @ref UInkThread::Yield() "yield" each
 * assoziated thread after the next choice until a migratable snapshot can be cerated. all threads
 * will then be @ref UInkThread::Resume() "resumed".
 * @attention if a thread is inside a tunnel it will still yield after a choice and will then stop
 * at an point where it cannot create a valid migratable snapshot, fix still pending.
 * @ingroup unreal
 */
UCLASS(BlueprintType)

class INKCPP_API UInkMigratableSnapshotAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/** @private */
	UPROPERTY(BlueprintAssignable)
	FInkMigratableSnapshotCompleted Completed;

	UFUNCTION(BlueprintCallable, meta = (BlueprintInternalUseOnly = "true"))
	/** Tries to create a migratable snapshot, on completion returns it.
	 * see @ref UInkMigratableSnapshotAsync for more details.
	 *
	 * @blueprint{ActionBase, Snapshot}
	 */
	static UInkMigratableSnapshotAsync* GetMigratableSnapshot(AInkRuntime* Runtime);

	/** @private */
	virtual void Activate() override;

private:
	UPROPERTY()
	TObjectPtr<AInkRuntime> Runtime;

	void HandleResult(const FInkSnapshot& Snapshot);
};
