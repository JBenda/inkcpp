#pragma once

#include "UObject/Object.h"

#include "Choice.generated.h"

namespace ink::runtime { class choice; }

UCLASS(BlueprintType)
class UChoice : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category="Read")
	FString GetText() const;

	UFUNCTION(BlueprintPure, Category="Read")
	int GetIndex() const;

protected:
	friend class UInkThread;
	void Initialize(const ink::runtime::choice*);

private:
	const ink::runtime::choice* data;
};