/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "CoreMinimal.h"
#include "InkRuntime.h"
#include "UObject/NoExportTypes.h"

#include "InkVar.h"
#include "InkDelegates.h"
#include "InkList.h"
#include "InkHandles.h"


#include "ink/runner.h"
#include "ink/types.h"

#include "InkThread.generated.h"

class UTagList;
class AInkRuntime;
class UInkChoice;

/** Base class for all ink threads
 * @ingroup unreal
 */
UCLASS(Blueprintable)

class INKCPP_API UInkThread : public UObject
{
	GENERATED_BODY()

public:
	UInkThread();
	~UInkThread();

	// Yields the thread immediately. Will wait until Resume().
	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** Yields the thread immediately.
	 * This will stop the execution (after finishing the current line).
	 * until @ref #Resume() is called.
	 *
	 * @ref #Yield() and @ref #Resume() working with a reference counter.
	 * therefore a thread can be yield multiple times, and must then be resumed
	 * the same amount.
	 *
	 * @blueprint
	 */
	void Yield();

	UFUNCTION(BlueprintPure, Category = "Ink")
	/** Checks if the thread is stopped.
	 * @retval true if the thread currently waiting to resume
	 * @see #Yield() #Resume()
	 *
	 * @blueprint
	 */
	bool IsYielding();

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** Resumes yielded thread.
	 * @see #Yield()
	 *
	 * @blueprint
	 */
	void Resume();

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** Kills thread at next possible moment
	 *
	 * @blueprint
	 */
	void Stop();

	UFUNCTION(BlueprintPure, Category = "Ink")

	/** Access runtime the thread belongs to
	 * @return runtime of the thread
	 *
	 * @blueprint
	 */
	AInkRuntime* GetRuntime() const { return mpRuntime; }

	// Called before the thread begins executing
	UFUNCTION(BlueprintImplementableEvent, Category = "Ink")
	/** triggered after initializing the runner
	 *
	 * @blueprint
	 */
	void OnStartup();

	// Called when the thread has printed a new line
	UFUNCTION(BlueprintImplementableEvent, Category = "Ink")
	/** triggered if a new line of context is available
	 * @param line text of new line
	 * @param tags tags associated with this line
	 *
	 * @blueprint
	 */
	void OnLineWritten(const FString& line, const UTagList* tags);

	// Called when a new knot/stitch is entered (tunnels are ignored)
	UFUNCTION(BlueprintImplementableEvent, Category = "Ink")
	/** triggered if a knew knot/stitch is entered (tunneling is ignored).
	 * Triggers before the first line of a knot/stitch is written
	 * @param global_tags tags assoziated with global file
	 * @param knot_tags tags assoziated with the current knot/stitch
	 *
	 * @blueprint
	 */
	void OnKnotEntered(const UTagList* global_tags, const UTagList* knot_tags);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ink")
	/** triggered when a tag is encountered
	 * @param tag_name the tag found
	 *
	 * @blueprint
	 */
	void OnTag(const FString& tag_name);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ink")
	/** triggered when reached a choice point.
	 * @param choices possible branches to choose from, in order to continue
	 * @see #PickChoice()
	 *
	 * @blueprint
	 */
	void OnChoice(const TArray<UInkChoice*>& choices);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ink")
	/** triggered when the thread reached the end of context
	 * @see AInkRuntime::StartExisting()
	 *
	 * @blueprint
	 */
	void OnShutdown();

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** picks a choice to continue with
	 * @see UInkChoice::GetIndex()
	 * @retval false if the index is out of range
	 *
	 * @blueprint
	 */
	bool PickChoice(int index);

	// Registers a callback for a named "tag function"
	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** Register a callback for a named "tag function".
	 * @return handle — call Cancel() to remove this binding (or pass to Unregister() from Blueprint)
	 *
	 * @blueprint
	 */
	FInkHandle RegisterTagFunction(FName functionName, const FTagFunctionDelegate& function);

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** Register an external function that returns a value.
	 * @return handle — call Cancel() to remove this binding (or pass to Unregister() from Blueprint)
	 * @see if you do not want to return something #RegisterExternalEvent()
	 *
	 * @blueprint
	 */
	FInkHandle RegisterExternalFunction(
	    const FString& functionName, const FExternalFunctionDelegate& function,
	    bool lookaheadSafe = false
	);

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** Register an external event (void return).
	 * @return handle — call Cancel() to remove this binding (or pass to Unregister() from Blueprint)
	 * @see If you want to return a value use #RegisterExternalFunction()
	 *
	 * @blueprint
	 */
	FInkHandle RegisterExternalEvent(
	    const FString& functionName, const FExternalFunctionVoidDelegate& function,
	    bool lookaheadSafe = false
	);

