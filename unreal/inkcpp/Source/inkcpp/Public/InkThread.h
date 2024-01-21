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
	/** Yields the thread immediatly.
	 * This will stop the execution (after finishing the current line).
	 * until @ref #Resume() is called.
	 * 
	 * @ref #Yield() and @ref #Resume() working with a refernce counter.
	 * therfore a thread can be yield multiple times, and must then be resumed
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
	/** triggered after initalizing the runner
	 *
	 * @blueprint
	 */
	void OnStartup();

	// Called when the thread has printed a new line
	UFUNCTION(BlueprintImplementableEvent, Category = "Ink")
	/** triggerd if a new line of context is available
	 * @param line text of new line
	 * @param tags tags assoziated with this line
	 *
	 * @blueprint
	 */
	void OnLineWritten(const FString& line, const UTagList* tags);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ink")
	/** triggered when a tag is encountered
	 * @param tag_name the tag found
	 *
	 * @blueprint
	 */
	void OnTag(const FString& tag_name);

	UFUNCTION(BlueprintImplementableEvent, Category = "Ink")
	/** triggered when reached a choice point.
	 * @param choices possible branches to choos for continue
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
	/** Register a callback for a named "tag function"
		* @see @ref TagFunction
		*
		* @blueprint
		*/
	void RegisterTagFunction(FName functionName, const FTagFunctionDelegate& function);

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** register a external function.
	 * A function provides a return value
	 * @see if you do not want to return something #RegisterExternalEvent()
	 *
	 * @blueprint
	 */
	void RegisterExternalFunction(
	    const FString& functionName, const FExternalFunctionDelegate& function,
	    bool lookaheadSafe = false
	);

	UFUNCTION(BlueprintCallable, Category = "Ink")
	/** register external event.
	 * A event has the return type void.
	 * @see  If you want to return a value use #RegisterExternalFunction()
	 *
	 * @blueprint
	 */
	void RegisterExternalEvent(
	    const FString& functionName, const FExternalFunctionVoidDelegate& function,
	    bool lookaheadSafe = false
	);


protected:
	/** @private */
	virtual void OnStartup_Implementation() {}

	/** @private */
	virtual void OnLineWritten_Implementation(const FString& line, UTagList* tags) {}

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

private:
	ink::runtime::runner mpRunner;
	UTagList*            mpTags;
	TArray<UInkChoice*>  mCurrentChoices; /// @TODO: make accassible?

	TMap<FName, FTagFunctionMulticastDelegate> mTagFunctions;

	FString mStartPath;
	bool    mbHasRun;

	int  mnChoiceToChoose;
	int  mnYieldCounter;
	bool mbInChoice;
	bool mbKill;
	bool mbInitialized;

	UPROPERTY()
	AInkRuntime* mpRuntime;
};
