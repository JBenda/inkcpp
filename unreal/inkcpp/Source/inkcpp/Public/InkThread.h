// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"

#include "InkVar.h"
#include "InkDelegates.h"


#include "ink/runner.h"
#include "ink/types.h"

#include "InkThread.generated.h"

class UTagList;
class AInkRuntime;
class UChoice;

/**
 * Base class for all ink threads
 */
UCLASS(Blueprintable)
class INKCPP_API UInkThread : public UObject
{
	GENERATED_BODY()

public:
	UInkThread();
	~UInkThread();

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
	void OnLineWritten(const FString& line, const UTagList* tags);

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
	UFUNCTION(BlueprintCallable, Category="Ink")
	bool PickChoice(int index);

	// Registers a callback for a named "tag function"
	UFUNCTION(BlueprintCallable, Category="Ink")
	void RegisterTagFunction(FName functionName, const FTagFunctionDelegate& function);

	UFUNCTION(BlueprintCallable, Category="Ink")
	void RegisterExternalFunction(const FString& functionName, const FExternalFunctionDelegate& function);
	
	UFUNCTION(BlueprintCallable, Category="Ink")
	void RegisterExternalEvent(const FString& functionName, const FExternalFunctionVoidDelegate& function);
	

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
	UTagList* mpTags;
	TArray<UChoice*> mCurrentChoices; /// @TODO: make accassible?

	TMap<FName, FTagFunctionMulticastDelegate> mTagFunctions;
	
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
