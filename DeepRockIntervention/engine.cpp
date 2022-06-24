#include "pch.h"

UEngine** Engine = nullptr;
TUObjectArray* ObjObjects = nullptr;
FNamePool* NamePoolData = nullptr;

std::string FNameEntry::String()
{
	if (bIsWide) { return std::string(); }
	return { AnsiName, Len };
}

FNameEntry* FNamePool::GetEntry(FNameEntryHandle handle) const
{
	return reinterpret_cast<FNameEntry*>(Blocks[handle.Block] + 2 * static_cast<uint64_t>(handle.Offset));
}

std::string FName::GetName() const
{
	auto entry = NamePoolData->GetEntry(Index);
	auto name = entry->String();
	if (Number > 0)
	{
		name += '_' + std::to_string(Number);
	}
	auto pos = name.rfind('/');
	if (pos != std::string::npos)
	{
		name = name.substr(pos + 1);
	}
	return name;
}

std::string UObject::GetName() const
{
	return Name.GetName();
}

std::string UObject::GetFullName() const
{
	std::string name;
	for (auto outer = Outer; outer; outer = outer->Outer) { name = outer->GetName() + "." + name; }
	name = Class->GetName() + " " + name + this->GetName();
	return name;
}

bool UObject::IsA(UClass* cmp) const
{
	for (auto super = Class; super; super = static_cast<UClass*>(super->SuperStruct))
	{
		if (super == cmp)
		{
			return true;
		}
	}
	return false;
}

bool EngineInit()
{
	auto main = GetModuleHandleA(nullptr);

	static char engineSig[] = { 0x48, 0x8b, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8b, 0x88, 0x08, 0x0b, 0x00, 0x00 };
	Engine = reinterpret_cast<decltype(Engine)>(FindPointer(main, engineSig, sizeof(engineSig), 0));
	if (!Engine) return false;

	static char objSig[] = { 0x48, 0x8B, 0x05, 0x00, 0x00, 0x00, 0x00, 0x48, 0x8B, 0x0C, 0xC8, 0x48, 0x8D, 0x04, 0xD1, 0xEB };
	UObject::GObjects = reinterpret_cast<TUObjectArray*>(FindPointer(main, objSig, sizeof(objSig), 0));
	if (!UObject::GObjects) return false;

	static char poolSig[] = { 0x48, 0x8d, 0x35, 0x00, 0x00, 0x00, 0x00, 0xeb, 0x16 };
	NamePoolData = reinterpret_cast<decltype(NamePoolData)>(FindPointer(main, poolSig, sizeof(poolSig), 0));
	if (!NamePoolData) return false;

	return true;
}