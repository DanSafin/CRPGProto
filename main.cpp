#include "EcsEngine.hpp"
#include "GridMap.hpp"
#include "Systems.hpp"
#include "raylib.h"
#include <iostream>
#include <vector>
#include <cmath>

int main()
{
	const int windowWidth = 800;
	const int windowHeight = 600;
	InitWindow(windowWidth, windowHeight, "Malacca Kingdom CRPG - Prototype Sandbox");
	SetTargetFPS(60);
	
	// Animation stuff
	//Texture2D playerTex = LoadTexture("assets/640x480/Idle/Body/Front_Idle_Body_0001.PNG");
	Texture2D playerIdle = LoadTexture("assets/x640p_Spritesheets/Idle_Body.PNG");
	Texture2D playerRun = LoadTexture("assets/x640p_Spritesheets/Run_Body.PNG");

	float animFrameTimer = 0.0f;
	int currentAnimFrame = 0;

	// Initialize our dynamic map layer database
	GridMap gameMap(10, 10);
	gameMap.setObstacle(4, 1, TileType::OBSTACLE);
	gameMap.setObstacle(4, 2, TileType::OBSTACLE);

	Registry registry;

	Entity player1 = registry.createEntity();
	registry.assignCombatStats(player1, "Hang Tuah", 18, "Pendekar", true);
	registry.assignPosition(player1, 1, 1);
	registry.assignHealth(player1, 120);
	registry.assignWeapon(player1, "Lembing", 3, 3);
	registry.assignMovement(player1, 4);
	auto* playerMove = registry.getMovement(player1);
	auto* playerStats = registry.getCombatStats(player1);
	auto* playerPos = registry.getPosition(player1);
	gameMap.getTile(1, 1)->isOccupied = true; // Set initial footprint map vacancy status

	Entity enemy1 = registry.createEntity();
	registry.assignCombatStats(enemy1, "Megat Seri Rama", 14, "Lanun Captain", true);
	registry.assignPosition(enemy1, 4, 3);
	registry.assignHealth(enemy1, 200);
	registry.assignWeapon(enemy1, "Keris", 2, 1);
	registry.assignMovement(enemy1, 2);
	gameMap.getTile(4, 3)->isOccupied = true;
	auto* enemyPos = registry.getPosition(enemy1);
	

	Entity npc1 = registry.createEntity();
	registry.assignPosition(npc1, 1, 3);
	registry.assignHealth(npc1, 50);
	registry.assignCombatStats(npc1, "Kroni", 2, "Kroni");
	gameMap.getTile(1, 3)->isOccupied = true;
	auto* npcPos = registry.getPosition(npc1);

	std::vector<Entity> combatants = registry.getCombatants();
	size_t activeturnindex = 0;
	CombatState currentphase = CombatState::ROUND_START;
	Rectangle attackBtn = { 10.0f, 490.0f, 150.0f, 40.0f };
	bool isAttackStanceActive = false;
	Rectangle endTrnBttn = { 10.0f, 540.0f, 140.0f, 40.0f };
	GameScene activeScene = GameScene::EXPLORATION;
	std::vector<DialogueNode> dialogueTree;
	int currentDialogueNodeIndex = 0;

	//DialogueSystem
	std::vector<std::string> dialogueLines;
	size_t currentLineIndex = 0;

	auto resetGame = [&]()
		{
			// free old tiles
			gameMap.getTile(playerPos->x, playerPos->y)->isOccupied = false;
			gameMap.getTile(enemyPos->x, enemyPos->y)->isOccupied = false;

			playerPos->x = 1; playerPos->y = 1;
			enemyPos->x = 4;  enemyPos->y = 3;
			playerPos->renderX = playerPos->renderY = -1.0f; // snaps to target next draw
			enemyPos->renderX = enemyPos->renderY = -1.0f;

			gameMap.getTile(1, 1)->isOccupied = true;
			gameMap.getTile(4, 3)->isOccupied = true;

			auto* ph = registry.getHealth(player1);
			auto* eh = registry.getHealth(enemy1);
			ph->currentHp = ph->maxHp;
			eh->currentHp = eh->maxHp;
			eh->isDead = false;

			playerMove->currentPoints = playerMove->maxPoints;
			playerStats->hasAttacked = false;
			isAttackStanceActive = false;
			activeturnindex = 0;
			currentphase = CombatState::ROUND_START;
			activeScene = GameScene::EXPLORATION;
		};

	while (!WindowShouldClose())
	{
		Vector2 mousePos = GetMousePosition();
		Point2D hoveredTile = ScreenToGrid(mousePos);

		switch (activeScene)
		{
		case GameScene::EXPLORATION:
		{
			bool enemyAlive = !registry.getHealth(enemy1)->isDead;

			if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
			{
				if (hoveredTile.x >= 0 && hoveredTile.x < gameMap.getWidth() &&
					hoveredTile.y >= 0 && hoveredTile.y < gameMap.getHeight())
				{
					if (playerPos)
					{
						ExplorationMovementSystem(registry, gameMap, player1, hoveredTile.x, hoveredTile.y);
					}
				}

				if (enemyAlive && playerPos && enemyPos)
				{
					int mandist = std::abs(enemyPos->x - playerPos->x) + std::abs(enemyPos->y - playerPos->y);

					if (mandist <= 2)
					{
						std::cout << "\n[AMBUSH TRIGGERED] Enemy spotted you! Locking grid cells and initiating combat initiative timeline..." << std::endl;

						currentphase = CombatState::ROUND_START;
						activeScene = GameScene::COMBAT;
					}
				}
			}

			if (IsKeyPressed(KEY_SPACE) && enemyAlive)
			{
				std::cout << "\n[SCENE] Entering combat matrix..." << std::endl;
				/*registry.getPosition(player1)->x = 1;
				registry.getPosition(player1)->y = 1;
				registry.getPosition(enemy1)->x = 4;
				registry.getPosition(enemy1)->y = 3;*/

				currentphase = CombatState::ROUND_START;
				activeScene = GameScene::COMBAT;
			}

			if (IsKeyPressed(KEY_Z))
			{
				if (playerPos && npcPos)
				{
					int dist = std::abs(npcPos->x - playerPos->x) + std::abs(npcPos->y - playerPos->y);

					if (dist <= 1)
					{
						std::cout << "\n[STORY] Loading branching dialogue script nodes for Kroni..." << std::endl;

						dialogueTree.clear();

						dialogueTree.push_back(DialogueNode{
						"Kroni: State your business traveler. I don't see many royal guards roaming out here.",
						{
							{ "1. Ask about chicken and coke.", 1, false },
							{ "2. [Threaten] Hand over your coin bag!", 2, false },
							{ "3. Leave peacefully.", -1, false } // Closes dialogue safely
						}
							});

						dialogueTree.push_back(DialogueNode{
						"Kroni: Chicken and coke? Go ask that pirate captain down the road. Though he might try to slice you open instead.",
						{
							{ "1. Thanks for the advice. [Leave]", -1, false },
							{ "2. Actually, I change my mind. Hand over the gold! [Fight]", 2, false }
						}
							});

						dialogueTree.push_back(DialogueNode{
						"Kroni: You think that shiny uniform makes you tough? Pirates! Draw your blades, defend the harbor!",
						{
							{ "1. Draw your steel! [Start Combat Encounter]", -1, true } 
						}
							});


						/*dialogueLines.clear();
						dialogueLines.push_back("Player: Im looking for chicken and coke.");
						dialogueLines.push_back("npc: Why dont you go ask that guy. Im sure he wont try to kill you.");
						dialogueLines.push_back("Player: Thanks npc");*/
						
						currentDialogueNodeIndex = 0;
						activeScene = GameScene::DIALOGUE;
					}
				}
			}
			break;
		}

		case GameScene::COMBAT:
		{
			auto EndTrn = [&]()
				{
					std::cout << "\n[UI COMMAND] End Turn button clicked! Handing initiative to AI..." << std::endl;

					playerMove->currentPoints = playerMove->maxPoints;
					isAttackStanceActive = false;
					playerStats->hasAttacked = false;
					activeturnindex++;
					if (activeturnindex >= combatants.size())
						currentphase = CombatState::ROUND_END;
					else
						currentphase = CombatState::AI_TURN;
				};
			
			if (currentphase == CombatState::PLAYER_TURN)
			{
				if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
				{
					if (CheckCollisionPointRec(mousePos, endTrnBttn))
					{
						EndTrn();
					}
					else if (CheckCollisionPointRec(mousePos, attackBtn))
					{
						isAttackStanceActive = !isAttackStanceActive;
						std::cout << "[UI COMMAND] Attack Stance toggled! State: "
							<< (isAttackStanceActive ? "READY" : "OFF") << std::endl;
					}
					else if (isAttackStanceActive && enemyPos != nullptr && hoveredTile.x == enemyPos->x && hoveredTile.y == enemyPos->y)
					{
						CombatDamageSystem(registry, player1, enemy1);
						isAttackStanceActive = false;
						playerStats->hasAttacked = true;
					}
					else if (hoveredTile.x >= 0 && hoveredTile.x < gameMap.getWidth() &&
						hoveredTile.y >= 0 && hoveredTile.y < gameMap.getHeight())
					{
						std::cout << "\n[COMMAND] Click Detected! Dispatched to destination: ["
							<< hoveredTile.x << "," << hoveredTile.y << "]" << std::endl;

						CombatMovementSystem(registry, gameMap, player1, hoveredTile.x, hoveredTile.y);
					}
				}
				else if (IsKeyPressed(KEY_SPACE))
				{
					EndTrn();
				}
			}

			ProcessCombatSequence(registry, gameMap, currentphase, combatants, activeturnindex);

			auto* enemyHealth = registry.getHealth(enemy1);
			auto* playerHealth = registry.getHealth(player1);
			if (playerHealth && !playerHealth->isDead && playerHealth->currentHp <= 0)
			{
				std::cout << "\n[SCENE] Defeat..." << std::endl;
				activeScene = GameScene::GAME_OVER;
			}
			else if (enemyHealth && !enemyHealth->isDead && enemyHealth->currentHp <= 0)
			{
				std::cout << "\n[SCENE] Victory! Returning to world map..." << std::endl;
				enemyHealth->isDead = true;
				gameMap.getTile(enemyPos->x, enemyPos->y)->isOccupied = false;
				activeScene = GameScene::EXPLORATION;
			}
			break;
		}
		case GameScene::DIALOGUE:
		{
			//DialogueSystem(currentphase, activeScene, dialogueLines, currentLineIndex);
			break;
		}
		case GameScene::GAME_OVER:
		{
			if (IsKeyPressed(KEY_R)) resetGame();
			break;
		}
		}


		BeginDrawing();
		ClearBackground(RAYWHITE);

		for (int y = 0; y < gameMap.getHeight(); ++y)
		{
			for (int x = 0; x < gameMap.getWidth(); ++x)
			{
				Tile* tile = gameMap.getTile(x, y);
				Color renderColor = LIME;

				if (tile->type == TileType::OBSTACLE)
				{
					renderColor = GRAY;
				}

				DrawIsometricTile(x, y, renderColor);
			}
		}

		if (hoveredTile.x >= 0 && hoveredTile.x < gameMap.getWidth() &&
			hoveredTile.y >= 0 && hoveredTile.y < gameMap.getHeight())
		{
			Color highlightColor = isAttackStanceActive ? ColorAlpha(RED, 0.4f) : ColorAlpha(GOLD, 0.4f);
			DrawIsometricTile(hoveredTile.x, hoveredTile.y, highlightColor);
		}

		switch (activeScene)
		{
		case GameScene::EXPLORATION:
		{
			DrawText("EXPLORATION MODE (Out of Battle)", 10, 10, 20, DARKGRAY);
			DrawText("Left-click to roam freely | Press SPACEBAR to trigger a Combat Encounter", 10, 35, 16, PURPLE);
			break;
		}

		case GameScene::COMBAT:
		{
			DrawText("COMBAT ARENA (In Battle)", 10, 10, 20, DARKGRAY);
			if (currentphase == CombatState::PLAYER_TURN)
				DrawText("Player Turn: Left-click a green tile to move and strike!", 10, 35, 16, BLUE);
			else
				DrawText("Enemy Turn: Automated state tracking actively running...", 10, 35, 16, RED);

			if (currentphase == CombatState::PLAYER_TURN)
			{
				if (!isAttackStanceActive)
				{
					std::vector<int> dist = ComputeReachable(gameMap, playerPos->x, playerPos->y, playerMove->currentPoints);

					for (int y = 0; y < gameMap.getHeight(); ++y)
					{
						for (int x = 0; x < gameMap.getWidth(); ++x)
						{
							Tile* tile = gameMap.getTile(x, y);

							if (dist[tile->y * gameMap.getWidth() + tile->x] > 0)
							{
								DrawIsometricTile(x, y, ColorAlpha(GREEN, 0.35f));
							}
						}
					}
				}
				
				
				bool isHovered = CheckCollisionPointRec(mousePos, endTrnBttn);
				Color btnColor = isHovered ? LIGHTGRAY : GRAY;

				DrawRectangleRec(endTrnBttn, btnColor);
				DrawRectangleLines(endTrnBttn.x, endTrnBttn.y, endTrnBttn.width, endTrnBttn.height, DARKGRAY);
				DrawText("END TURN", endTrnBttn.x + 24, endTrnBttn.y + 12, 16, BLACK);

				bool atkHovered = CheckCollisionPointRec(mousePos, attackBtn);
				Color atkBtnColor = isAttackStanceActive ? RED : (atkHovered ? LIGHTGRAY : GRAY);

				DrawRectangleRec(attackBtn, atkBtnColor);
				DrawRectangleLines(attackBtn.x, attackBtn.y, attackBtn.width, attackBtn.height, DARKGRAY);
				DrawText("ATTACK", attackBtn.x + 36, attackBtn.y + 12, 16, isAttackStanceActive ? WHITE : BLACK);
			}
			break;
		}
		case GameScene::DIALOGUE:
		{
			if (currentDialogueNodeIndex >= 0 && currentDialogueNodeIndex < (int)dialogueTree.size())
			{
				DialogueNode& currentNode = dialogueTree[currentDialogueNodeIndex];

				// 1. Draw the clean black background banner window overlay [CP2]
				DrawRectangle(20, 360, windowWidth - 40, 220, ColorAlpha(BLACK, 0.85f));
				DrawRectangleLines(20, 360, windowWidth - 40, 220, GOLD);

				// 2. Draw the NPC Speaker line text string [CP2]
				DrawText(currentNode.speakerText.c_str(), 40, 380, 16, RAYWHITE);

				// 3. Render and capture interactive choice box rows [CP2]
				int startY = 430;
				for (size_t i = 0; i < currentNode.options.size(); ++i)
				{
					// Dynamically map a clickable 500x30px layout bounding box rectangle for each button [CP2]
					Rectangle optionBox = { 40.0f, (float)(startY + (i * 35)), 500.0f, 30.0f };

					// Track mouse collision points [CP2]
					bool isHovered = CheckCollisionPointRec(mousePos, optionBox);

					// Text turns brilliant gold if highlighted, soft gray if idle [CP2]
					DrawText(currentNode.options[i].choiceText.c_str(), optionBox.x, optionBox.y, 14, isHovered ? GOLD : LIGHTGRAY);

					// MOUSE CLICK INTERCEPT SYSTEM: [CP1, CP2]
					if (isHovered && IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
					{
						std::cout << "[UI NARRATIVE] Option choice pressed index: " << i << std::endl;

						// Condition Check: Does this choice path demand immediate combat deployment? [CP1]
						if (currentNode.options[i].triggersCombat && !registry.getHealth(enemy1)->isDead)
						{
							std::cout << "[SCENE TRANSITION] Hostile node option selected! Engaging tactical combat..." << std::endl;
							currentphase = CombatState::ROUND_START;
							activeScene = GameScene::COMBAT; // Turn on turn-based battle limits instantly! [CP1]
						}
						else
						{
							// Update script branch pointer variables dynamically [CP1]
							currentDialogueNodeIndex = currentNode.options[i].nextNodeIndex;

							// Shield check: If the choice pointed to -1, close window safely [CP1]
							if (currentDialogueNodeIndex == -1)
							{
								std::cout << "[SCENE] Dialogue exited peacefully." << std::endl;
								activeScene = GameScene::EXPLORATION; // Unfreeze free roam controls
							}
						}
					}
				}
			}
			
			//if (currentLineIndex < dialogueLines.size())
			//{
			//	
			//	/*DrawRectangle(20, 460, windowWidth - 40, 110, ColorAlpha(BLACK, 0.85f));
			//	DrawRectangleLines(20, 460, windowWidth - 40, 110, GOLD);

			//	DrawText(dialogueLines[currentLineIndex].c_str(), 40, 485, 16, RAYWHITE);
			//	DrawText("[Press SPACEBAR to continue]", windowWidth - 240, 545, 12, GRAY);*/
			//}
			break;
		}
		case GameScene::GAME_OVER:
		{
			DrawRectangle(0, 0, windowWidth, windowHeight, ColorAlpha(BLACK, 0.7f));
			DrawText("YOU HAVE FALLEN", 250, 250, 40, RED);
			DrawText("Press R to restart", 300, 310, 20, RAYWHITE);
			break;
		}
		}

		std::vector<Entity> actors = registry.getActors();
		for (Entity actor : actors)
		{
			PositionComponent* pos = registry.getPosition(actor);
			CombatStatsComponent* stats = registry.getCombatStats(actor);
			HealthComponent* hp = registry.getHealth(actor);

			if (pos && !hp->isDead)
			{
				Vector2 targetPixelPos = GridToScreen(pos->x, pos->y);

				if (pos->renderX < 0.0f || pos->renderY < 0.0f)
				{
					pos->renderX = targetPixelPos.x;
					pos->renderY = targetPixelPos.y;
				}

				pos->renderX += (targetPixelPos.x - pos->renderX) * 0.10f;
				pos->renderY += (targetPixelPos.y - pos->renderY) * 0.10f;

				Vector2 smoothPixelPos = { pos->renderX, pos->renderY };

				Color actorColor = BLUE;
				if (stats->archetype == "Lanun Captain")
				{
					actorColor = RED;
				}
				else if (stats->archetype == "Kroni")
				{
					actorColor = YELLOW;
				}

				if (stats->archetype == "Pendekar")
				{
					float distanceToTarget = std::abs(targetPixelPos.x - pos->renderX) + std::abs(targetPixelPos.y - pos->renderY);

					bool isMoving = (distanceToTarget > 1.0f);
					Texture2D activeSheet = isMoving ? playerRun : playerIdle;

					animFrameTimer += GetFrameTime();
					if (animFrameTimer >= 0.05f)
					{
						animFrameTimer = 0.0f;
						currentAnimFrame = (currentAnimFrame + 1) % 24;
					}

					float frameWidth = (float)(activeSheet.width) / 6.0f;
					float frameHeight = (float)(activeSheet.height) / 4.0f;

					int rowIndex = currentAnimFrame % 4;
					int colIndex = currentAnimFrame / 4;

					Rectangle sourceRect =
					{
						(float)colIndex * frameWidth,
						(float)rowIndex * frameHeight,
						frameWidth,
						frameHeight
					};

					float desiredHeight = 130.0f;
					float desiredWidth = desiredHeight * (frameWidth / frameHeight);

					// 3. Destination: Map it over your smooth visual frame interpolation vectors [CP2]
					Rectangle destRect = {
						smoothPixelPos.x,
						smoothPixelPos.y,
						desiredWidth,
						desiredHeight
					};

					// 4. Center Offset Origin: Automatically centers the 32x32 bounding box over the tile slot center [CP2]
					Vector2 originOffset = { desiredWidth / 2.0f, desiredHeight * 0.85f };

					// 5. GPU render pass executing structural downscale matrices dynamically [CP2]
					DrawTexturePro(activeSheet, sourceRect, destRect, originOffset, 0.0f, WHITE);
				}
				else
					DrawCircleV(smoothPixelPos, 10.0f, actorColor);
				DrawText(stats->name.c_str(), smoothPixelPos.x - 20, smoothPixelPos.y - 25, 12, BLACK);

				//Drawing Health bar
				float healthRatio = (float)hp->currentHp / hp->maxHp;
				int barWidth = 40;
				int barHeight = 6;
				int barX = smoothPixelPos.x - (barWidth / 2);
				int barY = smoothPixelPos.y - 42;

				DrawRectangle(barX, barY, barWidth, barHeight, RED);
				DrawRectangle(barX, barY, barWidth * healthRatio, barHeight, GREEN);
				DrawRectangleLines(barX, barY, barWidth, barHeight, BLACK);

			}
		}

		EndDrawing();
	}
	UnloadTexture(playerIdle);
	UnloadTexture(playerRun);

	CloseWindow();
	return 0;
}
