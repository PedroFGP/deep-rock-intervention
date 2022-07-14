#pragma once
#include "pch.h"

typedef __m128	VectorRegister;
typedef __m128i VectorRegisterInt;
typedef __m128d VectorRegisterDouble;

#define VectorMultiply(a,b) _mm_mul_ps(a,b)
#define SHUFFLEMASK(A0,A1,B2,B3) ( (A0) | ((A1)<<2) | ((B2)<<4) | ((B3)<<6) )
#define VectorReplicate( Vec, ElementIndex ) _mm_shuffle_ps( Vec, Vec, SHUFFLEMASK(ElementIndex,ElementIndex,ElementIndex,ElementIndex) )
#define VectorMultiplyAdd( Vec1, Vec2, Vec3 ) _mm_add_ps( _mm_mul_ps(Vec1, Vec2), Vec3 )

struct FMatrix
{
public:
	union
	{
		__declspec(align(16)) float M[4][4];
	};

	__forceinline void VectorMatrixMultiply(void* Result, const void* Matrix1, const void* Matrix2)
	{
		const VectorRegister* A = (const VectorRegister*)Matrix1;
		const VectorRegister* B = (const VectorRegister*)Matrix2;
		VectorRegister* R = (VectorRegister*)Result;
		VectorRegister Temp, R0, R1, R2, R3;

		// First row of result (Matrix1[0] * Matrix2).
		Temp = VectorMultiply(VectorReplicate(A[0], 0), B[0]);
		Temp = VectorMultiplyAdd(VectorReplicate(A[0], 1), B[1], Temp);
		Temp = VectorMultiplyAdd(VectorReplicate(A[0], 2), B[2], Temp);
		R0 = VectorMultiplyAdd(VectorReplicate(A[0], 3), B[3], Temp);

		// Second row of result (Matrix1[1] * Matrix2).
		Temp = VectorMultiply(VectorReplicate(A[1], 0), B[0]);
		Temp = VectorMultiplyAdd(VectorReplicate(A[1], 1), B[1], Temp);
		Temp = VectorMultiplyAdd(VectorReplicate(A[1], 2), B[2], Temp);
		R1 = VectorMultiplyAdd(VectorReplicate(A[1], 3), B[3], Temp);

		// Third row of result (Matrix1[2] * Matrix2).
		Temp = VectorMultiply(VectorReplicate(A[2], 0), B[0]);
		Temp = VectorMultiplyAdd(VectorReplicate(A[2], 1), B[1], Temp);
		Temp = VectorMultiplyAdd(VectorReplicate(A[2], 2), B[2], Temp);
		R2 = VectorMultiplyAdd(VectorReplicate(A[2], 3), B[3], Temp);

		// Fourth row of result (Matrix1[3] * Matrix2).
		Temp = VectorMultiply(VectorReplicate(A[3], 0), B[0]);
		Temp = VectorMultiplyAdd(VectorReplicate(A[3], 1), B[1], Temp);
		Temp = VectorMultiplyAdd(VectorReplicate(A[3], 2), B[2], Temp);
		R3 = VectorMultiplyAdd(VectorReplicate(A[3], 3), B[3], Temp);

		// Store result
		R[0] = R0;
		R[1] = R1;
		R[2] = R2;
		R[3] = R3;
	};

	__forceinline FMatrix operator*(const FMatrix& Other) {
		FMatrix Result;
		VectorMatrixMultiply(&Result, this, &Other);
		return Result;
	};
};

class FVector 
{
public:
	float X, Y, Z;

	FVector() : X(0.f), Y(0.f), Z(0.f) {}
	FVector(float X, float Y, float Z) : X(X), Y(Y), Z(Z) {}
	FVector(float InF) : X(InF), Y(InF), Z(InF) { }
	float Size() const { return sqrtf(X * X + Y * Y + Z * Z); }
	float DistTo(const FVector& V) const { return (*this - V).Size(); }

