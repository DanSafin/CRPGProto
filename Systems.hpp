#pragma once

#include "EcsEngine.hpp"
#include "GridMap.hpp"
#include "Components.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <raylib.h>
#include <queue>

const int TILE_WIDTH = 64;
const int TILE_HEIGHT = 32;
const int MAP_OFFSET_X = 400;
const int MAP_OFFSET_Y = 150;

Vector2 GridToScreen(int x, int y);
Point2D ScreenToGrid(Vector2 mousePos);
void DrawIsometricTile(int x, int y, Color tileColor);
std::vector<int> ComputeReachable(GridMap& map, int startX, int startY, int maxSteps);
void CombatMovementSystem(Registry& registry, GridMap& gameMap, Entity entity, int targetX, int targetY);
void ExplorationMovementSystem(Registry& registry, GridMap& gameMap, Entity& entity, int targetX, int targetY);
std::vector<Entity> runTurnBasedSystem(Registry& registry);
void CombatDamageSystem(Registry& registry, Entity attacker, Entity target);
void ProcessCombatSequence(Registry& registry, GridMap& gameMap, CombatState& currentState, std::vector<Entity>& currentTimeline, size_t& activeTurnIndex);