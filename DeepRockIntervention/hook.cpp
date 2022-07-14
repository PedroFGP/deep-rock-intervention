#include "pch.h"

void** Address;
struct UFont* Font;
struct UFont* TitleFont;
TArray<AActor*> actors;

void(__thiscall* PostRenderOriginal)(UGameViewportClient*, UCanvas*);

FVector2D scale{ 1.f, 1.f };
FLinearColor color{ 1.f, 0.43f, 0.f, 1.f };
FLinearColor colorTexture{ 1.f, 1.f, 0.f, 1.f };
FLinearColor shadow{ 0.f, 0.f, 0.f, 0.f };
FLinearColor outline{ 0.f, 0.f, 0.f, 1.f };
FVector2D titlePos{ 10.f, 10.f };
FVector2D rotationPos{ 10.f, 25.f };
FVector2D targetRotationPos{ 10.f, 40.f };
FVector2D targetScalePos{ 10.f, 55.f };
FVector ZeroVector3D{ 0.f, 0.f, 0.f };
FVector origin, extends;

FLinearColor enemyColorInvis{ 1.f, 0.f, 0.f, 1.f };
FLinearColor enemyColorVisi{ 1.f, 1.f, 0.f, 1.f };
FLinearColor aimbotColor{ 0.f, 1.f, 0.f, 1.f };

void DrawBones(UCanvas* canvas, APlayerController* localController, AEnemyDeepPathfinderCharacter* enemy);

void Draw2DBoundingBox(UCanvas* canvas, APlayerController* localController, AActor* actor, bool isVisible);

void Draw3DBoundingBox(UCanvas* canvas, APlayerController* localController, AActor* actor, FLinearColor color, FRotator rotation);

void Draw3DBoundingBoxV2(UCanvas* canvas, APlayerController* localController, AActor* actor, FLinearColor color, FRotator rotation);

void Aimbot(UObject* world, TArray<AActor*> actors, APlayerController* localController, FVector cameraLocation, FRotator cameraRotation, bool originBased = true);

void DrawNames(UCanvas* canvas, APlayerController* localController, AActor* actor, UMinersManual* minerManual, FVector origin, FLinearColor color);

void DrawPlayerNames(UCanvas* canvas, APlayerController* localController, TArray<APlayerState*> players, APawn* localPawn);

void RemoveRecoil(AItem* equippedItem);

void AlterRateOfFire(AItem* equippedItem);