	FVector operator+(const FVector& other) const { return FVector(X + other.X, Y + other.Y, Z + other.Z); }
	FVector operator-(const FVector& other) const { return FVector(X - other.X, Y - other.Y, Z - other.Z); }
	FVector operator*(const FVector& V) const { return FVector(X * V.X, Y * V.Y, Z * V.Z); }
	FVector operator/(const FVector& V) const { return FVector(X / V.X, Y / V.Y, Z / V.Z); }
};

class FVector2D 
{
public:
	float X, Y;

	FVector2D() : X(0.f), Y(0.f) {}
	FVector2D(float X, float Y) : X(X), Y(Y) {}
	float Size() const { return sqrtf(X * X + Y * Y); }
	float DistTo(const FVector2D& V) const { return (*this - V).Size(); }

	FVector2D operator + (const FVector2D& other) const { return FVector2D(X + other.X, Y + other.Y); }
	FVector2D operator- (const FVector2D& other) const { return FVector2D(X - other.X, Y - other.Y); }
	FVector2D operator* (float scalar) const { return FVector2D(X * scalar, Y * scalar); }
	FVector2D operator/ (float scalar) const { return FVector2D(X / scalar, Y / scalar); }
	friend bool operator>(const FVector2D& one, const FVector2D& two) { return one.X > two.X && one.Y > two.Y; }
	friend bool operator<(const FVector2D& one, const FVector2D& two) { return one.X < two.X && one.Y < two.Y; }
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

	FMatrix ToMatrixWithScale() const;
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

class FTextData
{
public:
	PAD(0x28);
	FString Text;
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

class FKey
{
public:
	FName KeyName;
	PAD(0x10);
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

	void GetActorBounds(bool bOnlyCollidingComponents, struct FVector& Origin, struct FVector& BoxExtent, bool bIncludeFromChildActors)
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.Actor.GetActorBounds");
		struct
		{
			bool bOnlyCollidingComponents = false;
			FVector Origin;
			FVector BoxExtent;
			bool bIncludeFromChildActors = false;
		} params;

		params.bOnlyCollidingComponents = bOnlyCollidingComponents;
		params.bIncludeFromChildActors = bIncludeFromChildActors;

		ProcessEvent(this, fn, &params);

		Origin = params.Origin;
		BoxExtent = params.BoxExtent;
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

// Class FSD.EnemyDescriptor
// Size: 0x220 (Inherited: 0x30)
class UEnemyDescriptor
{
public:
	PAD(0x30);
	void* EnemyID; // 0x30(0x08)
};

// Class FSD.FSDPawn
// Size: 0x2f8 (Inherited: 0x280)
class AFSDPawn : public APawn 
{
public:
	PAD(0x58); // 0x280(0x58)
	struct UEnemyDescriptor* SpawnedFromDescriptor; // 0x2d8(0x08)
	PAD(0x18); // 0x2e0(0x18)

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
	struct FTextData* Name; // 0x38(0x18)
};

// Class FSD.SimpleObjectInfoComponent
// Size: 0x170 (Inherited: 0xb0)
class USimpleObjectInfoComponent
{
public:
	PAD(0xB0); // 0x00(0xb0)
	struct FTextData* InGameName; // 0xb0(0x18)
	struct FTextData* InGameDescription; // 0xc8(0x18)
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

// Class Engine.KismetTextLibrary
// Size: 0x28 (Inherited: 0x28)
class UKismetTextLibrary : public UObject
{
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Engine.KismetTextLibrary");
	}

