#pragma once

#include "UObject/Object.h"

#include "Choice.generated.h"

namespace ink::runtime { class choice; }
class UTagList;

UCLASS(BlueprintType)
class UChoice : public UObject
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintPure, Category="Ink")
	FString GetText() const;

	UFUNCTION(BlueprintPure, Category="Ink")
	int GetIndex() const;

	UFUNCTION(BlueprintPure, Category="Ink")
	const UTagList* GetTags() const;

protected:
	friend class UInkThread;
	void Initialize(const ink::runtime::choice*);

private:
	const ink::runtime::choice* data;
	const UTagList* tags;
};