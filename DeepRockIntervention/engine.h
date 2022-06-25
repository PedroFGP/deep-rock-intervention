#pragma once
#include "pch.h"

class FVector 
{
public:
	float X, Y, Z;

	FVector() : X(0.f), Y(0.f), Z(0.f) {}
	FVector(float X, float Y, float Z) : X(X), Y(Y), Z(Z) {}
	FVector(float InF) : X(InF), Y(InF), Z(InF) { }
	float Size() const { return sqrtf(X * X + Y * Y + Z * Z); }
	float DistTo(const FVector& V) const { return (*this - V).Size(); }
	FVector operator-(const FVector& other) const { return FVector(X - other.X, Y - other.Y, Z - other.Z); }
};

class FVector2D 
{
public:
	float X, Y;

	FVector2D() : X(0.f), Y(0.f) {}
	FVector2D(float X, float Y) : X(X), Y(Y) {}
};

const FVector2D ZeroVector(0.0f, 0.0f);
const FVector2D OneVector(1.0f, 1.0f);

class FRotator 
{
public:
	float Pitch, Yaw, Roll;
};

// ScriptStruct CoreUObject.Quat
// Size: 0x10 (Inherited: 0x00)
class FQuat
{
public:
	float X; // 0x00(0x04)
	float Y; // 0x04(0x04)
	float Z; // 0x08(0x04)
	float W; // 0x0c(0x04)
};

class FLinearColor 
{
public:
	float R, G, B, A;
	FLinearColor() : R(0.f), G(0.f), B(0.f), A(0.f) {};
	FLinearColor(float R, float G, float B, float A) : R(R), G(G), B(B), A(A) {};
};

// ScriptStruct CoreUObject.Transform
// Size: 0x30 (Inherited: 0x00)
class FTransform
{
public:
	FQuat Rotation; // 0x00(0x10)
	FVector Translation; // 0x10(0x0c)
	PAD(0x4); // 0x1c(0x04)
	class FVector Scale3D; // 0x20(0x0c)
	PAD(0x4); // 0x2c(0x04)
};



template<typename T>
struct TArray 
{
	T* Data;
	int Count;
	int Size;
};

class FString : public TArray<wchar_t> 
{
public:

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

class FText
{
public:
	FString* Text;
	PAD(0x10);
};

class FNameEntryHandle 
{
public:

	uint32_t Block = 0;
	uint32_t Offset = 0;

	FNameEntryHandle(uint32_t block, uint32_t offset) : Block(block), Offset(offset) {};
	FNameEntryHandle(uint32_t id) : Block(id >> 16), Offset(id & 65535) {};
	operator uint32_t() const { return (Block << 16 | Offset); }
};

class FNameEntry 
{
public:

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

class FNamePool
{
public:
	BYTE Lock[8];
	uint32_t CurrentBlock;
	uint32_t CurrentByteCursor;
	BYTE* Blocks[8192];

	FNameEntry* GetEntry(FNameEntryHandle handle) const;
};

class FName 
{
public:

	uint32_t Index;
	uint32_t Number;

	std::string GetName() const;
};

class TUObjectArray
{
public:

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
		static UClass* ptr = 0;

		if (!ptr) 
		{
			ptr = UObject::FindObject<UClass>("Class CoreUObject.Object");
		}

