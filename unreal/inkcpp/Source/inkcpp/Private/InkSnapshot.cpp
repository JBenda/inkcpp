/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "InkSnapshot.h"
#include "InkRuntime.h"

#include "Async/Async.h"

UInkMigratableSnapshotAsync* UInkMigratableSnapshotAsync::GetMigratableSnapshot(AInkRuntime* Runtime
)
{
	UInkMigratableSnapshotAsync* Node = NewObject<UInkMigratableSnapshotAsync>();
	Node->Runtime                     = Runtime;
	return Node;
}

void UInkMigratableSnapshotAsync::Activate()
{
	if (! Runtime) {
		Completed.Broadcast(FInkSnapshot());
		SetReadyToDestroy();
		return;
	}

	TFuture<FInkSnapshot> Future = Runtime->MigratableSnapshot();

	TWeakObjectPtr<UInkMigratableSnapshotAsync> WeakThis(this);

	Future.Next([WeakThis](FInkSnapshot Snapshot) {
		AsyncTask(ENamedThreads::GameThread, [WeakThis, Snapshot]() {
			if (! WeakThis.IsValid())
				return;

			WeakThis->Completed.Broadcast(Snapshot);
			WeakThis->SetReadyToDestroy();
		});
	});
}

void UInkMigratableSnapshotAsync::HandleResult(const FInkSnapshot& Snapshot)
{
	Completed.Broadcast(Snapshot);
	SetReadyToDestroy();
}
