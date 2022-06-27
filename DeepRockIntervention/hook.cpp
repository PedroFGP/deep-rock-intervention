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
FVector2D healthPos{ 10.f, 22.f };
FVector2D armorPos{ 10.f, 34.f };
FVector2D texturePos{ 10.f, 46.f };
FVector2D textureSize{ 40.f, 40.f };

FLinearColor enemyColorInvis{ 1.f, 0.f, 0.f, 1.f };
FLinearColor enemyColorVisi{ 1.f, 1.f, 0.f, 1.f };

void DrawBoundingBox(UCanvas* canvas, APlayerController* localController, AActor* actor, int healthValue);

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

		if (localCamera)
		{
			canvas->K2_DrawText(TitleFont, FString(L"Deep Rock Intervention"), titlePos, scale, color, false, shadow, scale, false, false, true, outline);
			//canvas->K2_DrawTexture(texturePos, textureSize, colorTexture);
			//canvas->K2_DrawLine({ 10.f, 96.f }, { 40.f, 96.f }, 2.0f, colorTexture);
			//canvas->K2_DrawBox({ 150.f, 10.f }, { 40.f, 40.f }, 2.0f, colorTexture);
			//canvas->K2_DrawPolygon({ 150.f, 150.f }, { 40.f, 40.f }, 16, colorTexture);

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
					auto closestLoc = FVector(0, 0, 0);

					for (int i = 0; i < level->Actors.Count; i++)
					{
						auto currentActor = level->Actors.Data[i];

						if (currentActor && currentActor->IsA(AEnemyDeepPathfinderCharacter::StaticClass()))
						{
							auto enemy = (AEnemyDeepPathfinderCharacter*)currentActor;

							if (!enemy->HealthComponent->IsAlive())
								continue;

							//DrawBones(canvas, localController, enemy);

							//DrawBoundingBox(canvas, localController, currentActor, (int)enemy->HealthComponent->GetHealth());

							auto minersManual = gameData->MinersManual;

							auto location = enemy->K2_GetActorLocation();

							if (minersManual)
							{
								auto enemyName = gameData->MinersManual->GetNameFromEnemyId(enemy->SpawnedFromDescriptor->EnemyID);

								FVector2D screen;
								if (localController->ProjectWorldLocationToScreen(location, screen, true))
								{
									canvas->K2_DrawText(Font, enemyName, screen, scale, color, false, shadow, scale, true, true, true, outline);
								}
							}
						}
					}

					/*if (closestLoc.X != 0.0f && closestLoc.Y != 0.0f)
					{
						auto delta = UKismetMathLibrary::NormalizedDeltaRotator(UKismetMathLibrary::FindLookAtRotation(cameraLoc, closestLoc), cameraRot);
						localController->AddYawInput(delta.Yaw);
						localController->AddPitchInput(delta.Pitch);
					}*/
				}
			}

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

					if (pawn->IsA(APlayerCharacter::StaticClass()))
					{
						screen.Y += 15.0f;
						auto pawnCharacter = (APlayerCharacter*)pawn;
						std::wstring health = L"Health: ";
						health += std::to_wstring(pawnCharacter->HealthComponent->GetHealth());
						canvas->K2_DrawText(TitleFont, FString(health.c_str()), screen, scale, color, false, shadow, scale, true, false, true, outline);
					}
				};
			}
		}
	}

	PostRenderOriginal(viewport, canvas);
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

void DrawBoundingBox(UCanvas* canvas, APlayerController* localController, AActor* actor, int healthValue)
{
	auto const camera = localController->PlayerCameraManager;
	auto cameraLoc = camera->GetCameraLocation();

	FVector origin, extends;
	actor->GetActorBounds(true, origin, extends, false);
	const FVector location = actor->K2_GetActorLocation();

	extends = extends * 0.9f;

	FVector one = origin; one.X -= extends.X; one.Y -= extends.Y; one.Z -= extends.Z;
	FVector two = origin; two.X += extends.X; two.Y -= extends.Y; two.Z -= extends.Z;
	FVector three = origin; three.X += extends.X; three.Y += extends.Y; three.Z -= extends.Z;
	FVector four = origin; four.Y += extends.Y; four.X -= extends.X; four.Z -= extends.Z;

	FVector five = one; five.Z += extends.Z * 2;
	FVector six = two; six.Z += extends.Z * 2;
	FVector seven = three; seven.Z += extends.Z * 2;
	FVector eight = four; eight.Z += extends.Z * 2;

	FVector2D s1, s2, s3, s4, s5, s6, s7, s8, healthPos;
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

	if (!localController->ProjectWorldLocationToScreen(location, healthPos, true))
		return;

	const bool bVisible = localController->LineOfSightTo(actor, &cameraLoc, false);

	auto finalColor = bVisible ? enemyColorVisi : enemyColorInvis;

	canvas->K2_DrawLine(s1, s2, 1.0f, finalColor);
	canvas->K2_DrawLine(s2, s3, 1.0f, finalColor);
	canvas->K2_DrawLine(s3, s4, 1.0f, finalColor);
	canvas->K2_DrawLine(s4, s1, 1.0f, finalColor);

	canvas->K2_DrawLine(s5, s6, 1.0f, finalColor);
	canvas->K2_DrawLine(s6, s7, 1.0f, finalColor);
	canvas->K2_DrawLine(s7, s8, 1.0f, finalColor);
	canvas->K2_DrawLine(s8, s5, 1.0f, finalColor);

	canvas->K2_DrawLine(s1, s5, 1.0f, finalColor);
	canvas->K2_DrawLine(s2, s6, 1.0f, finalColor);
	canvas->K2_DrawLine(s3, s7, 1.0f, finalColor);
	canvas->K2_DrawLine(s4, s8, 1.0f, finalColor);

	std::wstring health = L"Health: ";
	health += std::to_wstring(healthValue);
	canvas->K2_DrawText(TitleFont, FString(health.c_str()), healthPos, scale, finalColor, false, shadow, scale, false, false, true, outline);
}

bool CheatInit()
{
	auto engine = *Engine;
	if (!engine) return false;

	Font = engine->SubtitleFont;
	if (!Font) return false;

	TitleFont = engine->MediumFont;
	if (!TitleFont) return false;

	auto viewport = engine->GameViewport;
	if (!viewport) return false;

	auto vtable = viewport->Vtable;
	if (!vtable) return false;

	UGameFunctionLibrary::Init();

	UKismetTextLibrary::Init();

	UKismetMathLibrary::Init();

	Address = vtable + 0x63;
	PostRenderOriginal = reinterpret_cast<decltype(PostRenderOriginal)>(Address[0]);

	auto hook = &PostRenderHook;
	return PatchMem(Address, &hook, 8);
}

void CheatRemove()
{
	PatchMem(Address, &PostRenderOriginal, 8);
}