#pragma once
#include "pch.h"

struct FVector {
	float X, Y, Z;

	FVector() : X(0.f), Y(0.f), Z(0.f) {}
	FVector(float X, float Y, float Z) : X(X), Y(Y), Z(Z) {}
	FVector(float InF) : X(InF), Y(InF), Z(InF) { }
	float Size() const { return sqrtf(X * X + Y * Y + Z * Z); }
	float DistTo(const FVector& V) const { return (*this - V).Size(); }
	FVector operator-(const FVector& other) const { return FVector(X - other.X, Y - other.Y, Z - other.Z); }
};

struct FVector2D {
	float X, Y;
};

struct FRotator {
	float Pitch, Yaw, Roll;
};

struct FLinearColor {
	float R, G, B, A;
	FLinearColor() : R(0.f), G(0.f), B(0.f), A(0.f) {};
	FLinearColor(float R, float G, float B, float A) : R(R), G(G), B(B), A(A) {};
};

template<typename T>
struct TArray {
	T* Data;
	int Count;
	int Size;
};

struct FString : TArray<wchar_t> {

	inline FString()
	{
	}

	FString(const wchar_t* other)
	{
		Size = Count = *other ? std::wcslen(other) + 1 : 0;

		if (Count)
		{
			Data = const_cast<wchar_t*>(other);
		}
	};

	inline bool IsValid() const
	{
		return Data != nullptr;
	}

	inline const wchar_t* c_str() const
	{
		return Data;
	}
};

struct FNameEntryHandle {
	uint32_t Block = 0;
	uint32_t Offset = 0;

	FNameEntryHandle(uint32_t block, uint32_t offset) : Block(block), Offset(offset) {};
	FNameEntryHandle(uint32_t id) : Block(id >> 16), Offset(id & 65535) {};
	operator uint32_t() const { return (Block << 16 | Offset); }
};

struct FNameEntry {
	uint16_t bIsWide : 1;
	uint16_t LowercaseProbeHash : 5;
	uint16_t Len : 10;
	union
	{
		char AnsiName[1024];
		wchar_t	WideName[1024];
	};

	std::string String();
};

struct FNamePool
{
	BYTE Lock[8];
	uint32_t CurrentBlock;
	uint32_t CurrentByteCursor;
	BYTE* Blocks[8192];

	FNameEntry* GetEntry(FNameEntryHandle handle) const;
};

struct FName {
	uint32_t Index;
	uint32_t Number;

	std::string GetName() const;
};

struct TUObjectArray
{
	BYTE** Objects;
	BYTE* PreAllocatedObjects;
	uint32_t MaxElements;
	uint32_t NumElements;
	uint32_t MaxChunks;
	uint32_t NumChunks;

	class UObject* GetByIndex(uint32_t id)
	{
		if (id >= NumElements) return nullptr;
		uint64_t chunkIndex = id / 65536;
		if (chunkIndex >= NumChunks) return nullptr;
		auto chunk = Objects[chunkIndex];
		if (!chunk) return nullptr;
		uint32_t withinChunkIndex = id % 65536 * 24;
		auto item = *reinterpret_cast<UObject**>(chunk + withinChunkIndex);
		return item;
	}
};

class UClass;
class UObject
{
public:
	UObject(UObject* addr) { *this = addr; }
	static inline TUObjectArray* GObjects = nullptr;
	void** Vtable; // 0x0
	uint32_t ObjectFlags; // 0x8
	uint32_t InternalIndex; // 0xC
	UClass* Class; // 0x10
	FName Name; // 0x18
	UObject* Outer; // 0x20

	std::string GetName() const;

	std::string GetFullName() const;

	template<typename T>
	static T* FindObject(const std::string& name)
	{
		for (int i = 0; i < GObjects->NumElements; ++i)
		{
			auto object = GObjects->GetByIndex(i);

			if (object == nullptr)
			{
				continue;
			}

			if (object->GetFullName() == name)
			{
				return static_cast<T*>(object);
			}
		}
		return nullptr;
	}

	static UClass* FindClass(const std::string& name)
	{
		return FindObject<UClass>(name);
	}

