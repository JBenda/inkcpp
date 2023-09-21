#pragma once

#include "InkList.h"

bool UInkList::ContainsFlag(const FString& flag_name) const {
  return list_data->contains(TCHAR_TO_ANSI(*flag_name));
}

bool UInkList::ContainsEnum(const FString& enumName, const uint8& value) const {
  UEnum* Enum = FindObjectSafe<UEnum>(nullptr, *enumName, true);
  if(!Enum)
  {
    UE_LOG(InkCpp, Warning, TEXT("Failed to find enum with name '%s'"), *enumName);
    return false;
  }

  return list_data->contains(TCHAR_TO_ANSI(*Enum->GetNameStringByIndex(value)));  
}

TArray<uint8> UInkList::ElementsOf(const FString& enumName) const {
  TArray<uint8> ret;
  UEnum* Enum = FindObjectSafe<UEnum>(nullptr, *enumName, true);
  if (!Enum) {
    UE_LOG(InkCpp, Warning, TEXT("Failed to find enum with name '%s'"), *enumName);
    return ret;
  }

  int num = Enum->NumEnums();
  for(auto itr = list_data->begin(TCHAR_TO_ANSI(*enumName)); itr != list_data->end(); ++itr)
  {
    bool hit = false;
    for(int i = 0; i < num; ++i) {
      FString enumStr = Enum->GetNameStringByIndex(i);
      if (enumStr.EndsWith((*itr).flag_name)) {
        ret.Add(i);
        hit = true;
        break;
      }
    }
    if (!hit) {
      UE_LOG(InkCpp, Warning, TEXT("Failed to find list value '%s' in enum '%s'"), (*itr).flag_name, *enumName);
    }
  }

  return ret;
}

TArray<FString> UInkList::ElementsOfAsString(const FString& list_name) const {
  TArray<FString> ret;

  for(auto itr = list_data->begin(TCHAR_TO_ANSI(*list_name)); itr != list_data->end(); ++itr)
  {
    ret.Add(FString((*itr).flag_name));
  }
  return ret;
}

TArray<FListFlag> UInkList::Elements() const {
  TArray<FListFlag> ret;
  for(auto itr = list_data->begin(); itr != list_data->end(); ++itr)
  {
    ret.Add(FListFlag{
      .list_name = FString((*itr).list_name),
      .flag_name = FString((*itr).flag_name),
    });
  }
  return ret;
}

bool UInkList::ContainsList(const FString& name) const {
  return list_data->begin(TCHAR_TO_ANSI(*name)) != list_data->end();
}