		return ptr;
	}
};

inline void ProcessEvent(void* obj, void* function, void* parms)
{
	auto vtable = *reinterpret_cast<void***>(obj);
	reinterpret_cast<void(*)(void*, void*, void*)>(vtable[68])(obj, function, parms);
}

// Enum Engine.EBlendMode
enum EBlendMode : uint8_t {
	BLEND_Opaque = 0,
	BLEND_Masked = 1,
	BLEND_Translucent = 2,
	BLEND_Additive = 3,
	BLEND_Modulate = 4,
	BLEND_AlphaComposite = 5,
	BLEND_AlphaHoldout = 6,
	BLEND_MAX = 7
};

// Class CoreUObject.Field
// Size: 0x30 (Inherited: 0x28)
class UField : public UObject
{
public:
	UField* Next;
};

// Class CoreUObject.Struct
// Size: 0xb0 (Inherited: 0x30)
class UStruct : public UField 
{
public:
	PAD(0x10); // 0x30(0x10)
	UStruct* SuperStruct; // 0x40(0x8)
	PAD(0x68); // 0x48(0x80)
};

// Class CoreUObject.Class
// Size: 0x230 (Inherited: 0xb0)
class UClass : public UStruct 
{
public:
	PAD(0x180); // 0xb0(0x180)
};

// Class Engine.Canvas
// Size: 0x2d0 (Inherited: 0x28)
class UCanvas : public UObject 
{
public:
	PAD(0x38); // 0x28(0x38)
	void* DefaultTexture; // 0x60(0x08)
	PAD(0x268); // 0x68(0x268)

	void K2_DrawPolygon(struct FVector2D ScreenPosition, struct FVector2D Radius, int32_t NumberOfSides, struct FLinearColor RenderColor)
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.Canvas.K2_DrawPolygon");

		struct {
			void* RenderTexture;
			FVector2D ScreenPosition;
			FVector2D Radius;
			int32_t NumberOfSides;
			FLinearColor RenderColor;
		} parms;

		parms = { this->DefaultTexture, ScreenPosition, Radius, NumberOfSides,RenderColor };

		ProcessEvent(this, fn, &parms);
	}

	void K2_DrawTexture(struct FVector2D ScreenPosition, struct FVector2D ScreenSize, struct FLinearColor RenderColor) 
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.Canvas.K2_DrawTexture");

		struct {
			void* RenderTexture;
			FVector2D ScreenPosition;
			FVector2D ScreenSize;
			FVector2D CoordinatePosition;
			FVector2D CoordinateSize;
			FLinearColor RenderColor;
			EBlendMode BlendMode;
			float Rotation;
			FVector2D PivotPoint;
		} parms;

		parms = { this->DefaultTexture, ScreenPosition, ScreenSize, ZeroVector, OneVector, RenderColor, EBlendMode::BLEND_MAX, 0.0f, ZeroVector };

		ProcessEvent(this, fn, &parms);
	}

	void K2_DrawLine(struct FVector2D ScreenPositionA, struct FVector2D ScreenPositionB, float Thickness, struct FLinearColor RenderColor)
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.Canvas.K2_DrawLine");

		struct {
			FVector2D ScreenPositionA;
			FVector2D ScreenPositionB;
			float Thickness;
			FLinearColor RenderColor;
		} parms;

		parms = { ScreenPositionA , ScreenPositionB, Thickness, RenderColor };

		ProcessEvent(this, fn, &parms);
	}

	void K2_DrawBox(struct FVector2D ScreenPosition, struct FVector2D ScreenSize, float Thickness, struct FLinearColor RenderColor)
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.Canvas.K2_DrawBox");

		struct {
			FVector2D ScreenPosition;
			FVector2D ScreenSize;
			float Thickness;
			FLinearColor RenderColor;
		} parms;

		parms = { ScreenPosition , ScreenSize, Thickness, RenderColor };

		ProcessEvent(this, fn, &parms);
	}

	FVector2D K2_TextSize(struct UFont* RenderFont, struct FString RenderText, struct FVector2D Scale)
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.Canvas.K2_TextSize");

		struct {
			UFont* RenderFont;
			FString RenderText;
			FVector2D Scale;
			FVector2D ReturnValue;
		} parms;

		parms = { RenderFont , RenderText, Scale };

		ProcessEvent(this, fn, &parms);

		return parms.ReturnValue;
	}


	void K2_DrawText(struct UFont* RenderFont, struct FString RenderText, struct FVector2D ScreenPosition, struct FVector2D Scale, struct FLinearColor RenderColor, float Kerning, struct FLinearColor ShadowColor, struct FVector2D ShadowOffset, bool bCentreX, bool bCentreY, bool bOutlined, struct FLinearColor OutlineColor)
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.Canvas.K2_DrawText");

		struct {
			UFont* RenderFont;
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
	};
};

