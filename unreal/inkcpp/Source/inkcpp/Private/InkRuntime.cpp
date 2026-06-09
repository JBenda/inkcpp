/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "InkRuntime.h"

#include "Async/Async.h"

// Game includes
#include "inkcpp.h"
#include "InkThread.h"
#include "TagList.h"
#include "InkAsset.h"
#include "InkVar.h"

// inkcpp includes
#include "ink/story.h"
#include "ink/globals.h"
#include "ink/snapshot.h"
#include "system.h"
#include "types.h"
#include <functional>

namespace ink
{
using value = runtime::value;
} // namespace ink

// Sets default values
AInkRuntime::AInkRuntime()
    : mpRuntime(nullptr)
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you
	// don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

AInkRuntime::~AInkRuntime()
{
	if (mStableSnapshot.IsValid()) {
		mStableSnapshot->SetValue(FInkSnapshot());
		mStableSnapshot.Reset();
	}
}

// Called when the game starts or when spawned
void AInkRuntime::BeginPlay()
{
	// Create the CPU for the story
	if (InkAsset != nullptr) {
		// TODO: Better error handling? What if load fails.
		mpRuntime = ink::runtime::story::from_binary(
		    reinterpret_cast<unsigned char*>(InkAsset->CompiledStory.GetData()),
		    InkAsset->CompiledStory.Num(), false
		);
		UE_LOG(InkCpp, Display, TEXT("Loaded Ink asset"));

		// create globals
		if (mSnapshot) {
			LoadSnapshot(*mSnapshot);
		} else {
			mpGlobals = mpRuntime->new_globals();
		}
	} else {
		UE_LOG(InkCpp, Warning, TEXT("No story asset assigned."));
	}

	Super::BeginPlay();
}

void AInkRuntime::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	mThreads.Empty();
	mExclusiveStack.Empty();

	mpGlobals = ink::runtime::globals{};

	if (mpSnapshot) {
		delete mpSnapshot;
		mpSnapshot = nullptr;
	}
	mSnapshot.Reset();

	if (mpRuntime) {
		delete mpRuntime;
		mpRuntime = nullptr;
	}
}

// Called every frame
void AInkRuntime::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Early out: no threads.
	if (mThreads.Num() == 0)
		return;

	// Special: If we're in exclusive mode, only execute the thread at the
	//  top of the exclusive stack
	while (mExclusiveStack.Num() > 0) {
		// Make sure top thread is active
		UInkThread* top = mExclusiveStack.Top();

		// If it can't execute, we're not doing anything else
		if (! top->CanExecute())
			return;

		// Execute it
		if (top->Execute()) {
			mExclusiveStack.Remove(top);
			mThreads.Remove(top);

			// execute next exclusive thread
			continue;
		}

		// Nothing more to do
		return;
	}

	// Execute other available threads
	for (auto iter = mThreads.CreateIterator(); iter; iter++) {
		UInkThread* pNextThread = *iter;

		// Ignore threads that aren't eligible for execution
		if (! pNextThread->CanExecute())
			continue;

		// Execute
		if (pNextThread->Execute()) {
			// If the thread has finished, destroy it
			iter.RemoveCurrent();
			mExclusiveStack.Remove(pNextThread);
		}
	}
}

FInkHandle
    AInkRuntime::RegisterTagFunction(FName functionName, const FTagFunctionDelegate& function)
{
	TSharedPtr<bool> token = MakeShared<bool>(true);
	mTagFunctionTokens.FindOrAdd(functionName).Add(token);
	mTagFunctionDelegates.FindOrAdd(functionName).Add(function);
	mObserverTokens.Add(token);
	return FInkHandle(token);
}

void AInkRuntime::HandleTagFunction(UInkThread* Caller, const TArray<FString>& Params)
{
	FName name(*Params[0]);
	auto* tokens    = mTagFunctionTokens.Find(name);
	auto* delegates = mTagFunctionDelegates.Find(name);
	if (! tokens || ! delegates)
		return;

	// Fire active delegates, compact dead entries lazily
	for (int32 i = tokens->Num() - 1; i >= 0; --i) {
		if ((*tokens)[i].IsValid() && *(*tokens)[i]) {
			(*delegates)[i].ExecuteIfBound(Caller, Params);
		} else {
			tokens->RemoveAtSwap(i);
			delegates->RemoveAtSwap(i);
		}
	}
}

