#pragma once

#include "TagList.h"

#include "UObject/Object.h"

#include "InkChoice.generated.h"

namespace ink::runtime { class choice; }
class UTagList;

UCLASS(BlueprintType)
class UInkChoice : public UObject
{
	GENERATED_BODY()
public:
	UInkChoice();

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
	TObjectPtr<UTagList> tags;
};