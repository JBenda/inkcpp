// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Misc/Optional.h"

#include "InkDelegates.h"
#include "InkSnapshot.h"

#include "ink/types.h"
#include "ink/globals.h"


#include "InkRuntime.generated.h"

class UInkThread;
struct FInkVar;
namespace ink::runtime { class story; }

UCLASS()
class INKCPP_API AInkRuntime : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInkRuntime();
	~AInkRuntime();

	/**
	* Create a new Thread. If a Snapshot is set/loaded create Threads like there was before
	* if you want to create a fresh Thread despite an existing LoadedSnapshot enter the starting path
	*/
	UFUNCTION(BlueprintCallable, Category="Ink")
	UInkThread* Start(TSubclassOf<UInkThread> type, FString path = "", bool runImmediately = true);

	/**
	* Create a new Thread in existing memory, for more details \see AInkRuntime::Start()
	*/
	UFUNCTION(BlueprintCallable, Category="Ink")
	UInkThread* StartExisting(UInkThread* thread, FString path = "", bool runImmediately = true);
	
	// only tested in choices moments
	UFUNCTION(BlueprintCallable, Category="Ink")
	FInkSnapshot Snapshot();
	
	/**
	* Loads a snapshot file, therfore deletes globals and invalidate all current Threads
	* After this Start and StartExisting will load the corresponding Threads (on at a time)
	*/
	UFUNCTION(BlueprintCallable, Category="Ink")
	void LoadSnapshot(const FInkSnapshot& snapshot);


	// Marks a thread as "exclusive". As long as it is running, no other threads will update.
	UFUNCTION(BlueprintCallable, Category="Ink")
	void PushExclusiveThread(UInkThread* Thread);

	// Removes a thread from the exclusive stack. See PushExclusiveThread.
	UFUNCTION(BlueprintCallable, Category="Ink")
	void PopExclusiveThread(UInkThread* Thread);
	
	UFUNCTION(BlueprintCallable, Category="Ink")
	void RegisterTagFunction(FName functionName, const FTagFunctionDelegate & function);
	
	// for interanl use
	void HandleTagFunction(UInkThread* Caller, const TArray<FString>& Params);
	
	UFUNCTION(BlueprintCallable, Category="Ink")
	FInkVar GetGlobalVariable(const FString& name);
	
	UFUNCTION(BlueprintCallable, Category="Ink")
	void SetGlobalVariable(const FString& name, const FInkVar& value);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Story asset used in this level
	UPROPERTY(EditAnywhere, Category="Ink")
	class UInkAsset* InkAsset;

private:
	ink::runtime::story* mpRuntime;
	ink::runtime::globals mpGlobals;

	UPROPERTY()
	TArray<UInkThread*> mThreads;
	
	TMap<FName, FGlobalTagFunctionMulticastDelegate> mGlobalTagFunctions;

	UPROPERTY()
	TArray<UInkThread*> mExclusiveStack;
	
	// UPROPERTY(EditDefaultsOnly, Category="Ink")
	TOptional<FInkSnapshot> mSnapshot;
	// snapshot generates data when re-constructing the globals to allow reconstructing the threads
	ink::runtime::snapshot* mpSnapshot;
};
