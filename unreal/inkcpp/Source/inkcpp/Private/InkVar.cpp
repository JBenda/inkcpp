#include "InkVar.h"
#include "ink/types.h"

#include "Misc/AssertionMacros.h"

FInkVar::FInkVar(ink::runtime::value val) : FInkVar() {
	using v_types = ink::runtime::value::Type;
	switch(val.type) {
		case v_types::Bool:
			type = EInkVarType::Int;
			intVar = static_cast<int>(val.v_bool);
			break;
		case v_types::Uint32:
			type = EInkVarType::Int;
			/// @TODO: add warning if overflows
			intVar = static_cast<int>(val.v_uint32);
			break;
		case v_types::Int32:
			type = EInkVarType::Int;
			intVar = val.v_int32;
			break;
		case v_types::String:
			type = EInkVarType::String;
			stringVar = FString(val.v_string);
			break;
		case v_types::Float:
			type = EInkVarType::Float;
			floatVar = val.v_float;
			break;
		default:
			inkFail("unknown type!, failed to convert ink::value to InkVar");
	}
}
	
ink::runtime::value FInkVar::to_value() const {
	switch(type) {
		case EInkVarType::Int:
			return ink::runtime::value(intVar);
		case EInkVarType::Float:
			return ink::runtime::value(floatVar);
		case EInkVarType::String:
			return ink::runtime::value(TCHAR_TO_ANSI(*stringVar));
		case EInkVarType::None:
			inkFail("None type cant be passed");
		default:
			inkFail("Unsupported type");
	}
	
}

FString UInkVarLibrary::Conv_InkVarString(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.type == EInkVarType::String, TEXT("InkVar is not a String Type!")))
		return InkVar.stringVar;
	return FString(TEXT(""));
}

int UInkVarLibrary::Conv_InkVarInt(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.type == EInkVarType::Int, TEXT("InkVar is not an Int Type!")))
		return InkVar.intVar;
	return 0;
}

float UInkVarLibrary::Conv_InkVarFloat(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.type == EInkVarType::Float, TEXT("InkVar is not a Float Type!")))
		return InkVar.floatVar;
	return 0.f;
}

FName UInkVarLibrary::Conv_InkVarName(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.type == EInkVarType::String, TEXT("InkVar is not a String Type!")))
		return FName(*InkVar.stringVar);
	return NAME_None;
}

FText UInkVarLibrary::Conv_InkVarText(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.type == EInkVarType::String, TEXT("InkVar is not a String Type!")))
		return FText::FromString(InkVar.stringVar);
	return FText::GetEmpty();
}

bool UInkVarLibrary::Conv_InkVarBool(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.type == EInkVarType::Int, TEXT("InkVar is not an Int Type!")))
		return InkVar.intVar > 0;
	return false;
}

FInkVar UInkVarLibrary::Conv_StringInkVar(const FString& String)
{
	return FInkVar(String);
}

FInkVar UInkVarLibrary::Conv_IntInkVar(int Number)
{
	return FInkVar(Number);
}

FInkVar UInkVarLibrary::Conv_FloatInkVar(float Number)
{
	return FInkVar(Number);
}

FInkVar UInkVarLibrary::Conv_TextInkVar(const FText& Text)
{
	return FInkVar(Text.ToString());
}

FInkVar UInkVarLibrary::Conv_BoolInkVar(bool Boolean)
{
	return FInkVar(Boolean ? 1 : 0);
}
