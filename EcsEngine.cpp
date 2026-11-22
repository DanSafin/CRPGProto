#include "EcsEngine.hpp"

Entity Registry::createEntity() { return nextEntityId++; }

void Registry::assignPosition(Entity entity, int x, int y)
{
	positionComponents[entity] = PositionComponent{ x, y , -1.0f, -1.0f };
}

void Registry::assignHealth(Entity entity, int hp)
{
	healthComponents[entity] = HealthComponent{ hp,hp };
}

void Registry::assignCombatStats(Entity entity, const std::string& name, int semangat, const std::string& arch, bool isCombatant)
{
	combatStatsComponents[entity] = CombatStatsComponent{ name, semangat, arch, false, isCombatant };
}

void Registry::assignMovement(Entity entity, int maxSteps)
{
	movementComponents[entity] = MovementComponent{ maxSteps, maxSteps };
}

PositionComponent* Registry::getPosition(Entity entity)
{
	return positionComponents.contains(entity) ? &positionComponents[entity] : nullptr;
}

HealthComponent* Registry::getHealth(Entity entity)
{
	return healthComponents.contains(entity) ? &healthComponents[entity] : nullptr;
}

CombatStatsComponent* Registry::getCombatStats(Entity entity)
{
	return combatStatsComponents.contains(entity) ? &combatStatsComponents[entity] : nullptr;
}

MovementComponent* Registry::getMovement(Entity entity)
{
	return movementComponents.contains(entity) ? &movementComponents[entity] : nullptr;
}

std::vector<Entity> Registry::getCombatants() const
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

std::vector<Entity> Registry::getActors() const
{
	std::vector<Entity> actors;
	for (const auto& [entity, pos] : positionComponents)
	{
		actors.push_back(entity);
	}
	return actors;
}

void Registry::assignWeapon(Entity entity, const std::string& name, int bonusDmg, int range)
{
	weaponComponents[entity] = WeaponComponent{ name, bonusDmg, range };
}

WeaponComponent* Registry::getWeapon(Entity entity)
{
	return weaponComponents.contains(entity) ? &weaponComponents[entity] : nullptr;
}