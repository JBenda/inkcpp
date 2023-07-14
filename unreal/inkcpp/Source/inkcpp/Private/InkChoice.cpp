#include "InkChoice.h"

#include "ink/choice.h"

FString UInkChoice::GetText() const
{
	return data->text();
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
}