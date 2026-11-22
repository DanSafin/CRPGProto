#pragma once

#include "Components.hpp"
#include <vector>
#include <unordered_map>
#include <memory>
#include <cstdint>


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

	Entity createEntity();

	void assignPosition(Entity entity, int x, int y);
	void assignHealth(Entity entity, int hp);
	void assignCombatStats(Entity entity, const std::string& name, int semangat, const std::string& arch, bool isCombatant = false);
	void assignMovement(Entity entity, int maxSteps);

	PositionComponent* getPosition(Entity entity);
	HealthComponent* getHealth(Entity entity);
	CombatStatsComponent* getCombatStats(Entity entity);
	MovementComponent* getMovement(Entity entity);
	std::vector<Entity> getCombatants() const;
	std::vector<Entity> getActors() const;
	void assignWeapon(Entity entity, const std::string& name, int bonusDmg, int range);
	WeaponComponent* getWeapon(Entity entity);
};