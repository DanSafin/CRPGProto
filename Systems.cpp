#include "Systems.hpp"

Vector2 GridToScreen(int x, int y)
{
	float screenX = (x - y) * (TILE_WIDTH / 2.0f) + MAP_OFFSET_X;
	float screenY = (x + y) * (TILE_HEIGHT / 2.0f) + MAP_OFFSET_Y;
	return Vector2{ screenX, screenY };
}

Point2D ScreenToGrid(Vector2 mousePos)
{
	float mx = mousePos.x - MAP_OFFSET_X;
	float my = mousePos.y - MAP_OFFSET_Y;

	int gridX = std::round((mx / (TILE_WIDTH / 2.0f) + my / (TILE_HEIGHT / 2.0f)) / 2.0f);
	int gridY = std::round((my / (TILE_HEIGHT / 2.0f) - mx / (TILE_WIDTH / 2.0f)) / 2.0f);

	return Point2D{ gridX, gridY };
}

void DrawIsometricTile(int x, int y, Color tileColor)
{
	Vector2 center = GridToScreen(x, y);

	Vector2 top = { center.x, center.y - (TILE_HEIGHT / 2.0f) };
	Vector2 right = { center.x + (TILE_WIDTH / 2.0f), center.y };
	Vector2 bottom = { center.x, center.y + (TILE_HEIGHT / 2.0f) };
	Vector2 left = { center.x - (TILE_WIDTH / 2.0f), center.y };

	// Raylib native triangles split down the middle [CP2]
	DrawTriangle(top, left, right, tileColor);
	DrawTriangle(left, bottom, right, tileColor);

	DrawLineV(top, right, DARKGRAY);
	DrawLineV(right, bottom, DARKGRAY);
	DrawLineV(bottom, left, DARKGRAY);
	DrawLineV(left, top, DARKGRAY);
}

std::vector<int> ComputeReachable(GridMap& map, int startX, int startY, int maxSteps)
{
	std::vector<int> dist(map.getWidth() * map.getHeight(), -1);
	std::queue<Point2D> toVisit;
	Point2D start = { startX, startY };
	dist[startY * map.getWidth() + startX] = 0;
	toVisit.push(start);

	while (!toVisit.empty())
	{
		Point2D cur = toVisit.front();
		//std::cout << "Visiting [" << cur.x << "," << cur.y << "]" << std::endl;
		toVisit.pop();
		if (dist[cur.y * map.getWidth() + cur.x] >= maxSteps) continue;

		const int dx[4] = { 1, -1, 0, 0 };
		const int dy[4] = { 0, 0, 1, -1 };

		for (int i = 0; i < 4; i++)
		{
			int nx = cur.x + dx[i];
			int ny = cur.y + dy[i];

			if (nx < 0 || nx >= map.getWidth() || ny < 0 || ny >= map.getHeight()) continue;
			if (dist[ny * map.getWidth() + nx] != -1) continue;
			if (map.getTile(nx, ny)->type == TileType::OBSTACLE || map.getTile(nx, ny)->isOccupied) continue;

			dist[ny * map.getWidth() + nx] = dist[cur.y * map.getWidth() + cur.x] + 1;
			toVisit.push({ nx, ny });
			//std::cout << "  neighbor [" << nx << "," << ny << "]" << std::endl;
		}

	}

	/*for (int y = 0; y < map.getHeight(); y++)
	{
		for (int x = 0; x < map.getWidth(); x++)
			std::cout << dist[y * map.getWidth() + x] << "\t";
		std::cout << "\n";
	}*/

	return dist;
}

void CombatMovementSystem(Registry& registry, GridMap& gameMap, Entity entity, int targetX, int targetY)
{
	PositionComponent* pos = registry.getPosition(entity);
	CombatStatsComponent* stats = registry.getCombatStats(entity);
	MovementComponent* move = registry.getMovement(entity);

	if (pos == nullptr || stats == nullptr || move == nullptr)
	{
		std::cout << "target is not a movable entity" << std::endl;
		return;
	}
	if (targetX == pos->x && targetY == pos->y) return;
	if (targetX < 0 || targetX >= gameMap.getWidth() || targetY < 0 || targetY >= gameMap.getHeight()) return;

	std::vector<int> dist = ComputeReachable(gameMap, pos->x, pos->y, move->currentPoints);
	int cost = dist[targetY * gameMap.getWidth() + targetX];
	if (cost == -1)
	{
		std::cout << "Tile is not reachable" << std::endl;
		return;
	}

	move->currentPoints -= cost;
	gameMap.getTile(pos->x, pos->y)->isOccupied = false;
	pos->x = targetX;
	pos->y = targetY;
	gameMap.getTile(pos->x, pos->y)->isOccupied = true;
	std::cout << "cost of this move is " << cost << std::endl;

}