// Class Engine.Actor
// Size: 0x220 (Inherited: 0x28)
class AActor : public UObject 
{
public:
	PAD(0x1F8); // 0x28(0x1F8)

	struct FRotator K2_GetActorRotation() 
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.Actor.K2_GetActorRotation");

		FRotator rotation;
		ProcessEvent(this, fn, &rotation);

		return rotation;
	};

	struct FVector K2_GetActorLocation()
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.Actor.K2_GetActorLocation");

		FVector location;
		ProcessEvent(this, fn, &location);

		return location;
	}

	static UClass* StaticClass()
	{
		static UClass* ptr = 0;

		if (!ptr)
		{
			ptr = UObject::FindObject<UClass>("Class Engine.Actor");
		}

		return ptr;
	}
};

// Class Engine.Pawn
// Size: 0x280 (Inherited: 0x220)
class APawn : public AActor 
{
public:
	PAD(0x20); // 0x220(0x20)
	class APlayerState* PlayerState; // 0x240(0x08)
	PAD(0x08); // 0x248(0x08)
	class AController* LastHitBy; // 0x250(0x08)
	class AController* Controller; // 0x258(0x08)
	PAD(0x20); // 0x260(0x20)

	static UClass* StaticClass()
	{
		static UClass* ptr = 0;

		if (!ptr)
		{
			ptr = UObject::FindObject<UClass>("Class Engine.Pawn");
		}

		return ptr;
	}
};

// Class FSD.FSDPawn
// Size: 0x2f8 (Inherited: 0x280)
class AFSDPawn : public APawn 
{
public:
	PAD(0x78); // 0x280(0x78)

	struct UHealthComponentBase* GetHealthComponent() 
	{
		static auto fn = UObject::FindObject<UObject>("Function FSD.FSDPawn.GetHealthComponent");
		UHealthComponentBase* health;
		ProcessEvent(this, fn, &health);
		return health;
	}
};

// Class Engine.ActorComponent
// Size: 0xb0 (Inherited: 0x28)
class UActorComponent : public UObject
{
public:
	PAD(0xB0); // 0x28(0xb0)
};

// Class FSD.CharacterRecoilComponent
// Size: 0xe0 (Inherited: 0xb0)
class UCharacterRecoilComponent : public UActorComponent
{
public:
	struct FVector Recoil; // 0xb0(0x0c)
	struct FVector RecoilVelocity; // 0xbc(0x0c)
	struct FVector PrevError; // 0xc8(0x0c)
	PAD(0x0C); // 0xd4(0x0c)
};

// Class FSD.HealthComponentBase
// Size: 0x150 (Inherited: 0xb0)
class UHealthComponentBase : public UActorComponent 
{
public:
	PAD(0x50); // 0xb0(0x50)

	bool IsAlive()
	{
		static auto fn = UObject::FindObject<UObject>("Function FSD.HealthComponentBase.IsAlive");
		bool isAlive = true;
		ProcessEvent(this, fn, &isAlive);
		return isAlive;
	}

	float GetHealth()
	{
		static auto fn = UObject::FindObject<UObject>("Function FSD.HealthComponentBase.GetHealth");
		float health = 0.0f;
		ProcessEvent(this, fn, &health);
		return health;
	}


	float GetHealthPct()
	{
		static auto fn = UObject::FindObject<UObject>("Function FSD.HealthComponentBase.GetHealthPct");
		float health = 0.0f;
		ProcessEvent(this, fn, &health);
		return health;
	}
};

// Class FSD.HealthComponent
// Size: 0x250 (Inherited: 0x150)
class UHealthComponent : public UHealthComponentBase 
{
public:
	PAD(0x50); // 0x150(0x50)
	float Damage; // 0x1a0(0x04)
	PAD(0xAC); // 0x1a4(0xAC)

	float GetArmorPct()
	{
		static auto fn = UObject::FindObject<UObject>("Function FSD.HealthComponent.GetArmorPct");
		float armorPct = 0.0f;
		ProcessEvent(this, fn, &armorPct);
		return armorPct;
	}
};

