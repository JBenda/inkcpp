/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "inkcpp.h"
#include "UObject/TextProperty.h"
#include "Containers/StringConv.h"

#include "InkList.h"

#include "InkVar.generated.h"

/** Label for types possible contained in a @ref FInkVar
 * @ingroup unreal
 */
UENUM(BlueprintType)
enum class EInkVarType : uint8 {
	Float,  ///< contains a value of type float
	Int,    ///< contains a value of type int
	UInt,   ///< @todo currently not supported in Blueprints (converts to signed)
	Bool,   ///< contains a boolean
	String, ///< contains a string value
	List,   ///< contains a @ref UInkList
	None    ///< contains no value
};

namespace ink::runtime
{
struct value;
} // namespace ink::runtime

/** A wrapper for passing around ink vars to and from ink itself.
 * To access the values see @ref UInkVarLibrary
 * @see UInkVarLibrary
 * @ingroup unreal
 */
USTRUCT(BlueprintType)

struct INKCPP_API FInkVar {
	GENERATED_BODY()

	FInkVar()
	    : VarType(EInkVarType::None)
	    , IntVal(0)
	{
	}

	/** @private */
	FInkVar(float val)
	    : VarType(EInkVarType::Float)
	    , FloatVal(val)
	{
	}

	/** @private */
	FInkVar(int val)
	    : VarType(EInkVarType::Int)
	    , IntVal(val)
	{
	}

	/** @private */
	FInkVar(unsigned val)
	    : VarType(EInkVarType::UInt)
	    , UIntVal(val)
	{
		UE_LOG(
		    InkCpp, Warning,
		    TEXT("Converting unsigned to signed int, since missing blueprint support for unsigned type")
		);
	}

	/** @private */
	FInkVar(bool val)
	    : VarType(EInkVarType::Bool)
	    , BoolVal(val)
	{
	}

	/** @private */
	FInkVar(FString val)
	    : VarType(EInkVarType::String)
	    , IntVal(0)
	    , StringVal(MoveTemp(val))
	{
		BufferDecodedString();
	}

	/** @private */
	FInkVar(UInkList& List);

	/** @private */
	FInkVar(ink::runtime::value val);

	/** @private */
	ink::runtime::value to_value() const;

	/** Get the type contained in the value
	 * @retval EInkVarType::None if no value is contained (void)
	 * @private
	 */
	EInkVarType type() const { return VarType; }

	// -----------------------------------------------------------------------
	// Data — laid out as explicit named fields so the GC can see ListVal and
	// StringVal, and there is no union obscuring UObject* from the reflector.
	// -----------------------------------------------------------------------

	/** @private */
	UPROPERTY()
	EInkVarType VarType = EInkVarType::None;

	// Scalar storage — only one is valid at a time based on VarType.
	// Not UPROPERTY because they hold primitive types the GC does not need.
	/** @private */ float  FloatVal = 0.f;
	/** @private */ int32  IntVal   = 0;
	/** @private */ uint32 UIntVal  = 0;
	/** @private */ bool   BoolVal  = false;

	/** String storage — separate field so it is always properly constructed/destroyed.
	 * @private
	 */
	FString StringVal;

	/** List storage — UPROPERTY so the GC can trace the UObject*.
	 * Only valid when VarType == EInkVarType::List.
	 * @private
	 */
	UPROPERTY()
	TObjectPtr<UInkList> ListVal = nullptr;

	/** Keeps utf8 version of the string alive to write it back to the ink runtime.
	 * @private
	 */
	TArray<UTF8CHAR> Utf8{};

private:
	void BufferDecodedString()
	{
		FTCHARToUTF8 Convert(*StringVal);
		Utf8.SetNum(Convert.Length() + 1);
		UTF8CHAR*       dst = Utf8.GetData();
		const UTF8CHAR* src = reinterpret_cast<const UTF8CHAR*>(Convert.Get());
		while (*src) {
			*dst++ = *src++;
		}
		*dst = static_cast<UTF8CHAR>(0);
	}
};

/** Conversion Methods for @ref FInkVar
 * @ingroup unreal
 */
UCLASS()