void ExplorationMovementSystem(Registry& registry, GridMap& gameMap, Entity& entity, int targetX, int targetY)
{
	PositionComponent* pos = registry.getPosition(entity);
	CombatStatsComponent* stats = registry.getCombatStats(entity);

	if (pos == nullptr || stats == nullptr) return;

	Tile* destinationTile = gameMap.getTile(targetX, targetY);
	if (!destinationTile || destinationTile->type == TileType::OBSTACLE || destinationTile->isOccupied)
	{
		std::cout << "[EXPLORATION] Target coordinate is blocked!" << std::endl;
		return;
	}

	if (pos->x == targetX && pos->y == targetY) return;

	Tile* currentTile = gameMap.getTile(pos->x, pos->y);
	if (currentTile) currentTile->isOccupied = false;

	int nextX = pos->x;
	int nextY = pos->y;

	if (nextX != targetX)
		nextX += (targetX > nextX) ? 1 : -1;
	if (nextY != targetY)
		nextY += (targetY > nextY) ? 1 : -1;

	Tile* nextTile = gameMap.getTile(nextX, nextY);
	if (nextTile && nextTile->type != TileType::OBSTACLE && !nextTile->isOccupied)
	{
		pos->x = nextX;
		pos->y = nextY;
	}

	Tile* finalTile = gameMap.getTile(pos->x, pos->y);
	if (finalTile) finalTile->isOccupied = true;
}

std::vector<Entity> runTurnBasedSystem(Registry& registry)
{
	std::vector<Entity> combatants = registry.getCombatants();

	std::sort(combatants.begin(), combatants.end(), [&](Entity a, Entity b)
		{
			return registry.getCombatStats(a)->semangat > registry.getCombatStats(b)->semangat;
		});

	std::cout << "\nCalculated Initiative Timeline Order combat" << std::endl;
	for (size_t i = 0; i < combatants.size(); i++)
	{
		auto* stats = registry.getCombatStats(combatants[i]);
		auto* pos = registry.getPosition(combatants[i]);
		std::cout << i + 1 << ". [" << stats->archetype << "] " << stats->name
			<< " (Semangat: " << stats->semangat << ") at Grid [" << pos->x << "," << pos->y << "]" << std::endl;
	}
	return combatants;
}

void CombatDamageSystem(Registry& registry, Entity attacker, Entity target)
{
	auto* attackerStats = registry.getCombatStats(attacker);
	auto* attackerPos = registry.getPosition(attacker);
	auto* weapon = registry.getWeapon(attacker);

	auto* targetStats = registry.getCombatStats(target);
	auto* targetPos = registry.getPosition(target);
	auto* targetHealth = registry.getHealth(target);

	if (!attackerStats || !attackerPos || !targetStats || !targetPos || !targetHealth)
	{
		std::cout << "Something wrong with the entities involved" << std::endl;
		return;
	}

	if (attackerStats->hasAttacked)
	{
		std::cout << "Player already attacked for this round" << std::endl;
		return;
	}
	else
		std::cout << "Enemy detected! Attack!" << std::endl;

	int distance = std::abs(targetPos->x - attackerPos->x) + std::abs(targetPos->y - attackerPos->y);
	int activeAttackRange = (weapon) ? weapon->attackRange : 1;
	std::string currentWeaponName = (weapon) ? weapon->name : "Fists";

	std::cout << "\n--- [SYSTEM: COMBAT] " << attackerStats->name << " engages " << targetStats->name << " ---" << std::endl;
	std::cout << "Distance: " << distance << " tiles away | Weapon Equipped: " << currentWeaponName << " (Max Range: " << activeAttackRange << ")" << std::endl;

	if (distance > activeAttackRange)
	{
		std::cout << "Target is out of range!" << std::endl;
		return;
	}

	int weaponBonus = (weapon) ? weapon->bonusDamage : 0;
	int finalCalculateDamage = attackerStats->semangat + weaponBonus;

	targetHealth->currentHp -= finalCalculateDamage;
	if (targetHealth->currentHp < 0)
		targetHealth->currentHp = 0;

	std::cout << "HIT! " << attackerStats->name << " executes a strike sequence, dealing " << finalCalculateDamage << " damage!" << std::endl;
	std::cout << " -> " << targetStats->name << " Health: " << targetHealth->currentHp << " / " << targetHealth->maxHp << " HP" << std::endl;

	if (targetHealth->currentHp == 0)
	{
		std::cout << "DEFEATED! " << targetStats->name << " has collapsed onto the tactical grid." << std::endl;
		targetHealth->isDead = true;
	}
}