	UFUNCTION(BlueprintCallable, Category = "Ink")

	/** Unregister a previously registered external function, event, or tag function.
	 * Prefer calling @ref FInkHandle::Cancel() directly — that does not require the thread.
	 * @param handle the handle returned by RegisterExternalFunction() / RegisterExternalEvent() /
	 * RegisterTagFunction()
	 *
	 * @blueprint
	 */
	void Unregister(const FInkHandle& handle) { handle.Cancel(); }

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** Unregister all external functions and events bound to this thread.
	 * Useful when reusing a thread via StartExisting() to ensure no stale bindings remain.
	 *
	 * @blueprint
	 */
	void ClearExternalFunctions();

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** get knots assoziated with current knot.
	 * knot tags are tags listed behind a knot `== knot name ==` before the first line of content
	 *
	 * @blueprint
	 */
	const UTagList* GetKnotTags();

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** get global tags.
	 * global tags are tags listed at the top of the file before the first line of content
	 *
	 * @blueprint
	 */
	const UTagList* GetGlobalTags();

	/** Get choices from the last OnChoice event.
	 * @return array of choices available at the current choice point, empty if not at a choice
	 */
	const TArray<UInkChoice*>& GetCurrentChoices() const { return mCurrentChoices; }


protected:
	/** @private */
	virtual void OnStartup_Implementation() {}

	/** @private */
	virtual void OnLineWritten_Implementation(const FString& line, UTagList* tags) {}

	/** @private */
	virtual void OnKnotEntered_Implementation(const UTagList* global_tags, const UTagList* knot_tags)
	{
	}

	/** @private */
	virtual void OnTag_Implementation(const FString& line) {}

	/** @private */
	virtual void OnChoice_Implementation(const TArray<UInkChoice*>& choices) {}

	/** @private */
	virtual void OnShutdown_Implementation() {}

private:
	friend class AInkRuntime;

	void Initialize(FString path, AInkRuntime* runtime, ink::runtime::runner thread);
	bool Execute();
	bool CanExecute() const;

	bool ExecuteInternal();

	void ExecuteTagMethod(const TArray<FString>& Params);

	/** Register a UInkList that was created from runner-owned memory during this thread's
	 * execution. The thread will call Invalidate() on all registered lists before the next
	 * choose() call so that stale pointers are detected early.
	 * @private
	 */
	void RegisterLiveList(UInkList* list);
	friend FInkVar::FInkVar(UInkList&), FInkVar::FInkVar(ink::runtime::value);

	/** Invalidate all UInkList objects registered since the last choose().
	 * Called internally before mpRunner->choose().
	 * @private
	 */
	void InvalidateLiveLists();

private:
	ink::runtime::runner mpRunner;

	UPROPERTY()
	UTagList* mpTags;

	UPROPERTY()
	UTagList* mkTags = nullptr;

	UPROPERTY()
	UTagList* mgTags = nullptr;

	UPROPERTY()
	TArray<UInkChoice*> mCurrentChoices;

	/** Lists wrapping runner-owned memory, registered during current execute cycle. */
	TArray<TWeakObjectPtr<UInkList>> mLiveLists;

	/** Token storage for tag function registrations. Maps function name → parallel arrays of
	 *  tokens and delegates. Each token is shared with the corresponding FInkHandle.
	 *  Setting a token to false causes the entry to be skipped and lazily compacted. */
	TMap<FName, TArray<TSharedPtr<bool>>> mTagFunctionTokens;

	/** Multicast delegates for tag functions, parallel to mTagFunctionTokens. */
	TMap<FName, TArray<FTagFunctionDelegate>> mTagFunctionDelegates;

	/** Token storage for external function registrations, keyed by name hash.
	 *  A matching FInkHandle invalidates the token to suppress the binding. */
	TMap<uint32, TSharedPtr<bool>> mExternalFunctionTokens;

	FString mStartPath;
	bool    mbHasRun;

	int         mnChoiceToChoose;
	int         mnYieldCounter;
	bool        mbInChoice;
	bool        mbKill;
	bool        mbInitialized;
	ink::hash_t mCurrentKnot;

	UPROPERTY()
	AInkRuntime* mpRuntime;
};
