// Fill out your copyright notice in the Description page of Project Settings.

#include "InkThread.h"

#include "inkcpp.h"
#include "InkRuntime.h"
#include "TagList.h"
#include "Choice.h"
#include "ink/runner.h"

// Unreal includes
#include "Internationalization/Regex.h"

UInkThread::UInkThread() : mbHasRun(false), mnChoiceToChoose(-1), mnYieldCounter(0), mbKill(false) { }

void UInkThread::Yield()
{
	mnYieldCounter++;
}

bool UInkThread::IsYielding()
{
	return mnYieldCounter > 0;
}

void UInkThread::Resume()
{
	mnYieldCounter--;
}

void UInkThread::RegisterTagFunction(FName functionName, const FTagFunctionDelegate & function)
{
	// Register tag function
	mTagFunctions.FindOrAdd(functionName).Add(function);
}

void UInkThread::RegisterExternalFunction(const FString& functionName, const FExternalFunctionDelegate& function)
{
	mpRunner->bind_delegate(ink::hash_string(TCHAR_TO_ANSI (*functionName)), function);
	/*// Notify the runtime. It needs to make sure the external function is registered with Ink
	mpRuntime->ExternalFunctionRegistered(functionName);
	
	// Add to external functions registration map
	// TODO: Check for duplicates?
	mExternalFunctions.Add(functionName, function);*/
}

void UInkThread::Initialize(FString path, AInkRuntime* runtime, ink::runtime::runner thread)
{
	if (!ensureMsgf(!mbInitialized, TEXT("Thread already initialized!")))
	{
		return;
	}

	mStartPath = path;
	mpRuntime = runtime;
	mbInitialized = true;
	mpRunner = thread;

	OnStartup();
}

bool UInkThread::ExecuteInternal()
{
	// Kill thread
	if (mbKill)
		return true;

	// If this is the first time we're running, start us off at our starting path
	if (!mbHasRun)
	{
		if (!ensureMsgf(!mStartPath.IsEmpty(), TEXT("Thread executing without call to ::Initialize")))
			return true;

		//pStory->ChoosePathString(mStartPath, true, TArray<FInkVar>());
		// TODO: Move to correct story path
		mbHasRun = true;
	}

	// Execution loop
	while (true)
	{
		// Handle pending choice
		if (mnChoiceToChoose != -1)
		{
			if (ensure(mpRunner->num_choices() > 0))
			{
				mpRunner->choose(mnChoiceToChoose);
			}
			mnChoiceToChoose = -1;
		}

		// Execute until story yields or finishes
		while (mnYieldCounter == 0 && mpRunner->can_continue())
		{
			// Handle text
			FString line = mpRunner->getline();

			// Get tags
			// TODO: Tags
			TArray<FString> tags;// = pStory->CurrentTags();

			// Special: Line begins with >> marker
			if (line.StartsWith(TEXT(">>")))
			{
				// This is a special version of the tag function call
				// Expected: >> MyTagFunction(Arg1, Arg2, Arg3)
				FRegexPattern pattern = FRegexPattern(TEXT("^>>\\s*(\\w+)\\s*(\\((([\\w ]+)(,\\s*([\\w ]+))*)?\\))?$"));
				FRegexMatcher matcher = FRegexMatcher(pattern, line);
				if (matcher.FindNext())
				{
					// Get name of function
					FString functionName = matcher.GetCaptureGroup(1);

					// Get arguments
					TArray<FString> Arguments;
					Arguments.Add(functionName);
					int groupIndex = 4;
					while (matcher.GetCaptureGroupBeginning(groupIndex) != -1)
					{
						Arguments.Add(matcher.GetCaptureGroup(groupIndex).TrimStartAndEnd());
						groupIndex += 2;
					}

					// Call tag method
					ExecuteTagMethod(Arguments);
				}
			}
			else
			{
				// Forward to handler
				UTagList* pTags = NewObject<UTagList>(this, UTagList::StaticClass());
				pTags->Initialize(tags);
				OnLineWritten(line, pTags);
			}

			// Handle tags/tag methods post-line
			for (auto it = tags.CreateConstIterator(); it; ++it)
			{
				// Generic tag handler
				OnTag(*it);

				// Tag methods
				TArray<FString> params;
				it->ParseIntoArray(params, TEXT("_"));
				ExecuteTagMethod(params);
			}
		}

		// Handle choice block
		if (mnYieldCounter == 0 && mpRunner->num_choices() > 0)
		{
			mbInChoice = true;

			// TODO: Record choices somewhere?

			// Forward to handler
			TArray<UChoice*> choices;
			for (ink::size_t i = 0; i < mpRunner->num_choices(); i++)
			{
				UChoice* choice = NewObject<UChoice>(this);
				choice->Initialize(mpRunner->get_choice(i));
				choices.Add(choice);
			}
			OnChoice(choices);

			// If we've chosen a choice already, go back up to handle it.
			if (mnChoiceToChoose != -1)
				continue;
		}

		break;
	}

	// Have we reached the end? If so, destroy the thread
	if (!mpRunner->can_continue() && mnYieldCounter == 0 && mpRunner->num_choices() == 0)
	{
		UE_LOG(InkCpp, Display, TEXT("Destroying thread"));

		// TODO: Event for ending?

		// TODO: Destroy ourselves?
		return true;
	}

	// There's more to go. Return false to put us on the waiting list.
	return false;
}

/*bool UInkThread::HandleExternalFunction(const FString& functionName, TArray<FInkVar> arguments, FInkVar& result)
{
	// Check to see if we even know about this function
	const FExternalFunctionDelegate* pDelegate = mExternalFunctions.Find(functionName);
	if (pDelegate == nullptr)
		return false;

	// If so, execute and return
	pDelegate->Execute(arguments, result);
	return true;
}*/

void UInkThread::ExecuteTagMethod(const TArray<FString>& Params)
{
	// Look for method and execute with parameters
	FTagFunctionMulticastDelegate* function = mTagFunctions.Find(FName(*Params[0]));
	if (function != nullptr)
	{
#define GET_PARAM(n) Params.Num() > n ? Params[n] : FString()
		function->Broadcast(GET_PARAM(1), GET_PARAM(2), GET_PARAM(3));
#undef GET_PARAM
	}

	// Forward to runtime
	mpRuntime->HandleTagFunction(this, Params);
}

bool UInkThread::Execute()
{
	// Execute thread
	bool finished = ExecuteInternal();

	// If we've finished, run callback
	if (finished)
	{
		// Allow outsiders to subscribe
		OnThreadShutdown.Broadcast();
		OnShutdown();
	}

	// Return result
	return finished;
}

void UInkThread::PickChoice(int index)
{
	mnChoiceToChoose = index;
	mbInChoice = false;
}

bool UInkThread::CanExecute() const
{
	return mnYieldCounter == 0 && !mbInChoice;
}

void UInkThread::Stop()
{
	mbKill = true;
}