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

void UCanvas::K2_DrawText(struct UFont* RenderFont, struct FString RenderText, struct FVector2D ScreenPosition, struct FVector2D Scale, struct FLinearColor RenderColor, float Kerning, struct FLinearColor ShadowColor, struct FVector2D ShadowOffset, bool bCentreX, bool bCentreY, bool bOutlined, struct FLinearColor OutlineColor)
{
	static auto fn = UObject::FindObject<UObject>("Function Engine.Canvas.K2_DrawText");
	struct {
		void* RenderFont;
		FString RenderText;
		FVector2D ScreenPosition;
		FVector2D Scale;
		FLinearColor RenderColor;
		float Kerning;
		FLinearColor ShadowColor;
		FVector2D ShadowOffset;
		bool bCentreX;
		bool bCentreY;
		bool bOutlined;
		FLinearColor OutlineColor;
	} parms;
	parms = { RenderFont , RenderText, ScreenPosition, Scale, RenderColor, Kerning, ShadowColor, ShadowOffset, bCentreX, bCentreY, bOutlined, OutlineColor };
	ProcessEvent(this, fn, &parms);
}

FVector AActor::K2_GetActorLocation()
{
	static auto fn = UObject::FindObject<UObject>("Function Engine.Actor.K2_GetActorLocation");
	struct {
		FVector ReturnValue;
	} parms;
	ProcessEvent(this, fn, &parms);
	return parms.ReturnValue;
}

APawn* AController::K2_GetPawn()
{
	static auto fn = UObject::FindObject<UObject>("Function Engine.Controller.K2_GetPawn");
	struct {
		APawn* ReturnValue;
	} parms;
	ProcessEvent(this, fn, &parms);
	return parms.ReturnValue;
}

bool APlayerController::ProjectWorldLocationToScreen(FVector& WorldLocation, FVector2D& ScreenLocation, bool bPlayerViewportRelative)
{
	static auto fn = UObject::FindObject<UObject>("Function Engine.PlayerController.ProjectWorldLocationToScreen");
	struct {
		FVector WorldLocation;
		FVector2D ScreenLocation;
		bool bPlayerViewportRelative;
		bool ReturnValue;
	} parms;
	parms = { WorldLocation, ScreenLocation, bPlayerViewportRelative };
	ProcessEvent(this, fn, &parms);
	ScreenLocation = parms.ScreenLocation;
	return parms.ReturnValue;
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