UInkThread*
    AInkRuntime::Start(TSubclassOf<class UInkThread> type, FString path, bool startImmediately)
{
	UE_LOG(InkCpp, Display, TEXT("Start"));
	if (mpRuntime == nullptr || type == nullptr) {
		UE_LOG(InkCpp, Warning, TEXT("failed to start"));
		return nullptr;
	}

	// Instantiate the thread
	UInkThread* pThread = NewObject<UInkThread>(this, type);

	// Startup
	return StartExisting(pThread, path, startImmediately);
}

FInkSnapshot AInkRuntime::Snapshot()
{
	ink::runtime::snapshot* inkSnapshot = mpGlobals->create_snapshot();
	FInkSnapshot            snapshot(
      reinterpret_cast<const char*>(inkSnapshot->get_data()), inkSnapshot->get_data_len(),
      inkSnapshot->can_be_migrated()
  );
	delete inkSnapshot;
	return snapshot;
}

TFuture<FInkSnapshot> AInkRuntime::MigratableSnapshot()
{
	// Fast path: already stable
	FInkSnapshot snapshot = Snapshot();

	if (snapshot.Migratable) {
		TPromise<FInkSnapshot> Immediate;
		Immediate.SetValue(snapshot);
		return Immediate.GetFuture();
	}

	// Slow path: wait for stability
	TSharedRef<TPromise<FInkSnapshot>, ESPMode::ThreadSafe> Promise
	    = MakeShared<TPromise<FInkSnapshot>, ESPMode::ThreadSafe>();

	mStableSnapshot = Promise;

	return Promise->GetFuture();
}

void AInkRuntime::RunnerEnterStableState(UInkThread* thread)
{
	if (mStableSnapshot.IsValid()) {
		thread->Yield();
		mYieldedThreadsForSnapshot.Add(thread);
		FInkSnapshot snapshot = Snapshot();
		if (snapshot.Migratable) {
			mStableSnapshot->SetValue(snapshot);
			mStableSnapshot.Reset();
			for (auto& _thread : mYieldedThreadsForSnapshot) {
				_thread->Resume();
			}
			mYieldedThreadsForSnapshot.Empty();
		}
	}
}

void AInkRuntime::LoadSnapshot(const FInkSnapshot& snapshot)
{
	if (mpSnapshot) {
		delete mpSnapshot;
		mpSnapshot = nullptr;
	}
	mSnapshot  = snapshot;
	mpSnapshot = ink::runtime::snapshot::from_binary(
	    reinterpret_cast<unsigned char*>(mSnapshot->data.GetData()), mSnapshot->data.Num(), false
	);
	mpGlobals = mpRuntime->new_globals_from_snapshot(*mpSnapshot);
	if (! mpGlobals.is_valid()) {
		UE_LOG(InkCpp, Error, TEXT("Failed to load snapshot."));
		if (! mpSnapshot->can_be_migrated()) {
			UE_LOG(
			    InkCpp, Error,
			    TEXT("Unable to load snapshot. The story has changed and the snapshot was taken at in "
			         "instable moment.")
			);
		}
	}
}

UInkThread*
    AInkRuntime::StartExisting(UInkThread* thread, FString path, bool startImmediately /*= true*/)
{
	if (mpRuntime == nullptr) {
		UE_LOG(InkCpp, Warning, TEXT("Failed to start existing"));
		return nullptr;
	}

	if (! mpGlobals.is_valid()) {
		if (mSnapshot) {
			UE_LOG(
			    InkCpp, Error,
			    TEXT("Failed to start existing, due to invalid state after failed snapshot loading.")
			);
		} else {
			UE_LOG(InkCpp, Warning, TEXT("Failed to start existing"));
		}
		return nullptr;
	}

	// remove handle if it still exists
	mThreads.Remove(thread);
	mExclusiveStack.Remove(thread);

	// Initialize thread with new runner
	ink::runtime::runner runner;
	if (mSnapshot && path.IsEmpty()) {
		if (mpSnapshot->num_runners() == mThreads.Num()) {
			UE_LOG(
			    InkCpp, Warning,
			    TEXT("Already created all Threads from Snapshot!, will not create more. You can Still "
			         "create new Threads with entering the starting Path.")
			);
			return nullptr;
		}
		runner = mpRuntime->new_runner_from_snapshot(*mpSnapshot, mpGlobals, mThreads.Num());
	} else {
		runner = mpRuntime->new_runner(mpGlobals);
	}
	thread->Initialize(path, this, runner);

	// If we're not starting immediately, just queue
	if (! startImmediately ||
	    // Even if we want to start immediately, don't if there's an exclusive thread and it's not
	    // us
	    (mExclusiveStack.Num() > 0 && mExclusiveStack.Top() != thread)) {
		mThreads.Add(thread);
		return thread;
	}

	// Execute the newly created thread
	if (! thread->Execute()) {
		// If it hasn't finished immediately, add it to the threads list
		mThreads.Add(thread);
	}

	return thread;
}