	FString Conv_TextToString(struct FText& InText)
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.KismetTextLibrary.Conv_TextToString");
		FString string;
		ProcessEvent(defaultObj, fn, &string);
		return string;
	}
};

// Class FSD.DeepPathfinderCharacter
// Size: 0x388 (Inherited: 0x2f8)
class ADeepPathfinderCharacter : public AFSDPawn 
{
public:
	PAD(0x18); // 0x2f8(0x18)
	struct USkeletalMeshComponent* Mesh; // 0x310(0x08)
	PAD(0x70); // 0x318(0x70)
};

// Class FSD.EnemyDeepPathfinderCharacter
// Size: 0x3a8 (Inherited: 0x388)
class AEnemyDeepPathfinderCharacter : public ADeepPathfinderCharacter
{
public:
	PAD(0x08); // 0x388(0x08)
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

// Class FSD.AFlyingBug
// Size: 0x428 (Inherited: 0x3a8)
class AAFlyingBug : public AEnemyDeepPathfinderCharacter 
{
public:
	struct UEnemyComponent* EnemyComponent; // 0x3a8(0x08)
};

// Class FSD.EnemyMinersManualData
// Size: 0x180 (Inherited: 0x30)
class UEnemyMinersManualData 
{
public:
	PAD(0x30); // 0x00(0x30)
	void* EnemyID;
	struct FTextData* Name; // 0x38(0x08)
	PAD(0x10); // 0x40(0x10)
	struct FTextData* RichDescription; // 0x50(0x18)
	PAD(0x10); // 0x58(0x10)
	struct TArray<struct FText> GameplayTips; // 0x68(0x10)
	BYTE Family; // 0x78(0x01)
	BYTE EnemyType; // 0x79(0x01)
	PAD(0x106); // 0x7A(0x106)
};

// Class FSD.MinersManual
// Size: 0x1a0 (Inherited: 0x30)
class UMinersManual
{
public:
	PAD(0xC0); // 0x00(0xC0)
	struct TArray<struct UEnemyMinersManualData*> Enemies; // 0xc0(0x10)

	FString GetNameFromEnemyId(void* EnemyID)
	{
		for (int i = 0; i < Enemies.Size; i++)
		{
			auto enemyData = Enemies.Data[i];

			if (enemyData && enemyData->EnemyID == EnemyID)
			{
				return enemyData->Name->Text;
			}
		}

		return FString();
	}
};

// Class FSD.GameData
// Size: 0x988 (Inherited: 0x28)
class UGameData : public UObject
{
public:
	PAD(0x3B8); // 0x28(0x3B8)
	struct UMinersManual* MinersManual; // 0x3e0(0x08)
	PAD(0x5A0); // 0x3e8(0x5A0)

	static UClass* StaticClass()
	{
		static UClass* ptr = 0;

		if (!ptr)
		{
			ptr = UObject::FindObject<UClass>("Class FSD.GameData");
		}

		return ptr;
	}
};

// Class FSD.GameFunctionLibrary
// Size: 0x28 (Inherited: 0x28)
class UGameFunctionLibrary
{
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class FSD.GameFunctionLibrary");
	}

