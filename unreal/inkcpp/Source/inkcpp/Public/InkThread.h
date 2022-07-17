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

DECLARE_DYNAMIC_DELEGATE_ThreeParams(FTagFunctionDelegate, FString, FirstParameter, FString, SecondParameter, FString, ThirdParameter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FTagFunctionMulticastDelegate, FString, FirstParameter, FString, SecondParameter, FString, ThirdParameter);

DECLARE_DYNAMIC_DELEGATE_TwoParams(FExternalFunctionDelegate, const TArray<FInkVar>&, Arguments, FInkVar&, Result);

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FThreadShutdownDelegate);

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
	UFUNCTION(BlueprintCallable, Category="Controle")
	void Yield();

	UFUNCTION(BlueprintPure, Category="Controle")
	bool IsYielding();

	// Causes the thread to resume if yielded.
	UFUNCTION(BlueprintCallable, Category="Controle")
	void Resume();

	// Kills the thread, regardless of state
	UFUNCTION(BlueprintCallable, Category="Controle")
	void Stop();

	// Returns the runtime which owns this thread.
	UFUNCTION(BlueprintPure, Category="Setup")
	AInkRuntime* GetRuntime() const { return mpRuntime; }

	// Called before the thread begins executing
	UFUNCTION(BlueprintNativeEvent, Category="Events")
	void OnStartup();

	// Called when the thread has printed a new line
	UFUNCTION(BlueprintNativeEvent, Category="Events")
	void OnLineWritten(const FString& line, UTagList* tags);

	// Called when a tag has been processed on the current line
	UFUNCTION(BlueprintNativeEvent, Category="Events")
	void OnTag(const FString& line);

	// Called when the thread has requested a branch
	UFUNCTION(BlueprintNativeEvent, Category="Events")
	void OnChoice(const TArray<UChoice*>& choices);

	// Called before the thread is destroyed
	UFUNCTION(BlueprintNativeEvent, Category="Events")
	void OnShutdown();

	UPROPERTY(BlueprintAssignable, Category="Events")
	FThreadShutdownDelegate OnThreadShutdown;

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

	//bool HandleExternalFunction(const FString& functionName, TArray<FInkVar> arguments, FInkVar& result);

	void ExecuteTagMethod(const TArray<FString>& Params);

private:
	ink::runtime::runner mpRunner;

	FString mStartPath;
	bool mbHasRun;

	int mnChoiceToChoose;
	int mnYieldCounter;
	bool mbInChoice;
	bool mbKill;
	bool mbInitialized;

	UPROPERTY()
	AInkRuntime* mpRuntime;

	UPROPERTY()
	TMap<FName, FTagFunctionMulticastDelegate> mTagFunctions;

	/*UPROPERTY()
	TMap<FString, FExternalFunctionDelegate> mExternalFunctions;*/
};
