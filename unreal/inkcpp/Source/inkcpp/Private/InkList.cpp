#pragma once

#include "InkList.h"
#include <string>

bool UInkList::ContainsFlag(const FString& flag_name) const {
  return list_data->contains(TCHAR_TO_ANSI(*flag_name));
}

bool UInkList::ContainsEnum(const UEnum* Enum, const uint8& value) const {
  if(!Enum)
  {
    UE_LOG(InkCpp, Warning, TEXT("No Enum provided for ContainsEnum, it will fail therfore!"));
    return false;
  }

  return list_data->contains(TCHAR_TO_ANSI(*Enum->GetDisplayNameTextByValue(value).ToString()));  
}

TArray<uint8> UInkList::ElementsOf(const UEnum* Enum) const {
  TArray<uint8> ret;
  if (!Enum) {
    UE_LOG(InkCpp, Warning, TEXT("Failed to provide enum for elements of!"));
    return ret;
  }
  FString enumName = Enum->GetFName().ToString();

  int num = Enum->NumEnums();
  std::string str(TCHAR_TO_ANSI(*enumName));
  for(auto itr = list_data->begin(str.c_str()); itr != list_data->end(); ++itr)
  {
    bool hit = false;
    const FString flag(ANSI_TO_TCHAR((*itr).flag_name));
    UE_LOG(InkCpp, Warning, TEXT("Looking for flag: '%s'"), *flag);
    for(int i = 0; i < num; ++i) {
      FString enumStr = Enum->GetDisplayNameTextByIndex(i).ToString();
      UE_LOG(InkCpp, Warning, TEXT("\tenum: %s"), *enumStr);
      if (enumStr.EndsWith(flag)) {
        ret.Add(i);
        hit = true;
        break;
      }
    }
    if (!hit) {
      UE_LOG(InkCpp, Warning, TEXT("Failed to find list value '%s' in enum '%s'"), *flag, *enumName);
    }
  }

  return ret;
}

TArray<FString> UInkList::ElementsOfAsString(const UEnum* Enum) const {
  TArray<FString> ret;

  FString EnumName = Enum->GetFName().ToString();
  for(auto itr = list_data->begin(TCHAR_TO_ANSI(*EnumName)); itr != list_data->end(); ++itr)
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
