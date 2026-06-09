/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/Optional.h"
#include "Async/Future.h"

#include "InkDelegates.h"
#include "InkSnapshot.h"
#include "InkHandles.h"

#include "ink/types.h"
#include "ink/globals.h"


#include "InkRuntime.generated.h"

class UInkThread;
struct FInkVar;

namespace ink::runtime
{
class story;
} // namespace ink::runtime

/** Instantiated story with global variable storage and access, used to instantiate new threads.
 * @ingroup unreal
 */
UCLASS()

class INKCPP_API AInkRuntime : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInkRuntime();
	~AInkRuntime();

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/**
	 * Create a new Thread. If a Snapshot is set/loaded create Threads like there was before
	 * if you want to create a fresh Thread despite an existing LoadedSnapshot enter the starting path
	 *
	 * @blueprint
	 */
	UInkThread* Start(TSubclassOf<UInkThread> type, FString path = "", bool runImmediately = true);

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/**
	 * Create a new Thread in existing memory, for more details \see AInkRuntime::Start()
	 *
	 * @blueprint
	 */
	UInkThread* StartExisting(UInkThread* thread, FString path = "", bool runImmediately = true);

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** creates a snapshot of the current runtime state.
	 * can be loaded with @ref #LoadSnapshot()
	 *
	 * @attention this snapshot can only be loaded with the same story, for migratable snapshots
	 * please use UInkMigratableSnapshotAsync::UInkMigratableSnapshotAsync()
	 *
	 * make snapshot, if save? Return
	 * every time a thread chooses something
	 *   yield this thread
	 *   try to make snapshot, if save? fullfill promise and resume all threads
	 *
	 * @blueprint
	 */
	FInkSnapshot Snapshot();

	/** creates a snapshot the next time the story is in a stable state.
	 * for Blueprints please use snapshotAsync::UInkMigratableSnapshotAsync()
	 * This snapshot can be loaded with a new version of the same story.
	 * can be loaded with @ref #LoadSnapshot()
	 *
	 * @attention typical this snapshot will be created after the next choice is taken.
	 * To archive this each active runner will yield after the next choice and only continue after the
	 * snapshot is taken.
	 *
	 */
	TFuture<FInkSnapshot> MigratableSnapshot();

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/**
	 * Loads a snapshot file, therefore deletes globals and invalidate all current Threads
	 * After this Start and StartExisting will load the corresponding Threads (on at a time)
	 *
	 * @blueprint
	 */
	void LoadSnapshot(const FInkSnapshot& snapshot);


	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** Marks a thread as "exclusive".
	 * As long as it is running, no other threads will update.
	 * @see #PopExclusiveThread()
	 *
	 * @blueprint
	 */
	void PushExclusiveThread(UInkThread* Thread);

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** Removes a thread from the exclusive stack.
	 * @see #PushExclusiveThread()
	 *
	 * @blueprint
	 */
	void PopExclusiveThread(UInkThread* Thread);

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** register a "tag function"
	 * This function is executed if context or a tag in a special format appears.
	 * @return handle — call Cancel() to remove this binding (or pass to Unregister() from Blueprint)
	 * @see @ref TagFunction
	 *
	 * @blueprint
	 */
	FInkHandle RegisterTagFunction(FName functionName, const FTagFunctionDelegate& function);

	UFUNCTION(BlueprintCallable, Category = "Ink")

	/** Stop receiving variable-change notifications or unregister a tag function.
	 * Prefer calling @ref FInkHandle::Cancel() directly — that does not require the runtime.
	 * @param handle the handle returned by ObserverVariable / ObserverVariableEvent /
	 * ObserverVariableChange / RegisterTagFunction
	 *
	 * @blueprint
	 */
	void Unregister(const FInkHandle& handle) { handle.Cancel(); }

	/** @private for internal use */
	void HandleTagFunction(UInkThread* Caller, const TArray<FString>& Params);

	/** @private for internal use */
	void RunnerEnterStableState(UInkThread* thread);

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** Access a variable from the ink runtime.
	 * variables are shared between all threads in the same runtime.
	 * @param name of variable in ink script
	 *
	 * @blueprint
	 */
	FInkVar GetGlobalVariable(const FString& name);

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** Sets a global variable inside the ink runtime.
	 * @param name of variable in ink script
	 * @param value new value for the variable
	 *
	 * @blueprint
	 */
	void SetGlobalVariable(const FString& name, const FInkVar& value);

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** Gets a ping if variable changes.
	 * @return handle — call Cancel() to remove this binding (or pass to Unregister() from Blueprint)
	 * @see #ObserverVariableEvent() #ObserverVariableChange()
	 *
	 * @blueprint
	 */
	FInkHandle
	    ObserverVariable(const FString& variableName, const FVariableCallbackDelegate& callback);

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** On variable change provides new value.
	 * @return handle — call Cancel() to remove this binding (or pass to Unregister() from Blueprint)
	 * @see #ObserverVariable() #ObserverVariableChange()
	 *
	 * @blueprint
	 */
	FInkHandle ObserverVariableEvent(
	    const FString& variableName, const FVariableCallbackDelegateNewValue& callback
	);

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** On variable change provides old and new value.
	 * @return handle — call Cancel() to remove this binding (or pass to Unregister() from Blueprint)
	 * @attention if the variable set for the first time, the old value has value type @ref
	 * EInkVarType::None
	 * @see #ObserverVariableEvent() #ObserverVariable()
	 *
	 * @blueprint
	 */
	FInkHandle ObserverVariableChange(
	    const FString& variableName, const FVariableCallbackDelegateNewOldValue& callback
	);

protected:
	/** Called when the game starts or when spawned */
	virtual void BeginPlay() override;

	/** Called when the actor is being removed from play */
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	/** @private */
	virtual void Tick(float DeltaTime) override;

	// Story asset used in this level
	UPROPERTY(EditAnywhere, Category = "Ink")
	/** @private */
	class UInkAsset* InkAsset;

private:
	ink::runtime::story*  mpRuntime;
	ink::runtime::globals mpGlobals;

	UPROPERTY()
	TArray<UInkThread*> mThreads;

	/** Token storage for tag function registrations. Maps function name → parallel arrays of
	 *  tokens and delegates. Setting a token to false skips and lazily removes that entry. */
	TMap<FName, TArray<TSharedPtr<bool>>>     mTagFunctionTokens;
	TMap<FName, TArray<FTagFunctionDelegate>> mTagFunctionDelegates;

	UPROPERTY()
	TArray<UInkThread*> mExclusiveStack;

	// snapshot generates data when re-constructing the globals to allow reconstructing the threads
	TOptional<FInkSnapshot> mSnapshot;
	ink::runtime::snapshot* mpSnapshot = nullptr;

	/** Active observer tokens. When Cancel() is called on the handle the token is set to false,
	 *  the lambda checks it before firing and skips. Tokens are cleaned up lazily. */
	TArray<TSharedPtr<bool>>           mObserverTokens;
	TSharedPtr<TPromise<FInkSnapshot>> mStableSnapshot;
	UPROPERTY()
	TArray<UInkThread*> mYieldedThreadsForSnapshot;
};
