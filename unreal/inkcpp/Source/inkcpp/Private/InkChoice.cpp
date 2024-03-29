/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "InkChoice.h"

#include "ink/choice.h"

FString UInkChoice::GetText() const
{
	return data->text();
}

UInkChoice::UInkChoice() {
	tags = NewObject<UTagList>();
}

int UInkChoice::GetIndex() const
{
	return data->index();
}

const UTagList* UInkChoice::GetTags() const
{
	return tags;
}

void UInkChoice::Initialize(const ink::runtime::choice* c)
{
	data = c;
	if (c->has_tags()) {
		TArray<FString> fstring_tags{};
		for(unsigned i = 0; i < c->num_tags(); ++i) {
			fstring_tags.Add(FString(ANSI_TO_TCHAR(c->get_tag(i))));
		}
		tags->Initialize(fstring_tags);
	}
}
