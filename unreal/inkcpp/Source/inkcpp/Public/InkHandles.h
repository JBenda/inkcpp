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
 * Generic registration handle returned by all Register* functions on
 * @ref AInkRuntime and @ref UInkThread.
 *
 * Call @ref Cancel() to cancel the registration. Letting the handle go out of scope does NOT
 * automatically cancel — you must call Cancel() explicitly.
 * For Blueprint graphs, pass the handle to @ref AInkRuntime::Unregister() or
 * @ref UInkThread::Unregister() — those are thin wrappers around Cancel().
 *
 * The same handle type is used for variable observers, tag functions,
 * and external functions — the registering function name already makes
 * the context clear at the call site.
 *
 * @ingroup unreal
 */
USTRUCT(BlueprintType)

struct INKCPP_API FInkHandle {
	GENERATED_BODY()

	/** @private */
	FInkHandle() {}

	/** @private */
	explicit FInkHandle(TSharedPtr<bool> token)
	    : Token(MoveTemp(token))
	{
	}

	/** Returns true if this handle refers to an active registration. */
	bool IsValid() const { return Token.IsValid() && *Token; }

	/** Cancels the registration this handle refers to. Safe to call multiple times. */
	void Cancel() const
	{
		if (Token.IsValid())
			*Token = false;
	}

	/** @private */
	TSharedPtr<bool> Token;
};