	static UGameData* GetFSDGameData()
	{
		static auto fn = UObject::FindObject<UObject>("Function FSD.GameFunctionLibrary.GetFSDGameData");
		UGameData* gameData;
		ProcessEvent(defaultObj, fn, &gameData);
		return gameData;
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


// Class FSD.Item
// Size: 0x340 (Inherited: 0x220)
class AItem : public AActor 
{
public:
	PAD(0x120);
};

// Class FSD.AnimatedItem
// Size: 0x390 (Inherited: 0x340)
class AAnimatedItem : public AItem 
{
public:
	PAD(0x50);
};

// ScriptStruct FSD.RandRange
// Size: 0x08 (Inherited: 0x00)
class FRandRange 
{
public:
	float Min; // 0x00(0x04)
	float Max; // 0x04(0x04)
};

// ScriptStruct FSD.RecoilSettings
// Size: 0x28 (Inherited: 0x00)
class FRecoilSettings 
{
public:
	struct FRandRange RecoilRoll; // 0x00(0x08)
	struct FRandRange RecoilPitch; // 0x08(0x08)
	struct FRandRange RecoilYaw; // 0x10(0x08)
	bool CanRecoilDown; // 0x18(0x01)
	PAD(0x3); // 0x19(0x03)
	float SpringStiffness; // 0x1c(0x04)
	float CriticalDampening; // 0x20(0x04)
	float Mass; // 0x24(0x04)
};

// Enum FSD.EAmmoWeaponState
enum class EAmmoWeaponState : uint8_t 
{
	Equipping = 0,
	Ready = 1,
	Cycling = 2,
	Reloading = 3,
	BurstCycling = 4,
	EAmmoWeaponState_MAX = 5
};

// Class FSD.WeaponFireComponent
// Size: 0x110 (Inherited: 0xb0)
class UWeaponFireComponent : public UActorComponent
{
public:
	PAD(0x60); // 0xb0(0x80)
};

// Class FSD.DamageImpulse
// Size: 0x40 (Inherited: 0x30)
class UDamageImpulse
{
public:
	PAD(0x30); // 0x00(0x30)
	float ImpactForce; // 0x30(0x04)
	float UpwardForceScale; // 0x34(0x04)
};

// Class FSD.DamageComponent
// Size: 0x1d8 (Inherited: 0xb0)
class UDamageComponent : public UActorComponent
{
public:
	PAD(0x60); // 0xb0(0x50)
	struct UDamageImpulse* DamageImpulse; // 0x110(0x08)
	float Damage; // 0x118(0x04)
	float ArmorDamageMultiplier; // 0x11c(0x04)
	bool ShattersArmor; // 0x120(0x01)
	PAD(0x13); // 0x121(0x07)
	float WeakpointDamageMultiplier; // 0x134(0x04)
	float FrozenDamageBonusScale; // 0x138(0x04)
	float friendlyFireModifier; // 0x13c(0x04)
	float SelfFriendlyFireMultiplier; // 0x140(0x04)
	PAD(0x8); // 0x144(0x08)
	bool StaggerOnlyOnWeakpointHit; // 0x14c(0x01)
	PAD(0x3); // 0x14d(0x03)
	float StaggerChance; // 0x150(0x04)
	float StaggerDuration; // 0x154(0x04)
	float FearFactor; // 0x158(0x04)
	PAD(0x44); // 0x15c(0x04)
	bool UseAreaOfEffect; // 0x1a0(0x01)
	PAD(0x3); // 0x1a1(0x03)
	float RadialDamage; // 0x1a4(0x04)
	bool NoFriendlyFireFromRadial; // 0x1a8(0x01)
	PAD(0xF); // 0x1a9(0x07)
	float MinDamagePct; // 0x1b8(0x04)
	float DamageRadius; // 0x1bc(0x04)
	float MaxDamageRadius; // 0x1c0(0x04)
	PAD(0x14); // 0x1c4(0x14)
};

// Class FSD.HitscanBaseComponent
// Size: 0x238 (Inherited: 0x110)
class UHitscanBaseComponent : public UWeaponFireComponent
{
public:
	float SpreadPerShot; // 0x130(0x04)
	PAD(0x4); // 0x134(0x04)
	struct UDamageComponent* DamageComponent; // 0x138(0x08)
	bool UseDamageComponent; // 0x140(0x01)
	PAD(0x3); // 0x141(0x03)
	float Damage; // 0x144(0x04)
	float ArmorDamageMultiplier; // 0x148(0x04)
	PAD(0xC); // 0x14c(0x04)
	float WeakpointDamageMultiplier; // 0x158(0x04)
	int32_t MaxPenetrations; // 0x15c(0x04)
	PAD(0x4); // 0x160(0x03)
	float friendlyFireModifier; // 0x164(0x04)
	bool UseDynamicSpread; // 0x168(0x01)
	PAD(0x3); // 0x169(0x03)
	float MinSpread; // 0x16c(0x04)
	float MinSpreadWhenMoving; // 0x170(0x04)
	float MinSpreadWhenSprinting; // 0x174(0x04)
	float MaxSpread; // 0x178(0x04)
	PAD(0x8C); // 0x17c(0x04)
	float SpreadRecoveryPerSecond; // 0x208(0x04)
	PAD(0x8); // 0x20c(0x08)
	float VerticalSpreadMultiplier; // 0x214(0x04)
	float HorizontalSpredMultiplier; // 0x218(0x04)
	float MaxVerticalSpread; // 0x21c(0x04)
	float MaxHorizontalSpread; // 0x220(0x04)
	PAD(0x8); // 0x224(0x04)
	float RicochetChance; // 0x22c(0x04)
	bool RicochetOnWeakspotOnly; // 0x230(0x01)
	PAD(0x3); // 0x231(0x03)
	float RicochetMaxRange; // 0x234(0x04)
};

// Class FSD.AmmoDrivenWeapon
// Size: 0x6e0 (Inherited: 0x390)
class AAmmoDrivenWeapon : public AAnimatedItem 
{
public:
	PAD(0x80); // 0x390(0x80)
	struct UWeaponFireComponent* WeaponFire; // 0x410(0x08)
	PAD(0x220); // 0x418(0x220)
	int32_t MaxAmmo; // 0x638(0x04)
	int32_t ClipSize; // 0x63c(0x04)
	int32_t ShotCost; // 0x640(0x04)
	float RateOfFire; // 0x644(0x04)
	int32_t BurstCount; // 0x648(0x04)
	float BurstCycleTime; // 0x64c(0x04)
	float ReloadDuration; // 0x650(0x04)
	int32_t AmmoCount; // 0x654(0x04)
	int32_t ClipCount; // 0x658(0x04)
	PAD(0x34); // 0x65C(0x34)
	struct FRecoilSettings RecoilSettings; // 0x690(0x28)
	bool HasAutomaticFire; // 0x6b8(0x01)
	bool IsFiring; // 0x6b9(0x01)
	PAD(0x18); // 0x6ba(0x18)
	enum class EAmmoWeaponState WeaponState; // 0x6d2(0x01)
	PAD(0xd); // 0x6d3(0x0d)

	static UClass* StaticClass()
	{
		static UClass* ptr = 0;

		if (!ptr)
		{
			ptr = UObject::FindObject<UClass>("Class FSD.AmmoDrivenWeapon");
		}

		return ptr;
	}
};

// BlueprintGeneratedClass WPN_AssaultRifle.WPN_AssaultRifle_C
// Size: 0x750 (Inherited: 0x700)
class AWPN_AssaultRifle_C
{
public:
	PAD(0x718); // 0x00(0x718)
	struct UDamageComponent* Damage; // 0x718(0x08)
	PAD(0x20); // 0x720(0x20)
	struct UHitscanBaseComponent* HitScan; // 0x740(0x08)
	PAD(0x8); // 0x748(0x08)

	static UClass* StaticClass()
	{
		static UClass* ptr = 0;

		if (!ptr)
		{
			ptr = UObject::FindObject<UClass>("BlueprintGeneratedClass WPN_AssaultRifle.WPN_AssaultRifle_C");
		}

		return ptr;
	}
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

	struct AItem* GetEquippedItem()
	{
		static auto fn = UObject::FindObject<UObject>("Function FSD.PlayerCharacter.GetEquippedItem");

		AItem* ReturnValue;
		ProcessEvent(this, fn, &ReturnValue);
		return ReturnValue;
	}

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

// Class Engine.KismetMathLibrary
// Size: 0x28 (Inherited: 0x28)
class UKismetMathLibrary : public UObject
{
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Engine.KismetMathLibrary");
	}

	static struct FRotator NormalizedDeltaRotator(struct FRotator A, struct FRotator B)
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.KismetMathLibrary.NormalizedDeltaRotator");

		struct
		{
			struct FRotator                A;
			struct FRotator                B;
			struct FRotator                ReturnValue;
		} params;

		params.A = A;
		params.B = B;

		ProcessEvent(defaultObj, fn, &params);

		return params.ReturnValue;
	}

	static FRotator FindLookAtRotation(const FVector& Start, const FVector& Target) 
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.KismetMathLibrary.FindLookAtRotation");

		struct {
			FVector Start;
			FVector Target;
			FRotator ReturnValue;
		} params;

		params.Start = Start;
		params.Target = Target;
		ProcessEvent(defaultObj, fn, &params);
		return params.ReturnValue;
	}
};

// Class Engine.PlayerCameraManager
// Size: 0x2740 (Inherited: 0x220)
class APlayerCameraManager : public AActor
{
public:
	PAD(0x2520); // 0x220

