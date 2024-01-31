/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"

#include "InkVar.h"

#include "InkDelegates.generated.h"

/** @file
 * collection of Delegates typs used for the UE interface
 * @ingroup unreal
 */

#ifdef DOXYGEN
DOC_UF(BlueprintImplementableEvent, )
/** Delegate for a tag function
 * @see @ref TagFunction
 * @param Caller thread which found encountered this function
 * @param Params an array containing the arguments depending on the function call
 *
 * @blueprint
 */
void FTagFunctionDelegate(UInkThread* Caller, const TArray<FString>& Params);
#endif
/** @cond */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FTagFunctionDelegate, UInkThread*, Caller, const TArray<FString>&, Params);
/** @endcond*/

/** @private */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FTagFunctionMulticastDelegate, UInkThread*, Caller, const TArray<FString>&, Params);

#ifdef DOXYGEN
DOC_UF(BlueprintImplementableEvent, )
/** Delegate for external functions.
 * the number of arugments are defined iside the ink story
 * @param Arguments array containing all arguments passed to this function
 * @return value to be put on the stack in the inkruntime
 * @see @ref UInkThread::RegisterExternalEvent
 *
 * @blueprint
 */
FInkVar FExternalFunctionDelegate(const TArray<FInkVar>& Arguments);
#endif
/** @cond */
DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(FInkVar, FExternalFunctionDelegate, const TArray<FInkVar>&, Arguments);
/** @endcond */

#ifdef DOXYGEN
DOC_UF(BlueprintImplementableEvent, )
/** Delegate for external event.
 * the number of arguments are defined inside the ink story
 * @param Arguments array containing all arguments passed to this function
 * @see @ref UInkThread::RegisterExternalFunction
 *
 * @blueprint
 */
void FExternalFunctionVoidDelegate(const TArray<FInkVar>& Arguments);
#endif
/** @cond */
DECLARE_DYNAMIC_DELEGATE_OneParam(FExternalFunctionVoidDelegate, const TArray<FInkVar>&, Arguments);
/** @endcond */

/** @private */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FGlobalTagFunctionDelegate, UInkThread*, Caller, const TArray<FString>&, Params);

/** @private */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGlobalTagFunctionMulticastDelegate, UInkThread*, Caller, const TArray<FString>&, Params);


#ifdef DOXYGEN
DOC_UF(BlueprintImplementableEvent, )
/** Notification if variable changes.
 * @see @ref AInkRuntime::ObserverVariable()
 *
 * @blueprint
 */
void FVariableCallbackDelegate();
#endif
/** @cond */
DECLARE_DYNAMIC_DELEGATE(FVariableCallbackDelegate);
/** @endcond */

#ifdef DOXYGEN
DOC_UF(BlueprintImplementableEvent, )
/** Notification containing the new variable value, send on variable change.
 * @param value new value of the variable
 * @see @ref AInkRuntime::ObserverVariableEvent()
 *
 * @blueprint
 */
void FVariableCallbackDelegateNewValue(const FInkVar& value);
#endif
/** @cond */
DECLARE_DYNAMIC_DELEGATE_OneParam(FVariableCallbackDelegateNewValue, const FInkVar&, value);
/** @endcond */

#ifdef DOXYGEN
DOC_UF(BlueprintImplementableEvent, )
/** Notification containing old and new varible, send on variable change.
 * @param value new value of the variable
 * @param old_value previouse value of the variable has value @ref EInkVarType::None "None" if
 * variable is set for the first time
 * @see @ref AInkRuntime::ObserverVariableChange()
 *
 * @blueprint
 */
void FVariableCallbackDelegateNewOldValue(const FInkVar& value, const FInkVar& old_value);
#endif
/** @cond */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FVariableCallbackDelegateNewOldValue, const FInkVar&, value, const FInkVar&, old_value);
/** @endcond */

UCLASS()

/** @private */
class UDelegateKeepAlive : public UObject
{
	GENERATED_BODY()
};

/** @page TagFunction TagFunction
 * "tag functions" allowes converting tags or context lines to function calls
 * if a tag in the form `functionName_arg1_arg2` is found for a registered tag function
 * the function will be executed
 *
 * Also if a context line starts with a `>>` the normal processing of tags and context
 * will be ignored the corresponding tag functino will be called
 * the format for this function call is `>> FunctionName(Arg1[, Arg2]*)`
 *
 * @see To bind tag functions use @ref UInkThread::RegisterTagFunction() and @ref
 * AInkRuntime::RegisterTagFunction()
 */