	template<typename T>
	static T* GetObjectCasted(uint32_t index)
	{
		return static_cast<T*>(GObjects->GetByIndex(index));
	}

	bool IsA(UClass* cmp) const;

	static UClass* StaticClass()
	{
		static auto ptr = UObject::FindObject<UClass>("Class CoreUObject.Object");
		return ptr;
	}
};

inline void ProcessEvent(void* obj, void* function, void* parms)
{
	auto vtable = *reinterpret_cast<void***>(obj);
	reinterpret_cast<void(*)(void*, void*, void*)>(vtable[68])(obj, function, parms);
}

// Class CoreUObject.Field
// Size: 0x30 (Inherited: 0x28)
class UField : public UObject
{
public:
	UField* Next;
};

// Class CoreUObject.Struct
// Size: 0xb0 (Inherited: 0x30)
class UStruct : public UField {
public:
	PAD(0x10); // 0x30(0x10)
	UStruct* SuperStruct; // 0x40(0x8)
	PAD(0x68); // 0x48(0x80)
};

// Class CoreUObject.Class
// Size: 0x230 (Inherited: 0xb0)
struct UClass : public UStruct {
	PAD(0x180); // 0xb0(0x180)
};

// Class Engine.Canvas
// Size: 0x2d0 (Inherited: 0x28)
struct UCanvas : UObject {
	PAD(0x2A8); // 0x28(0x2A8)

	struct FVector2D K2_TextSize(struct UFont* RenderFont, struct FString RenderText, struct FVector2D Scale); // Function Engine.Canvas.K2_TextSize
	struct FVector2D K2_StrLen(struct UFont* RenderFont, struct FString RenderText); // Function Engine.Canvas.K2_StrLen
	void K2_DrawText(struct UFont* RenderFont, struct FString RenderText, struct FVector2D ScreenPosition, struct FVector2D Scale, struct FLinearColor RenderColor, float Kerning, struct FLinearColor ShadowColor, struct FVector2D ShadowOffset, bool bCentreX, bool bCentreY, bool bOutlined, struct FLinearColor OutlineColor); // Function Engine.Canvas.K2_DrawText
};

// Class Engine.Actor
// Size: 0x220 (Inherited: 0x28)
struct AActor : UObject {
	PAD(0x1F8); // 0x28(0x1F8)

	struct FRotator K2_GetActorRotation(); // Function Engine.Actor.K2_GetActorRotation
	struct FVector K2_GetActorLocation(); // Function Engine.Actor.K2_GetActorLocation
};

// Class Engine.Pawn
// Size: 0x280 (Inherited: 0x220)
struct APawn : AActor {
	PAD(0x20); // 0x220(0x20)
	struct APlayerState* PlayerState; // 0x240(0x08)
	PAD(0x08); // 0x248(0x08)
	struct AController* LastHitBy; // 0x250(0x08)
	struct AController* Controller; // 0x258(0x08)
	PAD(0x20); // 0x260(0x20)
};

// Class Engine.PlayerCameraManager
// Size: 0x2740 (Inherited: 0x220)
struct APlayerCameraManager : AActor {
	PAD(0x2520); // 0x220
};

// Class Engine.Controller
// Size: 0x298 (Inherited: 0x220)
struct AController : AActor {
	PAD(0x8); // 0x220(0x08)
	struct APlayerState* PlayerState; // 0x228(0x08)
	PAD(0x20); // 0x230(0x20)
	struct APawn* Pawn; // 0x250(0x08)
	PAD(0x8); // 0x258(0x08)
	struct ACharacter* Character; // 0x260(0x08)
	PAD(0x30); // 0x268(0x30)
	
	struct APawn* K2_GetPawn();
};

// Class Engine.PlayerController
// Size: 0x570 (Inherited: 0x298)
struct APlayerController : AController {
	PAD(0x20); // 0x298(0x08)
	struct APlayerCameraManager* PlayerCameraManager; // 0x2b8(0x08)

	bool ProjectWorldLocationToScreen(struct FVector& WorldLocation, struct FVector2D& ScreenLocation, bool bPlayerViewportRelative); // Function Engine.PlayerController.ProjectWorldLocationToScreen
};

