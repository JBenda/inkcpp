// name conflict with unreal include
#include "../Public/Choice.h"

#include "ink/choice.h"

FString UChoice::GetText() const
{
	return data->text();
}

int UChoice::GetIndex() const
{
	return data->index();
}

const UTagList* UChoice::GetTags() const
{
	return tags;
}

void UChoice::Initialize(const ink::runtime::choice* c)
{
	data = c;
}