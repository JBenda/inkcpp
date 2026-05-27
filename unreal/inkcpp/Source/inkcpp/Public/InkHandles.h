/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "CoreMinimal.h"
#include "Templates/SharedPointer.h"

#include "InkHandles.generated.h"

/**
 * Handle returned by @ref AInkRuntime::ObserveVariable() and variants.
 * Pass to @ref AInkRuntime::UnobserveVariable() to stop receiving callbacks.
 * Letting the handle go out of scope does NOT automatically unregister —
 * you must call UnobserveVariable() explicitly.
 * @ingroup unreal
 */
USTRUCT(BlueprintType)

struct INKCPP_API FInkObserverHandle {
	GENERATED_BODY()

	/** @private */
	FInkObserverHandle() {}

	/** @private */
	explicit FInkObserverHandle(TSharedPtr<bool> token)
	    : Token(MoveTemp(token))
	{
	}

	/** Returns true if this handle refers to an active observer registration. */
	bool IsValid() const { return Token.IsValid() && *Token; }

	/** @private */
	TSharedPtr<bool> Token;
};

/**
 * Handle returned by @ref UInkThread::RegisterExternalFunction() and
 * @ref UInkThread::RegisterExternalEvent().
 * Pass to @ref UInkThread::UnregisterExternalFunction() to remove the binding.
 * @ingroup unreal
 */
USTRUCT(BlueprintType)

struct INKCPP_API FExternalFunctionHandle {
	GENERATED_BODY()

	/** @private */
	FExternalFunctionHandle() {}

	/** @private */
	explicit FExternalFunctionHandle(TSharedPtr<bool> token, FString name)
	    : Token(MoveTemp(token))
	    , FunctionName(MoveTemp(name))
	{
	}

	/** Returns true if this handle refers to an active external function registration. */
	bool IsValid() const { return Token.IsValid() && *Token; }

	/** @private */
	TSharedPtr<bool> Token;
	/** @private */
	FString          FunctionName;
};
