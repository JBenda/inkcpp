#pragma once

#include "EditorReimportHandler.h"
#include "Factories/Factory.h"
#include "UObject/ObjectMacros.h"

#include "InkAssetFactory.generated.h"

UCLASS(hidecategories=Object)
class UInkAssetFactory : public UFactory, public FReimportHandler
{
	GENERATED_BODY()
	
public:
	UInkAssetFactory(const FObjectInitializer& ObjectInitializer);

	// Begin UFactory
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, 
		EObjectFlags Flags, const FString& Filename, const TCHAR* Parms,
		FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	virtual bool FactoryCanImport(const FString& Filename) override;
	// End UFactory

	// Begin FReimportHandler
	bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	const UObject* GetFactoryObject() const override;
	EReimportResult::Type Reimport(UObject* Obj) override;
	void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual int32 GetPriority() const override;
	// End FReimportHandler
};