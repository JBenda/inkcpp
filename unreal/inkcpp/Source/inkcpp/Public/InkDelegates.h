#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"

#include "InkDelegates.generated.h"

DECLARE_DYNAMIC_DELEGATE_FourParams(FGlobalTagFunctionDelegate, UInkThread*, Caller, const FString&, FirstParameter, const FString&, SecondParameter, const FString&, ThirdParameter);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FGlobalTagFunctionMulticastDelegate, UInkThread*, Caller, const FString&, FirstParameter, const FString&, SecondParameter, const FString&, ThirdParameter);

UCLASS()
class UFuckYou : public UObject
{
	GENERATED_BODY()
};