void ProcessCombatSequence(Registry& registry, GridMap& gameMap, CombatState& currentState, std::vector<Entity>& currentTimeline, size_t& activeTurnIndex)
{
	switch (currentState)
	{
	case CombatState::ROUND_START:
	{
		//TO DO: reset movement points and hasAttcked bool

		std::cout << "\n=== [STATE: ROUND_START] Gathering all combatants from the grid... ===" << std::endl;

		currentTimeline = runTurnBasedSystem(registry);
		if (currentTimeline.empty()) return;

		activeTurnIndex = 0;

		Entity firstUp = currentTimeline[activeTurnIndex];
		if (registry.getCombatStats(firstUp)->archetype == "Pendekar" || registry.getCombatStats(firstUp)->archetype == "Laksamana")
			currentState = CombatState::PLAYER_TURN;
		else
			currentState = CombatState::AI_TURN;
		break;
	}

	case CombatState::PLAYER_TURN:
	{
		//Entity activeEntity = currentTimeline[activeTurnIndex];
		//auto* stats = registry.getCombatStats(activeEntity);

		//Entity enemy = 0;
		//auto it = std::find_if(currentTimeline.begin(), currentTimeline.end(), [&](Entity e)
		//	{
		//		auto* cStats = registry.getCombatStats(e);
		//		return cStats && cStats->archetype == "Lanun Captain";
		//	});

		//if (it != currentTimeline.end())
		//	enemy = *it;

		//auto* enemyPos = registry.getPosition(enemy);

		//int targetWalkX = (enemyPos) ? enemyPos->x - 1 : 4;
		//int targetWalkY = (enemyPos) ? enemyPos->y : 3;

		//std::cout << "\n>>> [STATE: PLAYER_TURN] Active Hero: " << stats->name << " <<<" << std::endl;

		////MovementSystem(registry, gameMap, activeEntity, 4, 3);
		//if (enemy != 0)
		//{
		//	CombatDamageSystem(registry, activeEntity, enemy);
		//}

		////activeTurnIndex++;
		//if (activeTurnIndex >= currentTimeline.size())
		//	currentState = CombatState::ROUND_END;
		//else
		//{
		//	Entity nextUp = currentTimeline[activeTurnIndex];
		//	if (registry.getCombatStats(nextUp)->archetype == "Lanun Captain")
		//		currentState = CombatState::AI_TURN;
		//	else
		//		currentState = CombatState::PLAYER_TURN;
		//}
		break;
	}

	case CombatState::AI_TURN:
	{
		Entity activeEntity = currentTimeline[activeTurnIndex];
		auto* stats = registry.getCombatStats(activeEntity);
		auto* enemyPos = registry.getPosition(activeEntity);
		auto* enemyMove = registry.getMovement(activeEntity);

		Entity player = 0;
		auto it = std::find_if(currentTimeline.begin(), currentTimeline.end(), [&](Entity e)
			{
				auto* cStats = registry.getCombatStats(e);
				return cStats && cStats->archetype == "Pendekar";
			});

		if (it != currentTimeline.end())
			player = *it;
		auto* playerPos = registry.getPosition(player);

		std::cout << "\n>>> [STATE: AI_TURN] Active Enemy: " << stats->name << " <<<" << std::endl;

		if (enemyPos && playerPos)
		{
			int nextX = enemyPos->x;
			int nextY = enemyPos->y;

			if (nextX != playerPos->x)
				nextX += (playerPos->x > nextX) ? 1 : -1;
			else if (nextY != playerPos->y)
				nextY += (playerPos->y > nextY) ? 1 : -1;

			CombatMovementSystem(registry, gameMap, activeEntity, nextX, nextY);


			auto* freshEnemyPos = registry.getPosition(activeEntity);
			auto* freshPlayerPos = registry.getPosition(player);

			if (freshEnemyPos && freshPlayerPos)
			{
				CombatDamageSystem(registry, activeEntity, player);
			}
		}

		enemyMove->currentPoints = enemyMove->maxPoints;
		activeTurnIndex++;
		if (activeTurnIndex >= currentTimeline.size())
			currentState = CombatState::ROUND_END;
		else
		{
			Entity nextUp = currentTimeline[activeTurnIndex];
			auto* nextStats = registry.getCombatStats(nextUp);
			if (nextStats->archetype == "Pendekar" || nextStats->archetype == "Laksamana")
				currentState = CombatState::PLAYER_TURN;
			else
				currentState = CombatState::AI_TURN;
		}
		break;
	}

	case CombatState::ROUND_END:
	{
		std::cout << "\n=== [STATE: ROUND_END] All turns completed. Cycling round phase... ===" << std::endl;
		currentState = CombatState::ROUND_START;
		break;
	}
	}
}