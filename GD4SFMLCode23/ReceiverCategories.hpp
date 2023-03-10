// HUGO REY D00262075 : remove no necessary enum for the game
#pragma once
enum class ReceiverCategories
{
	kNone = 0,
	kScene = 1 << 0,
	kPlayerAircraft = 1 << 1,
	kEnemyAircraft = 1 << 2,
	kProjectile = 1 << 3,
	kAsteroid = 1 << 4
};

//A message that would be sent to all aircraft
//unsigned int all_aircraft = ReceiverCategories::kPlayerAircraft | ReceiverCategories::kAlliedAircraft | ReceiverCategories::kEnemyAircraft;