	struct FRotator GetCameraRotation()
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.PlayerCameraManager.GetCameraRotation");
		FRotator CompToWorld;
		ProcessEvent(this, fn, &CompToWorld);
		return CompToWorld;
	};

	struct FVector GetCameraLocation()
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.PlayerCameraManager.GetCameraLocation");
		FVector CompToWorld;
		ProcessEvent(this, fn, &CompToWorld);
		return CompToWorld;
	}
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

	bool GetBone(const uint32_t id, const FMatrix& componentToWorld, FVector& pos) {
		if (id >= CachedComponentSpaceTransforms.Count) return false;
		const auto& bone = CachedComponentSpaceTransforms.Data[id];
		auto boneMatrix = bone.ToMatrixWithScale();
		auto world = boneMatrix * componentToWorld;
		pos = { world.M[3][0], world.M[3][1], world.M[3][2] };
		return true;
	}
};

// Class Engine.SceneComponent
// Size: 0x200 (Inherited: 0xb0)
class USceneComponent : public UActorComponent 
{
public:
	PAD(0x6C); // 0xb0(0x6C)
	struct FVector RelativeLocation; // 0x11c(0x0c)
	struct FRotator RelativeRotation; // 0x128(0x0c)
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
	struct USceneComponent* TransformComponent; // 0x268(0x08)
	PAD(0x18); // 0x270(0x18)
	struct FRotator ControlRotation; // 0x288(0x0c)
	PAD(0x4); // 0x294(0x04)
	
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
	PAD(0x16C); // 0x2C0(0x16C)
	float InputYawScale; // 0x42c(0x04)
	float InputPitchScale; // 0x430(0x04)
	PAD(0x13C); // 0x434(0x13C)