void AInkRuntime::PushExclusiveThread(UInkThread* Thread)
{
	// If we're already on the stack, ignore
	if (mExclusiveStack.Find(Thread) != INDEX_NONE)
		return;

	// Push
	mExclusiveStack.Push(Thread);
}

void AInkRuntime::PopExclusiveThread(UInkThread* Thread)
{
	// Remove from the stack
	mExclusiveStack.Remove(Thread);
}

FInkVar AInkRuntime::GetGlobalVariable(const FString& name)
{
	ink::optional<ink::value> var = mpGlobals->get<ink::value>(TCHAR_TO_UTF8(*name));
	if (var) {
		return FInkVar(*var);
	} else {
		UE_LOG(InkCpp, Warning, TEXT("Failed to find global variable with name: %s"), *name);
	}
	return FInkVar{};
}

void AInkRuntime::SetGlobalVariable(const FString& name, const FInkVar& value)
{
	bool success = mpGlobals->set<ink::value>(TCHAR_TO_UTF8(*name), value.to_value());
	if (! success) {
		UE_LOG(InkCpp, Warning, TEXT("Filed to set global variable with name: %s"), *name);
		ink::optional<ink::value> var = mpGlobals->get<ink::value>(TCHAR_TO_UTF8(*name));
		if (var) {
			UE_LOG(
			    InkCpp, Warning, TEXT("Reason: wrong type!, got: %i, expected: %i"),
			    static_cast<int>(value.to_value().type), static_cast<int>(var->type)
			);
		} else {
			UE_LOG(InkCpp, Warning, TEXT("Reason: no variable with this name exists! '%s'"), *name);
		}
	}
}

FInkHandle
    AInkRuntime::ObserverVariable(const FString& name, const FVariableCallbackDelegate& callback)
{
	TSharedPtr<bool> token = MakeShared<bool>(true);
	mObserverTokens.Add(token);
	// Capture token by value; if it is set to false the callback is skipped.
	// Use TWeakObjectPtr for the bound UObject inside the delegate to avoid
	// keeping it alive longer than the GC would otherwise.
	mpGlobals->observe(TCHAR_TO_UTF8(*name), [token, callback]() {
		if (token.IsValid() && *token) {
			callback.ExecuteIfBound();
		}
	});
	return FInkHandle(token);
}

FInkHandle AInkRuntime::ObserverVariableEvent(
    const FString& name, const FVariableCallbackDelegateNewValue& callback
)
{
	TSharedPtr<bool> token = MakeShared<bool>(true);
	mObserverTokens.Add(token);
	mpGlobals->observe(TCHAR_TO_UTF8(*name), [token, callback](ink::runtime::value x) {
		if (token.IsValid() && *token) {
			callback.ExecuteIfBound(FInkVar(x));
		}
	});
	return FInkHandle(token);
}

FInkHandle AInkRuntime::ObserverVariableChange(
    const FString& name, const FVariableCallbackDelegateNewOldValue& callback
)
{
	TSharedPtr<bool> token = MakeShared<bool>(true);
	mObserverTokens.Add(token);
	mpGlobals->observe(
	    TCHAR_TO_UTF8(*name),
	    [token, callback](ink::runtime::value x, ink::optional<ink::runtime::value> y) {
		    if (token.IsValid() && *token) {
			    callback.ExecuteIfBound(FInkVar(x), y.has_value() ? FInkVar(y.value()) : FInkVar());
		    }
	    }
	);
	return FInkHandle(token);
}
