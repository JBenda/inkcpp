// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InkDelegates.h"
#include "ink/types.h"
#include "ink/globals.h"

#include "InkRuntime.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FGlobalTagFunctionDelegate, const TArray<FString>&, Params);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGlobalTagFunctionMulticastDelegate, const TArray<FString>&, Params);
class UInkThread;
class FInkVar;
namespace ink::runtime { class story; }

UCLASS()
class INKCPP_API AInkRuntime : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInkRuntime();
	~AInkRuntime();

	UFUNCTION(BlueprintCallable, Category="Ink")
	UInkThread* Start(TSubclassOf<UInkThread> type, FString path, bool runImmediately = true);

	UFUNCTION(BlueprintCallable, Category="Ink")
	UInkThread* StartExisting(UInkThread* thread, FString path, bool runImmediately = true);

	// Marks a thread as "exclusive". As long as it is running, no other threads will update.
	UFUNCTION(BlueprintCallable, Category="Ink")
	void PushExclusiveThread(UInkThread* Thread);

	// Removes a thread from the exclusive stack. See PushExclusiveThread.
	UFUNCTION(BlueprintCallable, Category="Ink")
	void PopExclusiveThread(UInkThread* Thread);
	
	UFUNCTION(BlueprintCallable, Category="Ink")
	void RegisterTagFunction(FName functionName, const FTagFunctionDelegate & function);
	
	UFUNCTION(BlueprintCallable, Category="Ink")
	void GetGlobalVariable(FName name, FInkVar& value) const;
	
	UFUNCTION(BlueprintCallable, Category="Ink")
	void SetGlobalVariable(FName name, const FInkVar& value);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Story asset used in this level
	UPROPERTY(EditAnywhere, Category="Put in correct category")
	class UInkAsset* InkAsset;

private:
	ink::runtime::story* mpRuntime;
	ink::runtime::globals mpGlobals;

	UPROPERTY()
	TArray<UInkThread*> mThreads;
	
	TMap<FName, FGlobalTagFunctionMulticastDelegate> mGlobalTagFunctions;

	UPROPERTY()
	TArray<UInkThread*> mExclusiveStack;
};