	bool ProjectWorldLocationToScreen(struct FVector WorldLocation, struct FVector2D& ScreenLocation, bool bPlayerViewportRelative)
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

	void AddYawInput(float Val) {
		Val = Val / InputYawScale;
		static auto fn = UObject::FindObject<UObject>("Function Engine.PlayerController.AddYawInput");
		ProcessEvent(this, fn, &Val);
	}

	void AddPitchInput(float Val) {
		Val = Val / InputPitchScale;
		static auto fn = UObject::FindObject<UObject>("Function Engine.PlayerController.AddPitchInput");
		ProcessEvent(this, fn, &Val);
	}
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
			ptr = UObject::FindObject<UClass>("Class FSD.FSDGameMode");
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

// Enum Engine.ETraceTypeQuery
enum class ETraceTypeQuery : uint8_t {
	TraceTypeQuery1 = 0,
	TraceTypeQuery2 = 1,
	TraceTypeQuery3 = 2,
	TraceTypeQuery4 = 3,
	TraceTypeQuery5 = 4,
	TraceTypeQuery6 = 5,
	TraceTypeQuery7 = 6,
	TraceTypeQuery8 = 7,
	TraceTypeQuery9 = 8,
	TraceTypeQuery10 = 9,
	TraceTypeQuery11 = 10,
	TraceTypeQuery12 = 11,
	TraceTypeQuery13 = 12,
	TraceTypeQuery14 = 13,
	TraceTypeQuery15 = 14,
	TraceTypeQuery16 = 15,
	TraceTypeQuery17 = 16,
	TraceTypeQuery18 = 17,
	TraceTypeQuery19 = 18,
	TraceTypeQuery20 = 19,
	TraceTypeQuery21 = 20,
	TraceTypeQuery22 = 21,
	TraceTypeQuery23 = 22,
	TraceTypeQuery24 = 23,
	TraceTypeQuery25 = 24,
	TraceTypeQuery26 = 25,
	TraceTypeQuery27 = 26,
	TraceTypeQuery28 = 27,
	TraceTypeQuery29 = 28,
	TraceTypeQuery30 = 29,
	TraceTypeQuery31 = 30,
	TraceTypeQuery32 = 31,
	TraceTypeQuery_MAX = 32,
	ETraceTypeQuery_MAX = 33
};

// Enum Engine.EDrawDebugTrace
enum class EDrawDebugTrace : uint8_t {
	None = 0,
	ForOneFrame = 1,
	ForDuration = 2,
	Persistent = 3,
	EDrawDebugTrace_MAX = 4
};

// ScriptStruct Engine.HitResult
// Size: 0x88 (Inherited: 0x00)
struct FHitResult {
	int32_t FaceIndex; // 0x00(0x04)
	float Time; // 0x04(0x04)
	float Distance; // 0x08(0x04)
	struct FVector Location; // 0x0c(0x0c)
	struct FVector ImpactPoint; // 0x18(0x0c)
	struct FVector Normal; // 0x24(0x0c)
	struct FVector ImpactNormal; // 0x30(0x0c)
	struct FVector TraceStart; // 0x3c(0x0c)
	struct FVector TraceEnd; // 0x48(0x0c)
	float PenetrationDepth; // 0x54(0x04)
	int32_t Item; // 0x58(0x04)
	char ElementIndex; // 0x5c(0x01)
	char bBlockingHit : 1; // 0x5d(0x01)
	char bStartPenetrating : 1; // 0x5d(0x01)
	PAD(0x1A); // 0x5E(0x1A)
	struct FName BoneName; // 0x78(0x08)
	struct FName MyBoneName; // 0x80(0x08)
};

extern UWorld* GWorld;

class UKismetSystemLibrary
{
private:
	static inline UClass* defaultObj;
public:
	static bool Init() {
		return defaultObj = UObject::FindObject<UClass>("Class Engine.KismetSystemLibrary");
	}