void PostRenderHook(UGameViewportClient* viewport, UCanvas* canvas)
{
	auto world = viewport->World;
	auto game = world->OwningGameInstance;
	auto state = world->GameState;
	auto level = world->PersistentLevel;

	if (state)
	{
		auto localPlayer = game->LocalPlayers.Data[0];
		auto localController = localPlayer->PlayerController;
		auto localCamera = localController->PlayerCameraManager;

		if (localCamera && localController)
		{
			canvas->K2_DrawText(TitleFont, FString(L"Deep Rock Intervention"), titlePos, scale, color, false, shadow, { 2.f, 2.f }, false, false, true, outline);
			canvas->K2_DrawText(TitleFont, FString(aimbotActive ? L"Aimbot: ON" : L"Aimbot: OFF"), targetRotationPos, scale, color, false, shadow, { 2.f, 2.f }, false, false, true, outline);
			canvas->K2_DrawText(TitleFont, FString(boundingBoxActive ? L"Bounding Boxes: ON" : L"Bounding Boxes: OFF"), targetScalePos, scale, color, false, shadow, { 2.f, 2.f }, false, false, true, outline);
			auto rotation = localController->ControlRotation;

			std::wstring rotationString = L"Pitch: ";
			rotationString += std::to_wstring(rotation.Pitch);
			rotationString += L" ,Yaw: ";
			rotationString += std::to_wstring(rotation.Yaw);
			rotationString += L" ,Roll: ";
			rotationString += std::to_wstring(rotation.Roll);
			canvas->K2_DrawText(TitleFont, FString(rotationString.c_str()), rotationPos, scale, color, false, shadow, scale, false, false, true, outline);

			auto localPawn = localController->K2_GetPawn();
			auto players = state->PlayerArray;

			if (localPawn && localPawn->IsA(APlayerCharacter::StaticClass()))
			{
				auto localPlayerCharacter = (APlayerCharacter*)localPawn;

				auto level = world->PersistentLevel;

				if (level)
				{
					auto const camera = localController->PlayerCameraManager;
					auto cameraLoc = camera->GetCameraLocation();
					auto cameraRot = camera->GetCameraRotation();
					auto gameData = UGameFunctionLibrary::GetFSDGameData();
					auto minersManual = gameData->MinersManual;

					auto equippedItem = localPlayerCharacter->GetEquippedItem();

					RemoveRecoil(equippedItem);
					AlterRateOfFire(equippedItem);

					/*if (ammoDrivenWeapon->IsA(AWPN_AssaultRifle_C::StaticClass()))
					{
						auto rifle = (AWPN_AssaultRifle_C*)ammoDrivenWeapon;

						static bool once = false;

						if (rifle->HitScan && !once)
						{
							rifle->HitScan->SpreadPerShot = 0.0f;
							rifle->HitScan->MaxSpread = 0.0f;
							rifle->HitScan->MinSpread = 0.0f;
							rifle->HitScan->VerticalSpreadMultiplier = 0.0f;
							rifle->HitScan->MaxVerticalSpread = 0.0f;
							rifle->HitScan->HorizontalSpredMultiplier = 0.0f;
							rifle->HitScan->MaxHorizontalSpread = 0.0f;
							once = true;
						}
					}*/

					if (aimbotActive)
						Aimbot(world, level->Actors, localController, cameraLoc, cameraRot);

					for (int i = 0; i < level->Actors.Count; i++)
					{
						auto currentActor = level->Actors.Data[i];

						if (currentActor && currentActor->IsA(AEnemyDeepPathfinderCharacter::StaticClass()))
						{
							auto enemy = (AEnemyDeepPathfinderCharacter*)currentActor;

							if (!enemy->HealthComponent->IsAlive())
								continue;

							currentActor->GetActorBounds(true, origin, extends, false);

							struct FHitResult hitResult;
							memset(&hitResult, 0, sizeof(FHitResult));
							auto hit = UKismetSystemLibrary::LineTraceSingle(world, cameraLoc, origin, hitResult);

							auto color = hit ? enemyColorInvis : enemyColorVisi;

							if (boundingBoxActive)
								Draw3DBoundingBoxV2(canvas, localController, currentActor, color, enemy->Controller->ControlRotation);

							//DrawNames(canvas, localController, currentActor, minersManual, origin, color);
						}
					}
				}
			}
		}
	}

	PostRenderOriginal(viewport, canvas);
}



void Aimbot(UObject* world, TArray<AActor*> actors, APlayerController* localController, FVector cameraLocation, FRotator cameraRotation, bool originBased)
{
	FVector aimPos, enemyOrigin, enemyExtends, closestPoint{ 0.0f, 0.0f, 0.0f };
	FVector2D s1, closestScreenPoint{ 0.0f, 0.0f };
	FVector2D screenCenter{ 960.0f, 540.0f };
	auto bestTarget = false;

	for (int i = 0; i < actors.Count; i++)
	{
		auto currentActor = actors.Data[i];

		if (currentActor && currentActor->IsA(AEnemyDeepPathfinderCharacter::StaticClass()))
		{
			auto enemy = (AEnemyDeepPathfinderCharacter*)currentActor;

			if (!enemy->HealthComponent->IsAlive())
				continue;

			if (originBased)
			{
				currentActor->GetActorBounds(true, aimPos, enemyExtends, false);

				struct FHitResult hitResult;
				memset(&hitResult, 0, sizeof(FHitResult));
				auto hit = UKismetSystemLibrary::LineTraceSingle(world, cameraLocation, aimPos, hitResult);

				if (hit)
					continue;

				if (!localController->ProjectWorldLocationToScreen(aimPos, s1, true))
					continue;

				if (closestScreenPoint.DistTo(screenCenter) > s1.DistTo(screenCenter))
				{
					closestPoint = aimPos;
					closestScreenPoint = s1;
				}
			}
			else
			{
				if (enemy->Mesh)
				{
					const FMatrix comp2world = enemy->Mesh->K2_GetComponentToWorld().ToMatrixWithScale();

					for (auto i = 0; i < enemy->Mesh->CachedComponentSpaceTransforms.Size; i++)
					{
						if (enemy->Mesh->GetBone(i, comp2world, aimPos))
						{
							struct FHitResult hitResult;
							memset(&hitResult, 0, sizeof(FHitResult));
							auto hit = UKismetSystemLibrary::LineTraceSingle(world, cameraLocation, aimPos, hitResult);

							if (hit)
								continue;

							if (!localController->ProjectWorldLocationToScreen(aimPos, s1, true))
								continue;

							if (closestScreenPoint.DistTo(screenCenter) > s1.DistTo(screenCenter))
							{
								closestPoint = aimPos;
								closestScreenPoint = s1;
							}
						}
					}
				}
			}
		}
	}

	auto delta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLocation, closestPoint), cameraRotation);

	if (closestPoint.X != 0.0f && closestPoint.Y != 0.0f && closestPoint.Z != 0.0f)
	{
		localController->AddPitchInput(delta.Pitch);
		localController->AddYawInput(delta.Yaw);
	}
}

