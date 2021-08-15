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

void UInkChoice::Initialize(const ink::runtime::choice* c)
{
	data = c;
}