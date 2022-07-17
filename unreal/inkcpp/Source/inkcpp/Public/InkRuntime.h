// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InkDelegates.h"
#include "ink/types.h"
#include "ink/globals.h"

#include "InkRuntime.generated.h"

class UInkThread;
namespace ink::runtime { class story; }

UCLASS()
class INKCPP_API AInkRuntime : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInkRuntime();
	~AInkRuntime();

	UFUNCTION(BlueprintCallable, Category="Start")
	UInkThread* Start(TSubclassOf<UInkThread> type, FString path, bool runImmediately = true);

	UFUNCTION(BlueprintCallable, Category="Start")
	UInkThread* StartExisting(UInkThread* thread, FString path, bool runImmediately = true);

	UFUNCTION(BlueprintCallable, Category="Tags")
	void RegisterGlobalTagFunction(FName FunctionName, const FGlobalTagFunctionDelegate& Function);

	// Called from UInkThread
	void HandleTagFunction(UInkThread* Caller, const TArray<FString>& Params);

	// Marks a thread as "exclusive". As long as it is running, no other threads will update.
	UFUNCTION(BlueprintCallable, Category="Exclusive Thread")
	void PushExclusiveThread(UInkThread* Thread);

	// Removes a thread from the exclusive stack. See PushExclusiveThread.
	UFUNCTION(BlueprintCallable, Category="Exclusive Thread")
	void PopExclusiveThread(UInkThread* Thread);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Story asset used in this level
	UPROPERTY(EditAnywhere, Category="Put in correct category")
	class UInkAsset* InkAsset;

	// Called by threads when they want to register an external function
	void ExternalFunctionRegistered(FString functionName);

	/* Returns the tags at the specified knot */
	UFUNCTION(BlueprintPure, Category="Tags")
	UTagList* GetTagsAtPath(FString Path);

private:
	// UFUNCTION()
	// FInkVar ExternalFunctionHandler(FString functionName, TArray<FInkVar> arguments);

private:
	ink::runtime::story* mpRuntime;
	ink::runtime::globals mpGlobals;

	UPROPERTY()
	TArray<UInkThread*> mThreads;

	/*UPROPERTY()
	UInkThread* mpCurrentThread;*/

	UPROPERTY()
	TSet<FString> mRegisteredFunctions;

	UPROPERTY()
	TMap<FName, FGlobalTagFunctionMulticastDelegate> mGlobalTagFunctions;

	UPROPERTY()
	TArray<UInkThread*> mExclusiveStack;
};