void DrawBones(UCanvas* canvas, APlayerController* localController, AEnemyDeepPathfinderCharacter* enemy)
{
	if (enemy->Mesh)
	{
		const FMatrix comp2world = enemy->Mesh->K2_GetComponentToWorld().ToMatrixWithScale();

		for (auto i = 0; i < enemy->Mesh->CachedComponentSpaceTransforms.Size; i++)
		{
			FVector pos;
			if (enemy->Mesh->GetBone(i, comp2world, pos))
			{
				FVector2D screen;
				if (!localController->ProjectWorldLocationToScreen(pos, screen, true)) continue;
				//auto boneName = enemy->Mesh->GetBoneName(i).GetName();
				std::wstring boneIndex = L"[";
				boneIndex += std::to_wstring(i) + L"]";
				canvas->K2_DrawText(TitleFont, FString(boneIndex.c_str()), screen, scale, color, false, shadow, scale, true, false, true, outline);
			};
		}
	}
}

void Draw2DBoundingBox(UCanvas* canvas, APlayerController* localController, AActor* actor, bool isVisible)
{
	FVector origin, extends;
	actor->GetActorBounds(true, origin, extends, false);
	const FVector location = actor->K2_GetActorLocation();

	extends = extends * 0.8f;

	FVector one = origin;
	one.X -= extends.X;
	one.Y -= extends.Y;
	one.Z -= extends.Z;

	FVector two = origin;
	two.X += extends.X;
	two.Y -= extends.Y;
	two.Z -= extends.Z;

	FVector three = origin;
	three.X += extends.X;
	three.Y += extends.Y;
	three.Z -= extends.Z;

	FVector four = origin;
	four.Y += extends.Y;
	four.X -= extends.X;
	four.Z -= extends.Z;

	FVector five = one;
	five.Z += extends.Z * 2;

	FVector six = two;
	six.Z += extends.Z * 2;

	FVector seven = three;
	seven.Z += extends.Z * 2;

	FVector eight = four;
	eight.Z += extends.Z * 2;

	FVector2D s1, s2, s3, s4, s5, s6, s7, s8;
	if (!localController->ProjectWorldLocationToScreen(one, s1, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(two, s2, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(three, s3, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(four, s4, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(five, s5, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(six, s6, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(seven, s7, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(eight, s8, true))
		return;

	float xArray[] = { s1.X, s2.X, s3.X, s4.X, s5.X, s6.X, s7.X, s8.X };
	float yArray[] = { s1.Y, s2.Y, s3.Y, s4.Y, s5.Y, s6.Y, s7.Y, s8.Y };
	float minX, maxX, minY, maxY;

	std::pair<float*, float*> minMaxX = minmaxfloat(xArray, sizeof xArray / sizeof * xArray);
	std::pair<float*, float*> minMaxY = minmaxfloat(yArray, sizeof yArray / sizeof * yArray);

	minX = *minMaxX.first;
	maxX = *minMaxX.second;
	minY = *minMaxY.first;
	maxY = *minMaxY.second;

	auto finalColor = isVisible ? enemyColorVisi : enemyColorInvis;

	auto finalPost = FVector2D(minX, minY);
	auto size = FVector2D(maxX - minX, maxY - minY);

	canvas->K2_DrawBox(finalPost, size, 2.0f, finalColor);
}


void Draw3DBoundingBox(UCanvas* canvas, APlayerController* localController, AActor* actor, FLinearColor color, FRotator rotation)
{
	FVector origin, extends;
	actor->GetActorBounds(true, origin, extends, false);
	
	FVector one = origin;
	one.X -= extends.X;
	one.Y -= extends.Y;
	one.Z -= extends.Z;

	FVector two = origin;
	two.X += extends.X;
	two.Y -= extends.Y;
	two.Z -= extends.Z;

	FVector three = origin;
	three.X += extends.X;
	three.Y += extends.Y;
	three.Z -= extends.Z;

	FVector four = origin;
	four.Y += extends.Y;
	four.X -= extends.X;
	four.Z -= extends.Z;

	FVector five = one;
	five.Z += extends.Z * 2;

	FVector six = two;
	six.Z += extends.Z * 2;

	FVector seven = three;
	seven.Z += extends.Z * 2;

	FVector eight = four;
	eight.Z += extends.Z * 2;

	FVector oneR = one, twoR = two, threeR = three, fourR = four, fiveR = five, sixR = six, sevenR = seven, eightR = eight;
	/*RotatePointOverAngles(one, rotation, oneR);
	RotatePointOverAngles(two, rotation, twoR);
	RotatePointOverAngles(three, rotation, threeR);
	RotatePointOverAngles(four, rotation, fourR);
	RotatePointOverAngles(five, rotation, fiveR);
	RotatePointOverAngles(six, rotation, sixR);
	RotatePointOverAngles(seven, rotation, sevenR);
	RotatePointOverAngles(eight, rotation, eightR);*/
	//rotate(one, rotation, oneR);
	//rotate(two, rotation, twoR);
	//rotate(three, rotation, threeR);
	//rotate(four, rotation, fourR);
	//rotate(five, rotation, fiveR);
	//rotate(six, rotation, sixR);
	//rotate(seven, rotation, sevenR);
	//rotate(eight, rotation, eightR);

	//oneR = oneR + origin;
	//twoR = twoR + origin;
	//threeR = threeR + origin;
	//fourR = fourR + origin;
	//fiveR = fiveR + origin;
	//sixR = sixR + origin;
	//sevenR = sevenR + origin;
	//eightR = eightR + origin;

	FVector2D s1, s2, s3, s4, s5, s6, s7, s8;
	if (!localController->ProjectWorldLocationToScreen(oneR, s1, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(twoR, s2, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(threeR, s3, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(fourR, s4, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(fiveR, s5, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(sixR, s6, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(sevenR, s7, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(eightR, s8, true))
		return;

	canvas->K2_DrawLine(s1, s2, 1.0f, color);
	canvas->K2_DrawLine(s2, s3, 1.0f, color);
	canvas->K2_DrawLine(s3, s4, 1.0f, color);
	canvas->K2_DrawLine(s4, s1, 1.0f, color);

	canvas->K2_DrawLine(s5, s6, 1.0f, color);
	canvas->K2_DrawLine(s6, s7, 1.0f, color);
	canvas->K2_DrawLine(s7, s8, 1.0f, color);
	canvas->K2_DrawLine(s8, s5, 1.0f, color);

	canvas->K2_DrawLine(s1, s5, 1.0f, color);
	canvas->K2_DrawLine(s2, s6, 1.0f, color);
	canvas->K2_DrawLine(s3, s7, 1.0f, color);
	canvas->K2_DrawLine(s4, s8, 1.0f, color);
}

void Draw3DBoundingBoxV2(UCanvas* canvas, APlayerController* localController, AActor* actor, FLinearColor color, FRotator rotation)
{
	FVector origin, extends;
	actor->GetActorBounds(true, origin, extends, false);

	FVector one;
	one.X -= extends.X;
	one.Y -= extends.Y;
	one.Z -= extends.Z;

	FVector two;
	two.X += extends.X;
	two.Y -= extends.Y;
	two.Z -= extends.Z;

	FVector three;
	three.X += extends.X;
	three.Y += extends.Y;
	three.Z -= extends.Z;

	FVector four;
	four.Y += extends.Y;
	four.X -= extends.X;
	four.Z -= extends.Z;

	FVector five = one;
	five.Z += extends.Z * 2;

	FVector six = two;
	six.Z += extends.Z * 2;

	FVector seven = three;
	seven.Z += extends.Z * 2;

	FVector eight = four;
	eight.Z += extends.Z * 2;

	FVector oneR = one, twoR = two, threeR = three, fourR = four, fiveR = five, sixR = six, sevenR = seven, eightR = eight;

	rotate(one, rotation, oneR);
	rotate(two, rotation, twoR);
	rotate(three, rotation, threeR);
	rotate(four, rotation, fourR);
	rotate(five, rotation, fiveR);
	rotate(six, rotation, sixR);
	rotate(seven, rotation, sevenR);
	rotate(eight, rotation, eightR);

	oneR = oneR + origin;
	twoR = twoR + origin;
	threeR = threeR + origin;
	fourR = fourR + origin;
	fiveR = fiveR + origin;
	sixR = sixR + origin;
	sevenR = sevenR + origin;
	eightR = eightR + origin;

	FVector2D s1, s2, s3, s4, s5, s6, s7, s8;
	if (!localController->ProjectWorldLocationToScreen(oneR, s1, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(twoR, s2, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(threeR, s3, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(fourR, s4, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(fiveR, s5, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(sixR, s6, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(sevenR, s7, true))
		return;

	if (!localController->ProjectWorldLocationToScreen(eightR, s8, true))
		return;

	canvas->K2_DrawLine(s1, s2, 1.0f, color);
	canvas->K2_DrawLine(s2, s3, 1.0f, color);
	canvas->K2_DrawLine(s3, s4, 1.0f, color);
	canvas->K2_DrawLine(s4, s1, 1.0f, color);

	canvas->K2_DrawLine(s5, s6, 1.0f, color);
	canvas->K2_DrawLine(s6, s7, 1.0f, color);
	canvas->K2_DrawLine(s7, s8, 1.0f, color);
	canvas->K2_DrawLine(s8, s5, 1.0f, color);

	canvas->K2_DrawLine(s1, s5, 1.0f, color);
	canvas->K2_DrawLine(s2, s6, 1.0f, color);
	canvas->K2_DrawLine(s3, s7, 1.0f, color);
	canvas->K2_DrawLine(s4, s8, 1.0f, color);
}



void DrawNames(UCanvas* canvas, APlayerController* localController, AActor* actor, UMinersManual* minerManual, FVector origin, FLinearColor color)
{
	auto enemy = (AEnemyDeepPathfinderCharacter*)actor;

	if (minerManual && enemy->SpawnedFromDescriptor)
	{
		auto enemyName = minerManual->GetNameFromEnemyId(enemy->SpawnedFromDescriptor->EnemyID);

		FVector2D screen;
		if (!localController->ProjectWorldLocationToScreen(origin, screen, true))
			return;

		canvas->K2_DrawText(Font, enemyName, screen, scale, color, false, shadow, scale, true, true, true, outline);
	}
}

void DrawPlayerNames(UCanvas* canvas, APlayerController* localController, TArray<APlayerState*> players, APawn* localPawn)
{
	for (auto i = 0; i < players.Count; i++)
	{
		auto player = players.Data[i];
		auto pawn = player->PawnPrivate;

		if (!pawn || pawn == localPawn) continue;

		auto location = pawn->K2_GetActorLocation();
		FVector2D screen;
		if (localController->ProjectWorldLocationToScreen(location, screen, true))
		{
			auto name = player->PlayerNamePrivate;
			canvas->K2_DrawText(Font, name, screen, scale, color, false, shadow, scale, true, true, true, outline);
		};
	}
}

void RemoveRecoil(AItem* equippedItem)
{
	if (equippedItem && equippedItem->IsA(AAmmoDrivenWeapon::StaticClass()))
	{
		auto ammoDrivenWeapon = (AAmmoDrivenWeapon*)equippedItem;

		if (ammoDrivenWeapon->ShotCost != 0)
		{	
			ammoDrivenWeapon->RecoilSettings.RecoilPitch.Max = 0.0f;
			ammoDrivenWeapon->RecoilSettings.RecoilPitch.Min = 0.0f;
			ammoDrivenWeapon->RecoilSettings.RecoilYaw.Max = 0.0f;
			ammoDrivenWeapon->RecoilSettings.RecoilYaw.Min = 0.0f;
			ammoDrivenWeapon->RecoilSettings.RecoilRoll.Max = 0.0f;
			ammoDrivenWeapon->RecoilSettings.RecoilRoll.Min = 0.0f;
			ammoDrivenWeapon->ShotCost = 0;
		}
	}
}

void AlterRateOfFire(AItem* equippedItem)
{
	if (equippedItem && equippedItem->IsA(AAmmoDrivenWeapon::StaticClass()))
	{
		auto ammoDrivenWeapon = (AAmmoDrivenWeapon*)equippedItem;

		static bool once = false;
		if (!once)
		{
			ammoDrivenWeapon->RateOfFire = 2 * ammoDrivenWeapon->RateOfFire;
			once = !once;
		}
	}
}


bool CheatInit()
{
	auto engine = *Engine;
	if (!engine) return false;

	Font = engine->SubtitleFont;
	if (!Font) return false;

	TitleFont = engine->LargeFont;
	if (!TitleFont) return false;

	auto viewport = engine->GameViewport;
	if (!viewport) return false;

	auto vtable = viewport->Vtable;
	if (!vtable) return false;

	UGameFunctionLibrary::Init();

	UKismetTextLibrary::Init();

	UKismetMathLibrary::Init();

	UKismetSystemLibrary::Init();

	Address = vtable + 0x63;
	PostRenderOriginal = reinterpret_cast<decltype(PostRenderOriginal)>(Address[0]);

	auto hook = &PostRenderHook;
	return PatchMem(Address, &hook, 8);
}

void CheatRemove()
{
	PatchMem(Address, &PostRenderOriginal, 8);
}