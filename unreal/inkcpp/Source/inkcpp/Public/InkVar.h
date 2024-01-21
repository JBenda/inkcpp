#pragma once

// NOTICE: EInkVarType and FInkVar are copied from David Colson's UnrealInk project @ https://github.com/DavidColson/UnrealInk

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UObject/TextProperty.h"
#include "Containers/Union.h"
#include "Containers/StringConv.h"

#include "InkList.h"

#include "InkVar.generated.h"

/** Label for types possible contained in a @ref FInkVar
 * @ingroup unreal
 */
UENUM(BlueprintType)
enum class EInkVarType : uint8
{
	Float,
	Int,
	UInt,
	Bool,
	String,
	List,
	None
};

namespace ink::runtime { struct value; }

/** A wrapper for passing around ink vars to and from ink itself.
 * To access the values see @ref UInkVarLibrary
 * @see UInkVarLibrary
 * @ingroup unreal
 */
USTRUCT(BlueprintType)
struct INKCPP_API FInkVar
{
	GENERATED_BODY()

	FInkVar() {}

	/** @private */
	FInkVar(float val) { value.SetSubtype<float>(val); }
	/** @private */
	FInkVar(int val) { value.SetSubtype<int>(val); }
	/** @private */
	FInkVar(unsigned val)
	{
		UE_LOG(InkCpp, Warning, TEXT("Converting unsigned to signed int, since missing blueprint support for unsigned type"));
		value.SetSubtype<int>(val);
	} // TODO: change if we find a way to support unsigned values in blueprints
	/** @private */
	FInkVar(bool val) { value.SetSubtype<bool>(val); }
	/** @private */
	FInkVar(FString val) {
		value.SetSubtype<FString>(val); 
		BufferDecodedString();
	}
	/** @private */
	FInkVar(const UInkList& List) { value.SetSubtype<UInkList*>(&List); }
	/** @private */
	FInkVar(ink::runtime::value val);
	
	/** @private */
	ink::runtime::value to_value() const;


	// allow changing via Editor, but not in controle flow, it is just a wrapper type to create a new one
	// UPROPERTY(EditAnywhere, Category = "Ink")
	/** @private */
	TUnion<float, int, unsigned, bool, FString, UInkList*> value;

	/** Keeps utf8 version of string alive to write it in runtime.
	 * @private 
	 */
	TArray<UTF8CHAR> utf8{};

	/** Get the type contained in the value
	 * @retval EInkVarType::None if no value is contained (void)
	 * @private
	 */
	EInkVarType type() const {
		uint8 id = value.GetCurrentSubtypeIndex();
		if(id >= static_cast<uint8>(EInkVarType::None))
		{ return EInkVarType::None; }
		else
		{ return static_cast<EInkVarType>(id); }
	}
private:
	void BufferDecodedString() {
		FTCHARToUTF8 Convert(*value.GetSubtype<FString>());
		utf8.SetNum(Convert.Length() + 1);
		UTF8CHAR* dst = utf8.GetData(); 
		[this,&dst](const UTF8CHAR* src){
			int i = 0;
			while(*src) {
				*dst++ = *src++;
			}
			*dst = static_cast<UTF8CHAR>(0);
		}(reinterpret_cast<const UTF8CHAR*>(Convert.Get()));
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
	 */
	static EInkVarType InkVarType(const FInkVar& InkVar) { return InkVar.type(); }

	UFUNCTION(BlueprintPure, meta = (DisplayName = "String (Ink Var)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Ink")
	/** @blueprint */
	static FString Conv_InkVarString(const FInkVar& InkVar);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Int (Ink Var)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Ink")
	/** @blueprint */
	static int Conv_InkVarInt(const FInkVar& InkVar);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Float (Ink Var)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Ink")
	/** @blueprint */
	static float Conv_InkVarFloat(const FInkVar& InkVar);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Name (Ink Var)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Ink")
	/** @blueprint */
	static FName Conv_InkVarName(const FInkVar& InkVar);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Text (Ink Var)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Ink")
	/** @blueprint */
	static FText Conv_InkVarText(const FInkVar& InkVar);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Bool (Ink Var)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Ink")
	/** @blueprint */
	static bool Conv_InkVarBool(const FInkVar& InkVar);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "InkList (Ink Var)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Ink")
	/** @blueprint */
	static const UInkList* Conv_InkVarInkList(const FInkVar& InkVar);
	
	// UFUNCTION(BlueprintPure, meta = (DisplayName = "UInt (Ink Var)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Ink")
	// static unsigned Conv_InkVarUInt(const FInkVar& InkVar);

	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ink Var (String)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Ink")
	/** @blueprint */
	static FInkVar Conv_StringInkVar(const FString& String);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ink Var (Int)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Ink")
	/** @blueprint */
	static FInkVar Conv_IntInkVar(int Number);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ink Var (Float)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Ink")
	/** @blueprint */
	static FInkVar Conv_FloatInkVar(float Number);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ink Var (Text)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Ink")
	/** @blueprint */
	static FInkVar Conv_TextInkVar(const FText& Text);
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ink Var (Name)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Ink")
	/** @blueprint */
	static FInkVar Conv_NameInkVar(const FName& Name);

	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ink Var (Bool)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Ink")
	/** @blueprint */
	static FInkVar Conv_BoolInkVar(bool Boolean);
	
	UFUNCTION(BlueprintPure, meta = (DisplayName = "Ink Var (InkList)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Ink")
	/** @blueprint */
	static FInkVar UInkVarLibrary::Conv_ListInkVar(const UInkList& List);
	
	// UFUNCTION(BlueprintPure, meta = (DisplayName = "Ink Var (UInt)", CompactNodeTitle = "->", BlueprintAutocast), Category = "Ink")
	// static FInkVar Conv_UIntInkVar(unsigned Number);
};