// Class FSD.EnemyHealthComponent
// Size: 0x288 (Inherited: 0x250)
class UEnemyHealthComponent : public UHealthComponent
{
public:
	PAD(0x10); // 0x250(0x0c)
	float Courage; // 0x260(0x04)
	float MaxHealth; // 0x264(0x04)
	PAD(0x20); // 0x268(0x20)
};

// Class FSD.EnemyFamily
// Size: 0x50 (Inherited: 0x30)
class UEnemyFamily
{
public:
	PAD(0x30); // 0x00(0x30)
	void* Icon; // 0x30(0x08)
	struct FText Name; // 0x38(0x18)
};

// Class FSD.SimpleObjectInfoComponent
// Size: 0x170 (Inherited: 0xb0)
class USimpleObjectInfoComponent
{
public:
	PAD(0xB0); // 0x00(0xb0)
	struct FText InGameName; // 0xb0(0x18)
	struct FText InGameDescription; // 0xc8(0x18)
};

// Class FSD.EnemyComponent
// Size: 0x1a8 (Inherited: 0x170)
class UEnemyComponent : public USimpleObjectInfoComponent 
{
public:
	struct UEnemyFamily* Family; // 0x170(0x08)
	PAD(0x10); // 0x178(0x10)
	struct FString mixerName; // 0x188(0x10)
	PAD(0x10); // 0x198(0x10)

	FString* GetFamilyName()
	{
		static auto fn = UObject::FindObject<UObject>("Function FSD.EnemyComponent.GetFamilyName");
		FText* armorPct;
		ProcessEvent(this, fn, &armorPct);
		return armorPct->Text;
	}
};

// Class FSD.EnemyPawn
// Size: 0x348 (Inherited: 0x2f8)
class AEnemyPawn : public AFSDPawn
{
public:
	PAD(0x8); // 0x2f8(0x08)
	struct UEnemyHealthComponent* Health; // 0x300(0x08)
	PAD(0x10); //struct UPawnStatsComponent* Stats; // 0x308(0x08)
	//struct UEnemyPawnAfflictionComponent* Affliction; // 0x310(0x08)
	struct UEnemyComponent* enemy; // 0x318(0x08)
	struct FName CenterMassSocketName; // 0x320(0x08)
	PAD(0x20);

	static UClass* StaticClass()
	{
		static UClass* ptr = 0;

		if (!ptr)
		{
			ptr = UObject::FindObject<UClass>("Class FSD.EnemyPawn");
		}

		return ptr;
	}
};

// Class FSD.EnemyDeepPathfinderCharacter
// Size: 0x3a8 (Inherited: 0x388)
class AEnemyDeepPathfinderCharacter
{
public:
	PAD(0x390); // 0x388(0x08)
	struct UEnemyHealthComponent* HealthComponent; // 0x390(0x08)
	PAD(0x10);// 0x398(0x10)

	static UClass* StaticClass()
	{
		static UClass* ptr = 0;

		if (!ptr)
		{
			ptr = UObject::FindObject<UClass>("Class FSD.EnemyDeepPathfinderCharacter");
		}

		return ptr;
	}
};

// Class FSD.PlayerHealthComponent
// Size: 0x390 (Inherited: 0x250)
class UPlayerHealthComponent : public UHealthComponent 
{
public:
	PAD(0x58); // 0x250(0x58)
	float MaxHealth; // 0x2a8(0x04)
	float MaxArmor; // 0x2ac(0x04)
	float ArmorDamage; // 0x2b0(0x04)
	PAD(0xDC); // 0x2B4(0xDC)
};

// Class Engine.Character
// Size: 0x4c0 (Inherited: 0x280)
class ACharacter : public APawn
{
public:
	class USkeletalMeshComponent* Mesh; // 0x280(0x08)
	PAD(0x238); // 0x288(0x238)
};