	static bool LineTraceSingle(UObject* WorldContextObject, struct FVector Start, struct FVector End, struct FHitResult& OutHit)
	{
		static auto fn = UObject::FindObject<UObject>("Function Engine.KismetSystemLibrary.LineTraceSingle");

		struct {
			UObject* WorldContextObject;
			FVector Start;
			FVector End;
			ETraceTypeQuery TraceChannel;
			bool bTraceComplex;
			struct TArray<struct AActor*> ActorsToIgnore;
			EDrawDebugTrace DrawDebugType;
			struct FHitResult OutHit;
			bool bIgnoreSelf;
			struct FLinearColor TraceColor; 
			struct FLinearColor TraceHitColor; 
			float DrawTime;
			bool ReturnValue;
		} params;

		auto color = FLinearColor();

		auto actorsToIgnore = TArray<struct AActor*>();
		params.WorldContextObject = WorldContextObject;
		params.Start = Start;
		params.End = End;
		params.TraceChannel = ETraceTypeQuery::TraceTypeQuery1;
		params.bTraceComplex = true;
		params.ActorsToIgnore = actorsToIgnore;
		params.DrawDebugType = EDrawDebugTrace::None;
		params.OutHit = OutHit;
		params.bIgnoreSelf = true;
		params.TraceColor = color;
		params.TraceHitColor = color;
		params.DrawTime = 0.0f;
		params.ReturnValue = false;

		ProcessEvent(defaultObj, fn, &params);
		OutHit = params.OutHit;
		return params.ReturnValue;
	}
};

extern UEngine** Engine;
extern FNamePool* NamePoolData;
extern TUObjectArray* ObjObjects;
extern bool aimbotActive;
extern bool boundingBoxActive;

bool EngineInit();
void RotatePointOverAngles(FVector& point, FRotator& rotation, FVector& out);
void rotate(FVector& point, FRotator& rotation, FVector& out);