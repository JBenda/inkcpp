/* Copyright (c) 2024 Julian Benda
 *
 * This file is part of inkCPP which is released under MIT license.
 * See file LICENSE.txt or go to
 * https://github.com/JBenda/inkcpp for full license details.
 */
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
	~UInkAssetFactory();

	// Begin UFactory
	virtual UObject* FactoryCreateFile(UClass* InClass, UObject* InParent, FName InName, 
		EObjectFlags Flags, const FString& Filename, const TCHAR* Parms,
		FFeedbackContext* Warn, bool& bOutOperationCanceled) override;
	virtual bool FactoryCanImport(const FString& Filename) override;
	// End UFactory

	// Begin FReimportHandler
	virtual bool                  CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
	virtual TObjectPtr<UObject>*  GetFactoryObject() const override;
	virtual EReimportResult::Type Reimport( UObject* Obj ) override;

	virtual void  SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
	virtual int32 GetPriority() const override;
	// End FReimportHandle
private:
	TObjectPtr<UObject> object_ptr;
};
