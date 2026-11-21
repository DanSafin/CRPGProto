#pragma once

#include "Components.hpp"
#include <vector>
#include <unordered_map>
#include <memory>


using Entity = uint32_t;

class Registry
{
private:
	Entity nextEntityId = 0;

	std::unordered_map<Entity, PositionComponent> positionComponents;
	std::unordered_map<Entity, HealthComponent> healthComponents;
	std::unordered_map<Entity, CombatStatsComponent> combatStatsComponents;
	std::unordered_map<Entity, WeaponComponent> weaponComponents;
	std::unordered_map<Entity, MovementComponent> movementComponents;

public:
	Registry() = default;

	Entity createEntity() { return nextEntityId++; }

	void assignPosition(Entity entity, int x, int y)
	{
		positionComponents[entity] = PositionComponent{ x, y , -1.0f, -1.0f};
	}

	void assignHealth(Entity entity, int hp)
	{
		healthComponents[entity] = HealthComponent{ hp,hp };
	}

	void assignCombatStats(Entity entity, const std::string& name, int semangat, const std::string& arch, bool isCombatant = false)
	{
		combatStatsComponents[entity] = CombatStatsComponent{ name, semangat, arch, false, isCombatant };
	}

	void assignMovement(Entity entity, int maxSteps)
	{
		movementComponents[entity] = MovementComponent{ maxSteps, maxSteps };
	}

	PositionComponent* getPosition(Entity entity)
	{
		return positionComponents.contains(entity) ? &positionComponents[entity] : nullptr;
	}

	HealthComponent* getHealth(Entity entity)
	{
		return healthComponents.contains(entity) ? &healthComponents[entity] : nullptr;
	}

	CombatStatsComponent* getCombatStats(Entity entity)
	{
		return combatStatsComponents.contains(entity) ? &combatStatsComponents[entity] : nullptr;
	}

	MovementComponent* getMovement(Entity entity)
	{
		return movementComponents.contains(entity) ? &movementComponents[entity] : nullptr;
	}

	std::vector<Entity> getCombatants() const
	{
		std::vector<Entity> combatants;
		for (const auto& [entity, stats] : combatStatsComponents)
		{
			if (!stats.isCombatant) continue;
			auto hp = healthComponents.find(entity);
			if (hp != healthComponents.end() && hp->second.isDead) continue;
			combatants.push_back(entity);
		}
		return combatants;
	}

	std::vector<Entity> getActors() const
	{
		std::vector<Entity> actors;
		for (const auto& [entity, pos] : positionComponents)
		{
			actors.push_back(entity);
		}
		return actors;
	}

	void assignWeapon(Entity entity, const std::string& name, int bonusDmg, int range)
	{
		weaponComponents[entity] = WeaponComponent{ name, bonusDmg, range };
	}

	WeaponComponent* getWeapon(Entity entity)
	{
		return weaponComponents.contains(entity) ? &weaponComponents[entity] : nullptr;
	}
};