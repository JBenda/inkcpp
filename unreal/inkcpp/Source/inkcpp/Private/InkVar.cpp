/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#include "InkVar.h"
#include "InkExecutionScope.h"
#include "InkThread.h"
#include "ink/types.h"

#include "Misc/AssertionMacros.h"

extern thread_local UInkThread* GExecutingInkThread;

FInkVar::FInkVar(ink::runtime::value val)
    : FInkVar()
{
	using v_types = ink::runtime::value::Type;
	switch (val.type) {
		case v_types::Bool:
			BoolVal = val.get<v_types::Bool>();
			VarType = EInkVarType::Bool;
			break;
		case v_types::Uint32:
			UE_LOG(
			    InkCpp, Warning,
			    TEXT("Converting uint to int, this will cause trouble if writing it back to ink (with "
			         "SetGlobalVariable)!")
			);
			IntVal  = ( int32 ) val.get<v_types::Uint32>();
			VarType = EInkVarType::Int;
			break;
		case v_types::Int32:
			IntVal  = val.get<v_types::Int32>();
			VarType = EInkVarType::Int;
			break;
		case v_types::String:
			StringVal = FString(UTF8_TO_TCHAR(val.get<v_types::String>()));
			VarType   = EInkVarType::String;
			break;
		case v_types::Float:
			FloatVal = val.get<v_types::Float>();
			VarType  = EInkVarType::Float;
			break;
		case v_types::List: {
			ListVal = NewObject<UInkList>();
			ListVal->SetList(val.get<v_types::List>());
			VarType = EInkVarType::List;
			// Register with the executing thread (if any) via the RAII-guarded
			// thread-local so the thread can invalidate this list before choose().
			// GExecutingInkThread is null when called outside Execute() (e.g.
			// GetGlobalVariable), in which case the list is globals-lifetime and
			// needs no per-choice invalidation.
			if (GExecutingInkThread) {
				GExecutingInkThread->RegisterLiveList(ListVal);
			}
			break;
		}
		default: inkFail("unknown type!, failed to convert ink::value to InkVar");
	}
}

FInkVar::FInkVar(UInkList& List)
    : VarType(EInkVarType::List)
    , IntVal(0)
    , ListVal(&List)
{
	if (GExecutingInkThread) {
		GExecutingInkThread->RegisterLiveList(ListVal);
	}
}

ink::runtime::value FInkVar::to_value() const
{
	switch (VarType) {
		case EInkVarType::Int: return ink::runtime::value(IntVal);
		case EInkVarType::Float: return ink::runtime::value(FloatVal);
		case EInkVarType::String:
			return ink::runtime::value(reinterpret_cast<const char*>(Utf8.GetData()));
		case EInkVarType::Bool: return ink::runtime::value(BoolVal);
		case EInkVarType::UInt: return ink::runtime::value(UIntVal);
		case EInkVarType::List: return ink::runtime::value(ListVal->GetData());
		default: inkFail("Unsupported type"); return ink::runtime::value();
	}
}

EInkVarType UInkVarLibrary::InkVarType(const FInkVar& InkVar) { return InkVar.VarType; }

FString UInkVarLibrary::Conv_InkVarString(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.VarType == EInkVarType::String, TEXT("InkVar is not a String Type!")))
		return InkVar.StringVal;
	return FString(TEXT(""));
}

int UInkVarLibrary::Conv_InkVarInt(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.VarType == EInkVarType::Int, TEXT("InkVar is not an Int Type!")))
		return InkVar.IntVal;
	return 0;
}

float UInkVarLibrary::Conv_InkVarFloat(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.VarType == EInkVarType::Float, TEXT("InkVar is not a Float Type!")))
		return InkVar.FloatVal;
	return 0.f;
}

FName UInkVarLibrary::Conv_InkVarName(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.VarType == EInkVarType::String, TEXT("InkVar is not a String Type!")))
		return FName(*InkVar.StringVal);
	return NAME_None;
}

FText UInkVarLibrary::Conv_InkVarText(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.VarType == EInkVarType::String, TEXT("InkVar is not a String Type!")))
		return FText::FromString(InkVar.StringVal);
	return FText::GetEmpty();
}

bool UInkVarLibrary::Conv_InkVarBool(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.VarType == EInkVarType::Bool, TEXT("InkVar is not an Bool Type!")))
		return InkVar.BoolVal;
	return false;
}

const UInkList* UInkVarLibrary::Conv_InkVarInkList(const FInkVar& InkVar)
{
	if (ensureMsgf(InkVar.VarType == EInkVarType::List, TEXT("InkVar is not an List Type!")))
		return InkVar.ListVal;
	return nullptr;
}

FInkVar UInkVarLibrary::Conv_StringInkVar(const FString& String) { return FInkVar(String); }

FInkVar UInkVarLibrary::Conv_IntInkVar(int Number) { return FInkVar(Number); }

FInkVar UInkVarLibrary::Conv_FloatInkVar(float Number) { return FInkVar(Number); }

FInkVar UInkVarLibrary::Conv_TextInkVar(const FText& Text) { return FInkVar(Text.ToString()); }

FInkVar UInkVarLibrary::Conv_NameInkVar(const FName& Name) { return FInkVar(Name.ToString()); }

FInkVar UInkVarLibrary::Conv_BoolInkVar(bool Boolean) { return FInkVar(Boolean); }

FInkVar UInkVarLibrary::Conv_ListInkVar(UInkList* List)
{
	if (ensureMsgf(List != nullptr, TEXT("Try to set list value from empty object"))) {
		return FInkVar(*List);
	}
	return FInkVar();
}