// Class Engine.Player
// Size: 0x48 (Inherited: 0x28)
struct UPlayer : UObject {
	PAD(0x8); // 0x28(0x08)
	struct APlayerController* PlayerController; // 0x30(0x08)
};

// Class Engine.PlayerState
// Size: 0x320 (Inherited: 0x220)
struct APlayerState : AActor {
	PAD(0x60); // 0x220(0x04)
	struct APawn* PawnPrivate; // 0x280(0x08)
	PAD(0x78); // 0x288
	struct FString PlayerNamePrivate; // 0x300(0x10)
};

// Class Engine.GameStateBase
// Size: 0x270 (Inherited: 0x220)
struct AGameStateBase : AActor {
	PAD(0x18); // 0x220
	struct TArray<struct APlayerState*> PlayerArray; // 0x238(0x10)
};

// Class Engine.GameInstance
// Size: 0x198 (Inherited: 0x28)
struct UGameInstance : UObject {
	PAD(0x10); // 0x28(0x10)
	struct TArray<struct UPlayer*> LocalPlayers; // 0x38(0x10)
};

// Class Engine.LevelActorContainer
// Size: 0x38 (Inherited: 0x28)
struct ULevelActorContainer : UObject {
	struct TArray<struct AActor*> Actors; // 0x28(0x10)
};

// Class Engine.Level
// Size: 0x298 (Inherited: 0x28)
struct ULevel : UObject {
	PAD(0x90); // 0x28(0x90)
	struct UWorld* OwningWorld; // 0xb8(0x08)
	PAD(0x18); // 0xC0(0x18)
	struct ULevelActorContainer* ActorCluster; // 0xd8(0x08)
	PAD(0x1B8); // 0xE0(0x1B8)
};

// Class Engine.World
// Size: 0x710 (Inherited: 0x28)
struct UWorld : UObject {
	PAD(0x8); // 0x28(0x08)
	struct ULevel* PersistentLevel; // 0x30(0x08)
	PAD(0xE8); // 0x38(0xE8)
	struct AGameStateBase* GameState; // 0x120(0x08)
	PAD(0x58); // 0x128(0x58)
	struct UGameInstance* OwningGameInstance; // 0x180(0x08)
	PAD(0x588); // 0x188(0x588)
};

// Class Engine.ScriptViewportClient
// Size: 0x38 (Inherited: 0x28)
struct UScriptViewportClient : UObject {
	PAD(0x10); // 0x28(0x10)
};

// Class Engine.GameViewportClient
// Size: 0x330 (Inherited: 0x38)
struct UGameViewportClient : UScriptViewportClient {
	PAD(0x40); // 0x38(0x08)
	struct UWorld* World; // 0x78(0x08)
	struct UGameInstance* GameInstance; // 0x80(0x08)
	PAD(0x2D8); // 0x88(0x2D8)
};

// Class Engine.Engine
// Size: 0xde0 (Inherited: 0x28)
struct UEngine : UObject {
	PAD(0x8); // 0x28(0x08)
	struct UFont* TinyFont; // 0x30(0x08)
	PAD(0x18); // 0x38(0x18)
	struct UFont* SmallFont; // 0x50(0x08)
	PAD(0x18); // 0x58(0x18)
	struct UFont* MediumFont; // 0x70(0x08)
	PAD(0x18); // 0x78(0x18)
	struct UFont* LargeFont; // 0x90(0x08)
	PAD(0x18); // 0x98(0x18)
	struct UFont* SubtitleFont; // 0xb0(0x08)
	PAD(0x6C8); // 0xb8(0x6C8)
	struct UGameViewportClient* GameViewport; // 0x780(0x08)
};

// Class Engine.GameplayStatics
// Size: 0x28 (Inherited: 0x28)
struct UGameplayStatics : UObject {

	void GetAllActorsOfClass(struct UObject* WorldContextObject, struct UClass* ActorClass, struct TArray<struct AActor*>* OutActors); // Function Engine.GameplayStatics.GetAllActorsOfClass // (Final|Native|Static|Public|HasOutParms|BlueprintCallable) // @ game+0x36eb890
};

extern UEngine** Engine;
extern FNamePool* NamePoolData;
extern TUObjectArray* ObjObjects;

bool EngineInit();