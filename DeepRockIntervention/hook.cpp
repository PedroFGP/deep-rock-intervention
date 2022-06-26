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
FLinearColor enemyColorVisi{ 0.f, 1.f, 1.f, 1.f };

void DrawBoundingBox(UCanvas* canvas, APlayerController* localController, AActor* actor);

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
				std::wstring health = L"Health: ";
				std::wstring armor = L"Armor: ";
				health += std::to_wstring(localPlayerCharacter->HealthComponent->GetHealth());
				armor += std::to_wstring(localPlayerCharacter->HealthComponent->GetArmorPct());
				canvas->K2_DrawText(TitleFont, FString(health.c_str()), healthPos, scale, color, false, shadow, scale, false, false, true, outline);
				canvas->K2_DrawText(TitleFont, FString(armor.c_str()), armorPos, scale, color, false, shadow, scale, false, false, true, outline);

				auto level = world->PersistentLevel;

				if (level)
				{
					for (int i = 0; i < level->Actors.Count; i++)
					{
						auto currentActor = level->Actors.Data[i];

						if (currentActor && currentActor->IsA(AEnemyDeepPathfinderCharacter::StaticClass()))
						{
							DrawBoundingBox(canvas, localController, currentActor);
						}
					}
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

void DrawBoundingBox(UCanvas* canvas, APlayerController* localController, AActor* actor)
{
	auto enemy = (AEnemyDeepPathfinderCharacter*)actor;
	auto const camera = localController->PlayerCameraManager;
	auto cameraLoc = camera->GetCameraLocation();

	FVector origin, extent;
	actor->GetActorBounds(true, origin, extent, false);
	const FVector location = actor->K2_GetActorLocation();

	FVector2D originPos;
	if (!localController->ProjectWorldLocationToScreen(origin, originPos, true))
		return;
	FVector2D extendedPos;
	if (!localController->ProjectWorldLocationToScreen(origin + extent, extendedPos, true))
		return;

	const bool bVisible = localController->LineOfSightTo(actor, &cameraLoc, false);

	auto finalColor = bVisible ? enemyColorVisi : enemyColorInvis;

	//headPos.X -= width / 2;

	auto size = FVector2D(abs(extendedPos.X - originPos.X), abs(extendedPos.Y - originPos.Y));

	auto finalOrigin = (originPos - (size / 2));

	canvas->K2_DrawBox(finalOrigin, size, 2.0f, finalColor);

	std::wstring health = L"Health: ";
	health += std::to_wstring((int)enemy->HealthComponent->GetHealth());
	canvas->K2_DrawText(TitleFont, FString(health.c_str()), finalOrigin, scale, finalColor, false, shadow, scale, false, false, true, outline);
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

	Address = vtable + 0x63;
	PostRenderOriginal = reinterpret_cast<decltype(PostRenderOriginal)>(Address[0]);

	auto hook = &PostRenderHook;
	return PatchMem(Address, &hook, 8);
}

void CheatRemove()
{
	PatchMem(Address, &PostRenderOriginal, 8);
}