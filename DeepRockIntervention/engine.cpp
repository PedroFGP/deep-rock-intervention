#include "pch.h"

UEngine** Engine = nullptr;
TUObjectArray* ObjObjects = nullptr;
FNamePool* NamePoolData = nullptr;
extern bool aimbotActive = false;

FMatrix FTransform::ToMatrixWithScale() const

{
	FMatrix OutMatrix;
	OutMatrix.M[3][0] = Translation.X;
	OutMatrix.M[3][1] = Translation.Y;
	OutMatrix.M[3][2] = Translation.Z;

	const float x2 = Rotation.X + Rotation.X;
	const float y2 = Rotation.Y + Rotation.Y;
	const float z2 = Rotation.Z + Rotation.Z;
	{
		const float xx2 = Rotation.X * x2;
		const float yy2 = Rotation.Y * y2;
		const float zz2 = Rotation.Z * z2;

		OutMatrix.M[0][0] = (1.0f - (yy2 + zz2)) * Scale3D.X;
		OutMatrix.M[1][1] = (1.0f - (xx2 + zz2)) * Scale3D.Y;
		OutMatrix.M[2][2] = (1.0f - (xx2 + yy2)) * Scale3D.Z;
	}
	{
		const float yz2 = Rotation.Y * z2;
		const float wx2 = Rotation.W * x2;

		OutMatrix.M[2][1] = (yz2 - wx2) * Scale3D.Z;
		OutMatrix.M[1][2] = (yz2 + wx2) * Scale3D.Y;
	}
	{
		const float xy2 = Rotation.X * y2;
		const float wz2 = Rotation.W * z2;

		OutMatrix.M[1][0] = (xy2 - wz2) * Scale3D.Y;
		OutMatrix.M[0][1] = (xy2 + wz2) * Scale3D.X;
	}
	{
		const float xz2 = Rotation.X * z2;
		const float wy2 = Rotation.W * y2;

		OutMatrix.M[2][0] = (xz2 + wy2) * Scale3D.Z;
		OutMatrix.M[0][2] = (xz2 - wy2) * Scale3D.X;
	}

	OutMatrix.M[0][3] = 0.0f;
	OutMatrix.M[1][3] = 0.0f;
	OutMatrix.M[2][3] = 0.0f;
	OutMatrix.M[3][3] = 1.0f;

	return OutMatrix;
}

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

void rotate(FVector& point, FRotator& rotation, FVector& out)
{
	out.X = 0;
	out.Y = 0;
	out.Z = 0;

	FRotator rRotation {0.0f, 0.0f, 0.0f};
	rRotation.Pitch = D2R(rotation.Pitch);
	rRotation.Yaw = D2R(rotation.Yaw);
	rRotation.Roll = D2R(rotation.Roll);

	auto cosa = cos(rRotation.Yaw);
	auto sina = sin(rRotation.Yaw);

	auto cosb = cos(rRotation.Pitch);
	auto sinb = sin(rRotation.Pitch);

	auto cosc = cos(rRotation.Roll);
	auto sinc = sin(rRotation.Roll);

	auto Axx = cosa * cosb;
	auto Axy = cosa * sinb * sinc - sina * cosc;
	auto Axz = cosa * sinb * cosc + sina * sinc;

	auto Ayx = sina * cosb;
	auto Ayy = sina * sinb * sinc + cosa * cosc;
	auto Ayz = sina * sinb * cosc - cosa * sinc;

	auto Azx = -sinb;
	auto Azy = cosb * sinc;
	auto Azz = cosb * cosc;

	out.X = Axx * point.X + Axy * point.Y + Axz * point.Z;
	out.Y = Ayx * point.X + Ayy * point.Y + Ayz * point.Z;
	out.Z = Azx * point.X + Azy * point.Y + Azz * point.Z;
}