/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once
#include "TagList.h"

#include "UObject/Object.h"

#include "InkChoice.generated.h"

namespace ink::runtime { class choice; }
class UTagList;

/** Representing a Ink Choice in the story flow
 * @ingroup unreal
 */
UCLASS(BlueprintType)
class UInkChoice : public UObject
{
	GENERATED_BODY()
public:
	UInkChoice();

	UFUNCTION(BlueprintPure, Category="Ink")
	/** Access context of choice.
	 * @return text contained in choice
	 * @blueprint
	 */
	FString GetText() const;

	UFUNCTION(BlueprintPure, Category="Ink")
	/** Get idintifcator for @ref UInkThread::PickChoice()
	 * @return id used in @ref UInkThread::PickChoice()
	 *
	 * @blueprint
	 */
	int GetIndex() const;

	UFUNCTION(BlueprintPure, Category="Ink")
	/** Tags asszoiated with the choice.
	 * @return with choice assoziated tags
	 *
	 * @blueprint
	 */
	const UTagList* GetTags() const;

protected:
	friend class UInkThread;
	/** @private */
	void Initialize(const ink::runtime::choice*);

private:
	const ink::runtime::choice* data;
	TObjectPtr<UTagList> tags;
};
