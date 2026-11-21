#pragma once
#include <string>
#include <vector>

enum class GameScene
{
	EXPLORATION,
	COMBAT,
	DIALOGUE,
	GAME_OVER
};

struct Point2D
{
	int x = 0;
	int y = 0;
};

struct PositionComponent
{
	int x = 0;
	int y = 0;

	float renderX = -1.0f;
	float renderY = -1.0f;
};

struct HealthComponent
{
	int currentHp = 100;
	int maxHp = 100;
	bool isDead = false;
};

struct CombatStatsComponent
{
	std::string name;
	int semangat = 10;
	std::string archetype;
	bool hasAttacked = false;
	bool isCombatant = false;
};

enum class CombatState
{
	ROUND_START,
	PLAYER_TURN,
	AI_TURN,
	ROUND_END
};

struct WeaponComponent
{
	std::string name = "Unarmed";
	int bonusDamage = 0;
	int attackRange = 1;
};

struct MovementComponent
{
	int currentPoints = 4;
	int maxPoints = 4;
};

struct DialogueOption
{
	std::string choiceText;
	int nextNodeIndex = -1;
	bool triggersCombat = false;
};

struct DialogueNode
{
	std::string speakerText;
	std::vector<DialogueOption> options;
};

