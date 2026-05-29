/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

class UInkThread;

/** Thread-local pointer to the UInkThread currently executing inside Execute().
 *  Set exclusively via FInkExecutionScope — never write it directly.
 *  Used by FInkVar to register newly created UInkList wrappers so the thread
 *  can invalidate them before the next choose() call.
 */
extern thread_local UInkThread* GExecutingInkThread;

/** RAII guard that sets/clears GExecutingInkThread for the lifetime of a
 *  UInkThread::Execute() call. Impossible to forget to clear.
 */
struct FInkExecutionScope {
	explicit FInkExecutionScope(UInkThread* thread) { GExecutingInkThread = thread; }

	~FInkExecutionScope() { GExecutingInkThread = nullptr; }

	FInkExecutionScope(const FInkExecutionScope&)            = delete;
	FInkExecutionScope& operator=(const FInkExecutionScope&) = delete;
};
