// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "InkVar.h"

#include "ink/runner.h"
#include "ink/types.h"

#include "InkThread.generated.h"

class UTagList;
class AInkRuntime;
class UChoice;

DECLARE_DYNAMIC_DELEGATE_OneParam(FTagFunctionDelegate, const TArray<FString>&, Params);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTagFunctionMulticastDelegate, const TArray<FString>&, Params);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FExternalFunctionDelegate, const TArray<FInkVar>&, Arguments, FInkVar&, Result);

/**
 * Base class for all ink threads
 */
UCLASS(Blueprintable)
class INKCPP_API UInkThread : public UObject
{
	GENERATED_BODY()

public:
	UInkThread();

	// Yields the thread immediately. Will wait until Resume().
	UFUNCTION(BlueprintCallable, Category="Ink")
	void Yield();

	UFUNCTION(BlueprintPure, Category="Ink")
	bool IsYielding();

	// Causes the thread to resume if yielded.
	UFUNCTION(BlueprintCallable, Category="Ink")
	void Resume();

	// Kills the thread, regardless of state
	UFUNCTION(BlueprintCallable, Category="Ink")
	void Stop();

	// Returns the runtime which owns this thread.
	UFUNCTION(BlueprintPure, Category="Ink")
	AInkRuntime* GetRuntime() const { return mpRuntime; }

	// Called before the thread begins executing
	UFUNCTION(BlueprintImplementableEvent , Category="Ink")
	void OnStartup();

	// Called when the thread has printed a new line
	UFUNCTION(BlueprintImplementableEvent , Category="Ink")
	void OnLineWritten(const FString& line, const UTagList& tags);

	// Called when a tag has been processed on the current line
	UFUNCTION(BlueprintImplementableEvent , Category="Ink")
	void OnTag(const FString& line);

	// Called when the thread has requested a branch
	UFUNCTION(BlueprintImplementableEvent , Category="Ink")
	void OnChoice(const TArray<UChoice*>& choices);

	// Called before the thread is destroyed
	UFUNCTION(BlueprintImplementableEvent , Category="Ink")
	void OnShutdown();

	// Picks a choice by index at the current branch
	UFUNCTION(BlueprintCallable, Category="Action")
	void PickChoice(int index);

	// Registers a callback for a named "tag function"
	UFUNCTION(BlueprintCallable, Category="Setup")
	void RegisterTagFunction(FName functionName, const FTagFunctionDelegate& function);

	UFUNCTION(BlueprintCallable, Category="Setup")
	void RegisterExternalFunction(const FString& functionName, const FExternalFunctionDelegate& function);

protected:
	virtual void OnStartup_Implementation() { }
	virtual void OnLineWritten_Implementation(const FString& line, UTagList* tags) { }
	virtual void OnTag_Implementation(const FString& line) { }
	virtual void OnChoice_Implementation(const TArray<UChoice*>& choices) { }
	virtual void OnShutdown_Implementation() { }
	
private:
	friend class AInkRuntime;

	void Initialize(FString path, AInkRuntime* runtime, ink::runtime::runner thread);
	bool Execute();
	bool CanExecute() const;

	bool ExecuteInternal();

	void ExecuteTagMethod(const TArray<FString>& Params);

private:
	ink::runtime::runner mpRunner;
	UTagList mTags;
	TArray<UChoice*> mCurrentChoices; /// @TODO: make accassible?

	FString mStartPath;
	bool mbHasRun;

	int mnChoiceToChoose;
	int mnYieldCounter;
	bool mbInChoice;
	bool mbKill;
	bool mbInitialized;

	UPROPERTY()
	AInkRuntime* mpRuntime;
};
