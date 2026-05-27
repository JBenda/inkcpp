/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "InkChoice.h"

#include "ink/choice.h"

UInkChoice::UInkChoice() { tags = NewObject<UTagList>(); }

FString UInkChoice::GetText() const { return Text; }

int UInkChoice::GetIndex() const { return Index; }

const UTagList* UInkChoice::GetTags() const { return tags; }

void UInkChoice::Initialize(const ink::runtime::choice* c)
{
	// Copy all data out of the runner immediately — the pointer is only valid
	// until the next getline() or choose() call.
	Text  = FString(UTF8_TO_TCHAR(c->text()));
	Index = c->index();
	if (c->has_tags()) {
		TArray<FString> fstring_tags{};
		for (unsigned i = 0; i < c->num_tags(); ++i) {
			fstring_tags.Add(FString(UTF8_TO_TCHAR(c->get_tag(i))));
		}
		tags->Initialize(fstring_tags);
	}
}
