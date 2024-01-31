/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "InkVar.h"
#include "ink/types.h"

#include "Misc/AssertionMacros.h"

FInkVar::FInkVar(ink::runtime::value val) : FInkVar() {
	using v_types = ink::runtime::value::Type;
	switch(val.type) {
		case v_types::Bool: value.SetSubtype<bool>(val.get<v_types::Bool>()); break;
		case v_types::Uint32:
			UE_LOG(InkCpp, Warning, TEXT("Converting uint to int, this will cause trouble if writing it back to ink (with SetGlobalVariable)!"));
			value.SetSubtype<int>(val.get<v_types::Uint32>());
			break;
		case v_types::Int32: value.SetSubtype<int>(val.get<v_types::Int32>()); break;
		case v_types::String: value.SetSubtype<FString>(FString(val.get<v_types::String>())); break;
		case v_types::Float: value.SetSubtype<float>(val.get<v_types::Float>()); break;
		case v_types::List: {
			UInkList* list = NewObject<UInkList>();
			list->SetList(val.get<v_types::List>());
			value.SetSubtype<UInkList*>(list);
			break;
		}
		default:
			inkFail("unknown type!, failed to convert ink::value to InkVar");
	}
}
	
ink::runtime::value FInkVar::to_value() const {
	switch(type()) {
		case EInkVarType::Int:
			return ink::runtime::value(value.GetSubtype<int>());
		case EInkVarType::Float:
			return ink::runtime::value(value.GetSubtype<float>());
		case EInkVarType::String:
			return ink::runtime::value(reinterpret_cast<const char*>(utf8.GetData()));
		case EInkVarType::Bool:
			return ink::runtime::value(value.GetSubtype<bool>());
		case EInkVarType::UInt:
			return ink::runtime::value(value.GetSubtype<unsigned>());
		case EInkVarType::List:
			return ink::runtime::value(value.GetSubtype<UInkList*>()->GetData());
		default:
			inkFail("Unsupported type");
			return ink::runtime::value();
	}
	
}

EInkVarType UInkVarLibrary::InkVarType(const FInkVar& InkVar) { return InkVar.type(); }
FString UInkVarLibrary::Conv_InkVarString(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.type() == EInkVarType::String, TEXT("InkVar is not a String Type!")))
		return InkVar.value.GetSubtype<FString>();
	return FString(TEXT(""));
}

int UInkVarLibrary::Conv_InkVarInt(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.type() == EInkVarType::Int, TEXT("InkVar is not an Int Type!")))
		return InkVar.value.GetSubtype<int>();
	return 0;
}

float UInkVarLibrary::Conv_InkVarFloat(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.type() == EInkVarType::Float, TEXT("InkVar is not a Float Type!")))
		return InkVar.value.GetSubtype<float>();
	return 0.f;
}

FName UInkVarLibrary::Conv_InkVarName(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.type() == EInkVarType::String, TEXT("InkVar is not a String Type!")))
		return FName(*InkVar.value.GetSubtype<FString>());
	return NAME_None;
}

FText UInkVarLibrary::Conv_InkVarText(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.type() == EInkVarType::String, TEXT("InkVar is not a String Type!")))
		return FText::FromString(InkVar.value.GetSubtype<FString>());
	return FText::GetEmpty();
}

bool UInkVarLibrary::Conv_InkVarBool(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.type() == EInkVarType::Bool, TEXT("InkVar is not an Bool Type!")))
		return InkVar.value.GetSubtype<bool>();
	return false;
}

const UInkList* UInkVarLibrary::Conv_InkVarInkList(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.type() == EInkVarType::List, TEXT("InkVar is not an List Type!")))
		return InkVar.value.GetSubtype<UInkList*>();
	return nullptr;
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

FInkVar UInkVarLibrary::Conv_NameInkVar(const FName& Name)
{
	return FInkVar(Name.ToString());
}

FInkVar UInkVarLibrary::Conv_BoolInkVar(bool Boolean)
{
	return FInkVar(Boolean);
}

FInkVar UInkVarLibrary::Conv_ListInkVar(UInkList* List)
{
	if (ensureMsgf(List != nullptr, TEXT("Try to set list value from empty object"))) {
		return FInkVar(*List);
	}
	return FInkVar();
}
