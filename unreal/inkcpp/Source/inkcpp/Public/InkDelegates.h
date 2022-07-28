#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"

#include "InkVar.h"

#include "InkDelegates.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FTagFunctionDelegate, UInkThread*, Caller, const TArray<FString>&, Params);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTagFunctionMulticastDelegate, UInkThread*, Caller, const TArray<FString>&, Params);
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(FInkVar, FExternalFunctionDelegate, const TArray<FInkVar>&, Arguments);
DECLARE_DYNAMIC_DELEGATE_OneParam(FExternalFunctionVoidDelegate, const TArray<FInkVar>&, Arguments);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FGlobalTagFunctionDelegate, UInkThread*, Caller, const TArray<FString>&, Params);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGlobalTagFunctionMulticastDelegate, UInkThread*, Caller, const TArray<FString>&, Params);

UCLASS()
class UFuckYou : public UObject
{
	GENERATED_BODY()
};