class INKCPP_API UInkVarLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Var Type", BlueprintAutocast), Category = "Ink")
	/** Get the type contained in the value
	 * @retval EInkVarType::None if no value is contained (void)
	 *
	 * @blueprint
	 */
	static EInkVarType InkVarType(const FInkVar& InkVar);

	UFUNCTION(
	    BlueprintPure,
	    meta     = (DisplayName = "String (Ink Var)", CompactNodeTitle = "->", BlueprintAutocast),
	    Category = "Ink"
	)
	/** Access String value
	 *
	 * @blueprint
	 */
	static FString Conv_InkVarString(const FInkVar& InkVar);

	UFUNCTION(
	    BlueprintPure,
	    meta     = (DisplayName = "Int (Ink Var)", CompactNodeTitle = "->", BlueprintAutocast),
	    Category = "Ink"
	)
	/** Access Int value
	 *
	 * @blueprint
	 */
	static int Conv_InkVarInt(const FInkVar& InkVar);

	UFUNCTION(
	    BlueprintPure,
	    meta     = (DisplayName = "Float (Ink Var)", CompactNodeTitle = "->", BlueprintAutocast),
	    Category = "Ink"
	)
	/** Access Float Value
	 *
	 * @blueprint
	 */
	static float Conv_InkVarFloat(const FInkVar& InkVar);

	UFUNCTION(
	    BlueprintPure,
	    meta     = (DisplayName = "Name (Ink Var)", CompactNodeTitle = "->", BlueprintAutocast),
	    Category = "Ink"
	)
	/** Access String value as FName
	 *
	 * @blueprint
	 */
	static FName Conv_InkVarName(const FInkVar& InkVar);

	UFUNCTION(
	    BlueprintPure,
	    meta     = (DisplayName = "Text (Ink Var)", CompactNodeTitle = "->", BlueprintAutocast),
	    Category = "Ink"
	)
	/** Access String value as FText
	 *
	 * @blueprint
	 */
	static FText Conv_InkVarText(const FInkVar& InkVar);

	UFUNCTION(
	    BlueprintPure,
	    meta     = (DisplayName = "Bool (Ink Var)", CompactNodeTitle = "->", BlueprintAutocast),
	    Category = "Ink"
	)
	/** Access bool value
	 * @blueprint */
	static bool Conv_InkVarBool(const FInkVar& InkVar);

	UFUNCTION(
	    BlueprintPure,
	    meta     = (DisplayName = "InkList (Ink Var)", CompactNodeTitle = "->", BlueprintAutocast),
	    Category = "Ink"
	)
	/** Access @ref UInkList "List" value
	 *
	 * @blueprint
	 */
	static const UInkList* Conv_InkVarInkList(const FInkVar& InkVar);

	UFUNCTION(
	    BlueprintPure,
	    meta     = (DisplayName = "Ink Var (String)", CompactNodeTitle = "->", BlueprintAutocast),
	    Category = "Ink"
	)
	/** Convert string to @ref FInkVar
	 *
	 * @blueprint
	 */
	static FInkVar Conv_StringInkVar(const FString& String);

	UFUNCTION(
	    BlueprintPure,
	    meta     = (DisplayName = "Ink Var (Int)", CompactNodeTitle = "->", BlueprintAutocast),
	    Category = "Ink"
	)
	/** Convert int to @ref FInkVar
	 *
	 * @blueprint
	 */
	static FInkVar Conv_IntInkVar(int Number);

	UFUNCTION(
	    BlueprintPure,
	    meta     = (DisplayName = "Ink Var (Float)", CompactNodeTitle = "->", BlueprintAutocast),
	    Category = "Ink"
	)
	/** Convert float to @ref FInkVar
	 *
	 * @blueprint
	 */
	static FInkVar Conv_FloatInkVar(float Number);

	UFUNCTION(
	    BlueprintPure,
	    meta     = (DisplayName = "Ink Var (Text)", CompactNodeTitle = "->", BlueprintAutocast),
	    Category = "Ink"
	)
	/** Convert FText to @ref FInkVar of type @ref EInkVarType::String "String"
	 *
	 * @blueprint
	 */
	static FInkVar Conv_TextInkVar(const FText& Text);

	UFUNCTION(
	    BlueprintPure,
	    meta     = (DisplayName = "Ink Var (Name)", CompactNodeTitle = "->", BlueprintAutocast),
	    Category = "Ink"
	)
	/** Convert FName to @ref FInkVar of type @ref EInkVarType::String "String"
	 *
	 * @blueprint
	 */
	static FInkVar Conv_NameInkVar(const FName& Name);

	UFUNCTION(
	    BlueprintPure,
	    meta     = (DisplayName = "Ink Var (Bool)", CompactNodeTitle = "->", BlueprintAutocast),
	    Category = "Ink"
	)
	/** Convert bool to @ref FInkVar
	 *
	 * @blueprint
	 */
	static FInkVar Conv_BoolInkVar(bool Boolean);

	UFUNCTION(
	    BlueprintPure,
	    meta     = (DisplayName = "Ink Var (InkList)", CompactNodeTitle = "->", BlueprintAutocast),
	    Category = "Ink"
	)
	/** Converts @ref UInkList "List" to @ref FInkVar
	 *
	 * @blueprint
	 */
	static FInkVar Conv_ListInkVar(UInkList* List);
};