// Class FSD.PlayerCharacter
// Size: 0xe10 (Inherited: 0x4c0)
class APlayerCharacter : public ACharacter 
{
public:
	PAD(0x578); // 0x4c0(0x578)
	struct USkeletalMeshComponent* FPMesh; // 0xa38(0x08)
	PAD(0x60); // 0xa40(0x60)
	struct UPlayerHealthComponent* HealthComponent; // 0xaa0(0x08)
	PAD(0x28); // 0xaa8(0x28)
	struct UCharacterRecoilComponent* RecoilComponent; // 0xad0(0x08)
	PAD(0x338); // 0xad8(0x338)

	static UClass* StaticClass()
	{
		static UClass* ptr = 0;

		if (!ptr)
		{
			ptr = UObject::FindObject<UClass>("Class FSD.PlayerCharacter");
		}

		return ptr;
	}
};

// Class Engine.PlayerCameraManager
// Size: 0x2740 (Inherited: 0x220)
class APlayerCameraManager : public AActor
{
public:
	PAD(0x2520); // 0x220
};

// Class Engine.SkeletalMeshComponent
// Size: 0xed0 (Inherited: 0x6a0)
class USkeletalMeshComponent 
{
public:
	PAD(0x740); // 0x0(0x740)
	struct TArray<struct FTransform> CachedBoneSpaceTransforms; // 0x740(0x10)
	struct TArray<struct FTransform> CachedComponentSpaceTransforms; // 0x750(0x10)

	FName GetBoneName(int BoneIndex)
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.SkinnedMeshComponent.GetBoneName");
		struct
		{
			int BoneIndex = 0;
			FName ReturnValue;
		} params;

		params.BoneIndex = BoneIndex;
		ProcessEvent(this, fn, &params);
		return params.ReturnValue;
	}

	FTransform K2_GetComponentToWorld() 
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.SceneComponent.K2_GetComponentToWorld");
		FTransform CompToWorld;
		ProcessEvent(this, fn, &CompToWorld);
		return CompToWorld;
	}
};

// Class Engine.Controller
// Size: 0x298 (Inherited: 0x220)
class AController : public AActor
{
public:
	PAD(0x8); // 0x220(0x08)
	struct APlayerState* PlayerState; // 0x228(0x08)
	PAD(0x20); // 0x230(0x20)
	struct APawn* Pawn; // 0x250(0x08)
	PAD(0x8); // 0x258(0x08)
	struct ACharacter* Character; // 0x260(0x08)
	PAD(0x30); // 0x268(0x30)
	
	APawn* K2_GetPawn()
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.Controller.K2_GetPawn");

		APawn* pawn;
		ProcessEvent(this, fn, &pawn);

		return pawn;
	}
};

// Class Engine.PlayerController
// Size: 0x570 (Inherited: 0x298)
class APlayerController : public AController
{
public:
	PAD(0x20); // 0x298(0x08)
	struct APlayerCameraManager* PlayerCameraManager; // 0x2b8(0x08)

	bool ProjectWorldLocationToScreen(struct FVector& WorldLocation, struct FVector2D& ScreenLocation, bool bPlayerViewportRelative)
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
	};
};

// Class Engine.Player
// Size: 0x48 (Inherited: 0x28)
class UPlayer : public UObject 
{
public:
	PAD(0x8); // 0x28(0x08)
	struct APlayerController* PlayerController; // 0x30(0x08)
};

// Class Engine.PlayerState
// Size: 0x320 (Inherited: 0x220)
class APlayerState : public AActor 
{
public:
	PAD(0x60); // 0x220(0x04)
	struct APawn* PawnPrivate; // 0x280(0x08)
	PAD(0x78); // 0x288
	struct FString PlayerNamePrivate; // 0x300(0x10)
};

// Class Engine.GameModeBase
// Size: 0x2c0 (Inherited: 0x220)
class AGameModeBase : public AActor {
public:
	PAD(0xA0); //0x220(0xA0)
};

// Class Engine.GameStateBase
// Size: 0x270 (Inherited: 0x220)
class AGameStateBase : public AActor 
{
public:
	struct AGameModeBase* GameModeClass; // 0x220(0x08)
	struct AGameModeBase* AuthorityGameMode; // 0x228(0x08)
	PAD(0x08); // 0x230(0x08)
	struct TArray<struct APlayerState*> PlayerArray; // 0x238(0x10)
	PAD(0x28);
};

// Class Engine.GameMode
// Size: 0x308 (Inherited: 0x2c0)
class AGameMode : public AGameModeBase 
{
public:
	struct FName MatchState; // 0x2c0(0x08)
	PAD(0x4); // 0x2c8(0x4)
	int32_t NumSpectators; // 0x2cc(0x04)
	int32_t NumPlayers; // 0x2d0(0x04)
	int32_t NumBots; // 0x2d4(0x04)
	PAD(0x30); // 0x2d8(0x30)
};

// Class FSD.FSDGameMode
// Size: 0x4f0 (Inherited: 0x308)
class AFSDGameMode : public AGameMode 
{
public:
	PAD(0x50); // 0x308(0x50)
	void* EnemySpawnManager;//struct UEnemySpawnManager* EnemySpawnManager; // 0x358(0x08)
	PAD(0x190); // 0x360(0x190)

	static UClass* StaticClass()
	{
		static UClass* ptr = 0;

		if (!ptr)
		{
			ptr = UObject::FindObject<UClass>(" Class FSD.FSDGameMode");
		}

		return ptr;
	}
};

// Class Engine.GameInstance
// Size: 0x198 (Inherited: 0x28)
class UGameInstance : public UObject
{
public:
	PAD(0x10); // 0x28(0x10)
	struct TArray<struct UPlayer*> LocalPlayers; // 0x38(0x10)
};

// Class Engine.LevelActorContainer
// Size: 0x38 (Inherited: 0x28)
class ULevelActorContainer : public UObject 
{
public:
	struct TArray<struct AActor*> Actors; // 0x28(0x10)
};

// Class Engine.Level
// Size: 0x298 (Inherited: 0x28)
class ULevel : public UObject
{
public:
	PAD(0x70); // 0x28(0x70)
	struct TArray<struct AActor*> Actors; // 0x98(0x10)
	PAD(0x1F0); // 0xA0(0x1F0)
};

// Class Engine.LevelStreaming
// Size: 0x150 (Inherited: 0x28)
class ULevelStreaming : public UObject 
{
public:
	PAD(0x100); // 0x28(0x100)
	struct ULevel* LoadedLevel; // 0x128(0x08)
	struct ULevel* PendingUnloadLevel; // 0x130(0x08)
	PAD(0x18); // 0x138(0x18)
};

// Class Engine.World
// Size: 0x710 (Inherited: 0x28)
class UWorld : public UObject 
{
public:
	PAD(0x8); // 0x28(0x08)
	struct ULevel* PersistentLevel; // 0x30(0x08)
	PAD(0x50); // 0x38(0x50)
	struct TArray<struct ULevelStreaming*> StreamingLevels; // 0x88(0x10)
	PAD(0x88); // 0x98(0x88)
	struct AGameStateBase* GameState; // 0x120(0x08)
	PAD(0x10); // 0x128(0x10)
	struct TArray<struct ULevel*> Levels; // 0x138(0x10)
	PAD(0x38); // 0x148(0x38)
	struct UGameInstance* OwningGameInstance; // 0x180(0x08)
	PAD(0x588); // 0x188(0x588)
};

// Class Engine.ScriptViewportClient
// Size: 0x38 (Inherited: 0x28)
class UScriptViewportClient : public UObject 
{
public:
	PAD(0x10); // 0x28(0x10)
};

// Class Engine.GameViewportClient
// Size: 0x330 (Inherited: 0x38)
class UGameViewportClient : public UScriptViewportClient
{
public:
	PAD(0x40); // 0x38(0x08)
	struct UWorld* World; // 0x78(0x08)
	struct UGameInstance* GameInstance; // 0x80(0x08)
	PAD(0x2D8); // 0x88(0x2D8)
};

// Class Engine.Engine
// Size: 0xde0 (Inherited: 0x28)
class UEngine : public UObject
{
public:
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

extern UEngine** Engine;
extern FNamePool* NamePoolData;
extern TUObjectArray* ObjObjects;

bool EngineInit();