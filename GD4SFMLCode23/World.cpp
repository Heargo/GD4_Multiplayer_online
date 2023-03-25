#include "World.hpp"
#include "Projectile.hpp"
#include "Pickup.hpp"
#include "Utility.hpp"
#include "ParticleNode.hpp"
#include "ParticleType.hpp"
#include "SoundNode.hpp"
#include <SFML/Graphics/RenderWindow.hpp>
#include <iostream>
#include <limits>
#include <SFML/Window/Mouse.hpp>
#include "Asteroid.hpp"
#include "GameServer.hpp"


World::World(sf::RenderTarget& output_target, FontHolder& font, SoundPlayer& sounds, State::Context context, bool networked)
	:m_target(output_target)
	,m_camera(output_target.getDefaultView())
	,m_textures()
	,m_fonts(font)
	,m_sounds(sounds)
	,m_scenegraph()
	,m_scene_layers()
	,m_world_bounds(0.f, 0.f, 5000.f, 5000.f)
	,m_spawn_position()
	,m_scrollspeed(-50.f)
	,m_scrollspeed_compensation(1.f)
	,m_player_aircraft()
	,m_enemy_spawn_points()
	,m_active_enemies()
	,m_networked_world(networked)
	,m_network_node(nullptr)
	,m_finish_sprite(nullptr)
	,m_context(context)
	,m_socket(context.socket)
	,m_existingAsteroides()
	,m_existingAsteroidesSize()
{
	//center spawn position
	m_spawn_position.x = m_world_bounds.width / 2.f;
	m_spawn_position.y = m_world_bounds.height / 2.f;
	
	m_scene_texture.create(m_target.getSize().x, m_target.getSize().y);
	LoadTextures();
	BuildScene();

	m_camera.setCenter(m_spawn_position);
}

void World::SetWorldScrollCompensation(float compensation)
{
	m_scrollspeed_compensation = compensation;
}

void World::Update(sf::Time dt)
{
	//stick camera to local player
	for (Aircraft* a : m_player_aircraft)
	{
		a->ApplyFriction();
		//camera follow local player
		if(IsLocalPlayer(a->GetIdentifier()))
			m_camera.setCenter(a->getPosition());
	}

	DestroyEntitiesOutsideView(); // maybe to remove (?)


	//Forward the commands to the scenegraph, sort out velocity
	while (!m_command_queue.IsEmpty())
	{
		m_scenegraph.OnCommand(m_command_queue.Pop(), dt);
	}
	AdaptPlayerVelocity();

	HandleCollisions();

	//Remove all of the destroyed entities
	//RemoveWrecks() only destroys the entities, not the pointers in m_player_aircraft
	auto first_to_remove = std::remove_if(m_player_aircraft.begin(), m_player_aircraft.end(), std::mem_fn(&Aircraft::IsMarkedForRemoval));
	m_player_aircraft.erase(first_to_remove, m_player_aircraft.end());
	m_scenegraph.RemoveWrecks();

	//SpawnEnemies();
	
	//Apply movement
	m_scenegraph.Update(dt, m_command_queue);
	AdaptPlayerPosition();

	UpdateSounds();
}

void World::Draw()
{
	if (PostEffect::IsSupported())
	{
		m_scene_texture.clear();
		m_scene_texture.setView(m_camera);
		m_scene_texture.draw(m_scenegraph);
		m_scene_texture.display();
		m_bloom_effect.Apply(m_scene_texture, m_target);
	}
	else
	{
		m_target.setView(m_camera);
		m_target.draw(m_scenegraph);
	}
}

Aircraft* World::GetAircraft(int identifier) const
{
	for (Aircraft* a : m_player_aircraft)
	{
		if (a->GetIdentifier() == identifier)
		{
			return a;
		}
	}
	return nullptr;
}

void World::RemoveAircraft(int identifier)
{
	Aircraft* aircraft = GetAircraft(identifier);
	if (aircraft)
	{
		aircraft->Destroy();
		m_player_aircraft.erase(std::find(m_player_aircraft.begin(), m_player_aircraft.end(), aircraft));
	}
}

Aircraft* World::AddAircraft(int identifier,bool isLocalPlayer)
{
	std::unique_ptr<Aircraft> player(new Aircraft(AircraftType::kEagle, m_textures, m_fonts, isLocalPlayer));
	player->setPosition(m_camera.getCenter());
	player->SetIdentifier(identifier);

	m_player_aircraft.emplace_back(player.get());
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(player));

	if(isLocalPlayer)
		m_local_player_identifiers.push_back(identifier);
	
	std::cout << "Player " << identifier << " spawned" << std::endl;
	
	return m_player_aircraft.back();
}

void World::CreatePickup(sf::Vector2f position, PickupType type)
{
	std::unique_ptr<Pickup> pickup(new Pickup(type, m_textures));
	pickup->setPosition(position);
	pickup->SetVelocity(0.f, 1.f);
	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(pickup));
}

bool World::PollGameAction(GameActions::Action& out)
{
	return m_network_node->PollGameAction(out);
}

bool World::IsLocalPlayer(int identifier)
{
	//check if in m_local_player_identifiers
	return std::find(m_local_player_identifiers.begin(), m_local_player_identifiers.end(), identifier) != m_local_player_identifiers.end();
}

void World::SetCurrentBattleFieldPosition(float lineY)
{
	m_camera.setCenter(m_camera.getCenter().x, lineY - m_camera.getSize().y / 2);
	m_spawn_position.y = m_world_bounds.height;
}

void World::SetWorldHeight(float height)
{
	m_world_bounds.height = height;
}

bool World::HasAlivePlayer() const
{
	return !m_player_aircraft.empty();
}

bool World::HasPlayerReachedEnd() const
{
	if (Aircraft* aircraft = GetAircraft(1))
	{
		return !m_world_bounds.contains(aircraft->getPosition());
	}
	return false;
}

CommandQueue& World::GetCommandQueue()
{
	return m_command_queue;
}

void World::LoadTextures()
{
	m_textures.Load(Texture::kEntities, "Media/Textures/Entities.png");
	m_textures.Load(Texture::kSpace, "Media/Textures/greenNebula.png");
	m_textures.Load(Texture::kExplosion, "Media/Textures/Explosion.png");
	m_textures.Load(Texture::kParticle, "Media/Textures/Particle.png");
	m_textures.Load(Texture::kAsteroid, "Media/Textures/asteroid.png");

	//print "ok" in console
	std::cout << "ok" << std::endl;

	m_textures.Load(Texture::kLocalPlayer, "Media/Textures/Spaceship_01_BLUE.png");
	m_textures.Load(Texture::kRemotePlayer, "Media/Textures/Spaceship_01_RED.png");
	std::cout << "ok spaceships" << std::endl;
}

void World::BuildScene()
{
	//Initialize the different layers
	for(std::size_t i=0; i < static_cast<int>(Layers::kLayerCount); ++i)
	{ 
		ReceiverCategories category = (i == static_cast<int>(Layers::kLowerAir)) ? ReceiverCategories::kScene : ReceiverCategories::kNone;
		SceneNode::Ptr layer(new SceneNode(category));
		m_scene_layers[i] = layer.get();
		m_scenegraph.AttachChild(std::move(layer));
	}

	//Prepare the background
	sf::Texture& texture = m_textures.Get(Texture::kSpace);
	sf::IntRect textureRect(m_world_bounds);
	//expand textureRect to make the background bigger than the canvas
	int sizeIncrease = 1920;
	textureRect.width += sizeIncrease;
	textureRect.height += sizeIncrease;
	texture.setRepeated(true);

	float view_height = m_camera.getSize().y;
	sf::IntRect texture_rect(m_world_bounds);
	texture_rect.height += static_cast<int>(view_height);

	//Add the background sprite to the world
	std::unique_ptr<SpriteNode> background_sprite(new SpriteNode(texture, textureRect));
	background_sprite->setPosition(m_world_bounds.left - sizeIncrease / 2, m_world_bounds.top - sizeIncrease / 2);
	//background_sprite->sceneNodeName = "Background";
	m_scene_layers[static_cast<int>(Layers::kBackground)]->AttachChild(std::move(background_sprite));


	// Add particle node to the scene
	std::unique_ptr<ParticleNode> smokeNode(new ParticleNode(ParticleType::kSmoke, m_textures));
	m_scene_layers[static_cast<int>(Layers::kLowerAir)]->AttachChild(std::move(smokeNode));

	// Add propellant particle node to the scene
	std::unique_ptr<ParticleNode> propellantNode(new ParticleNode(ParticleType::kPropellant, m_textures));
	m_scene_layers[static_cast<int>(Layers::kLowerAir)]->AttachChild(std::move(propellantNode));


	// Add sound effect node
	std::unique_ptr<SoundNode> soundNode(new SoundNode(m_sounds));
	m_scenegraph.AttachChild(std::move(soundNode));

	if (m_networked_world)
	{
		std::unique_ptr<NetworkNode> network_node(new NetworkNode());
		m_network_node = network_node.get();
		m_scenegraph.AttachChild(std::move(network_node));
	}

	//AddEnemies();

	//create the asteroid field
	SpawnAsteroides(30);
}

void World::AdaptPlayerPosition()
{
	//Keep the player on the sceen 
	sf::FloatRect view_bounds = GetViewBounds();
	sf::FloatRect world_bounds = (m_spawn_position, m_world_bounds);
	const float border_distance = 40.f;
	
	for (Aircraft* aircraft : m_player_aircraft)
	{
		//keep player in the world
		sf::Vector2f position = aircraft->getPosition();
		position.x = std::max(position.x, world_bounds.left + border_distance);
		position.x = std::min(position.x, world_bounds.left + world_bounds.width - border_distance);
		position.y = std::max(position.y, world_bounds.top + border_distance);
		position.y = std::min(position.y, world_bounds.top + world_bounds.height - border_distance);
		aircraft->setPosition(position);
	}
}

void World::AdaptPlayerVelocity()
{
	for (Aircraft* aircraft : m_player_aircraft)
	{
		sf::Vector2f velocity = aircraft->GetVelocity();

		//Apply friction to the player
		/*if (velocity.x != 0.f && velocity.y != 0.f)
		{
			aircraft->SetVelocity(velocity * 0.7071f);
		}*/
	}
}

sf::FloatRect World::GetViewBounds() const
{
	return sf::FloatRect(m_camera.getCenter() - m_camera.getSize() / 2.f, m_camera.getSize());
}

sf::FloatRect World::GetBattlefieldBounds() const
{
	//Return camera bounds + a small area at the top where enemies spawn offscreen
	sf::FloatRect bounds = GetViewBounds();
	bounds.top -= 100.f;
	bounds.height += 100.f;

	return bounds;
}

void World::SpawnEnemies()
{
	////Spawn an enemy when it is relevant i.e when about to come on screen - in BattleFieldBounds
	//while (!m_enemy_spawn_points.empty() && m_enemy_spawn_points.back().m_y > GetBattlefieldBounds().top)
	//{
	//	SpawnPoint spawn = m_enemy_spawn_points.back();
	//	
	//	std::unique_ptr<Aircraft> enemy(new Aircraft(spawn.m_type, m_textures, m_fonts));
	//	enemy->setPosition(spawn.m_x, spawn.m_y);
	//	enemy->setRotation(180.f);
	//	//If the game is networked the server is responsible for spawning pickups
	//	
	//	if (m_networked_world)
	//	{
	//		enemy->DisablePickups();
	//	}
	//	m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(enemy));
	//	m_enemy_spawn_points.pop_back();
	//}
}

void World::AddEnemy(AircraftType type, float relX, float relY)
{
	SpawnPoint spawn(type, m_spawn_position.x + relX, m_spawn_position.y - relY);
	m_enemy_spawn_points.emplace_back(spawn);
}

void World::AddEnemies()
{
	if (m_networked_world)
	{
		return;
	}
	//Add all emenies
	AddEnemy(AircraftType::kRaptor, 0.f, 500.f);
	AddEnemy(AircraftType::kRaptor, 0.f, 1000.f);
	AddEnemy(AircraftType::kRaptor, +100.f, 1150.f);
	AddEnemy(AircraftType::kRaptor, -100.f, 1150.f);
	AddEnemy(AircraftType::kAvenger, 70.f, 1500.f);
	AddEnemy(AircraftType::kAvenger, -70.f, 1500.f);
	AddEnemy(AircraftType::kAvenger, -70.f, 1710.f);
	AddEnemy(AircraftType::kAvenger, 70.f, 1700.f);
	AddEnemy(AircraftType::kAvenger, 30.f, 1850.f);
	AddEnemy(AircraftType::kRaptor, 300.f, 2200.f);
	AddEnemy(AircraftType::kRaptor, -300.f, 2200.f);
	AddEnemy(AircraftType::kRaptor, 0.f, 2200.f);
	AddEnemy(AircraftType::kRaptor, 0.f, 2500.f);
	AddEnemy(AircraftType::kAvenger, -300.f, 2700.f);
	AddEnemy(AircraftType::kAvenger, -300.f, 2700.f);
	AddEnemy(AircraftType::kRaptor, 0.f, 3000.f);
	AddEnemy(AircraftType::kRaptor, 250.f, 3250.f);
	AddEnemy(AircraftType::kRaptor, -250.f, 3250.f);
	AddEnemy(AircraftType::kAvenger, 0.f, 3500.f);
	AddEnemy(AircraftType::kAvenger, 0.f, 3700.f);
	AddEnemy(AircraftType::kRaptor, 0.f, 3800.f);
	AddEnemy(AircraftType::kAvenger, 0.f, 4000.f);
	AddEnemy(AircraftType::kAvenger, -200.f, 4200.f);
	AddEnemy(AircraftType::kRaptor, 200.f, 4200.f);
	AddEnemy(AircraftType::kRaptor, 0.f, 4400.f);

	//Sort according to y value so that lower enemies are checked first
	SortEnemies();
}

void World::SortEnemies()
{
	//Sort all enemies according to their y-value, such that lower enemies are checked first for spawning
	std::sort(m_enemy_spawn_points.begin(), m_enemy_spawn_points.end(), [](SpawnPoint lhs, SpawnPoint rhs)
	{
		return lhs.m_y < rhs.m_y;
	});
}

void World::DestroyEntitiesOutsideView()
{
	Command command;
	command.category = static_cast<int>(ReceiverCategories::kEnemyAircraft) | static_cast<int>(ReceiverCategories::kProjectile);
	command.action = DerivedAction<Entity>([this](Entity& e, sf::Time dt)
	{
		//Does the object interest with battlefield?
		if (!GetBattlefieldBounds().intersects(e.GetBoundingRect()))
		{
			e.Remove();
		}
	});
	m_command_queue.Push(command);
}

void World::GuideMissiles()
{
	//Target the closest enemy in the world
	Command enemyCollector;
	enemyCollector.category = static_cast<int>(ReceiverCategories::kEnemyAircraft);
	enemyCollector.action = DerivedAction<Aircraft>([this](Aircraft& enemy, sf::Time)
	{
		if (!enemy.IsDestroyed())
		{
			m_active_enemies.emplace_back(&enemy);
		}
	});

	Command missileGuider;
	missileGuider.category = static_cast<int>(ReceiverCategories::kAlliedProjectile);
	missileGuider.action = DerivedAction<Projectile>([this](Projectile& missile, sf::Time dt)
	{
		if (!missile.IsGuided())
		{
			return;
		}

		float min_distance = std::numeric_limits<float>::max();
		Aircraft* closest_enemy = nullptr;

		for (Aircraft* enemy : m_active_enemies)
		{
			float enemy_distance = Distance(missile, *enemy);

			if (enemy_distance < min_distance)
			{
				closest_enemy = enemy;
				min_distance = enemy_distance;
			}
		}

		if (closest_enemy)
		{
			missile.GuideTowards(closest_enemy->GetWorldPosition());
		}
	});

	m_command_queue.Push(enemyCollector);
	m_command_queue.Push(missileGuider);
	m_active_enemies.clear();
}

bool MatchesCategories(SceneNode::Pair& colliders, ReceiverCategories type1, ReceiverCategories type2)
{
	unsigned int category1 = colliders.first->GetCategory();
	unsigned int category2 = colliders.second->GetCategory();
	if (static_cast<int>(type1) & category1 && static_cast<int>(type2) & category2)
	{
		return true;
	}
	else if (static_cast<int>(type1) & category2 && static_cast<int>(type2) & category1)
	{
		std::swap(colliders.first, colliders.second);
		return true;
	}
	else
	{
		return false;
	}
}

void World::HandleCollisions()
{
	std::set<SceneNode::Pair> collision_pairs;
	m_scenegraph.CheckSceneCollision(m_scenegraph, collision_pairs);
	for (SceneNode::Pair pair : collision_pairs)
	{
		if (MatchesCategories(pair, ReceiverCategories::kPlayerAircraft, ReceiverCategories::kEnemyAircraft))
		{
			auto& player = static_cast<Aircraft&>(*pair.first);
			auto& enemy = static_cast<Aircraft&>(*pair.second);
			//Collision Response
			player.Damage(enemy.GetHitPoints());
			enemy.Destroy();
		}
		else if (MatchesCategories(pair, ReceiverCategories::kPlayerAircraft, ReceiverCategories::kPickup))
		{
			auto& player = static_cast<Aircraft&>(*pair.first);
			auto& pickup = static_cast<Pickup&>(*pair.second);
			//Collision Response
			pickup.Apply(player);
			pickup.Destroy();
			player.PlayLocalSound(m_command_queue, SoundEffect::kCollectPickup);
		}
		else if (MatchesCategories(pair, ReceiverCategories::kPlayerAircraft, ReceiverCategories::kAlliedProjectile)
			&& static_cast<Projectile&>(*pair.second).GetOwnerIdentifier() != static_cast<Aircraft&>(*pair.first).GetIdentifier())
		{
			auto& aircraft = static_cast<Aircraft&>(*pair.first);
			auto& projectile = static_cast<Projectile&>(*pair.second);
			//Collision Response
			aircraft.Damage(projectile.GetDamage());
			m_context.lastHit = projectile.GetOwnerIdentifier();

			//update score to server send packet
			sf::Packet packet;
			packet << static_cast<sf::Int32>(Client::PacketType::kLeaderbordUpdate);
			packet << projectile.GetOwnerIdentifier(); //the id of the killer
			packet << aircraft.GetIdentifier(); //the id of the killed
			
			//send it
			m_socket->send(packet);
			
			projectile.Destroy();
		}
		//collision bullet / asteroid
		else if (MatchesCategories(pair, ReceiverCategories::kAlliedProjectile, ReceiverCategories::kAsteroid))
		{
			auto& projectile = static_cast<Projectile&>(*pair.first);
			auto& asteroid = static_cast<Asteroid&>(*pair.second);
			//Collision Response
			float distance = Distance(projectile, asteroid);
			if (distance <= asteroid.GetRadius())
			{
				projectile.Destroy();
			}
		}
		
		//collision with asteroid
		else if (MatchesCategories(pair, ReceiverCategories::kPlayerAircraft, ReceiverCategories::kAsteroid))
		{
			std::cout << "collision with asteroid" << std::endl;
			auto& aircraft = static_cast<Aircraft&>(*pair.first);
			auto& asteroid = static_cast<Asteroid&>(*pair.second);
			float distance = Distance(aircraft, asteroid);
			if (!aircraft.IsDestroyed() && distance <= asteroid.GetRadius())
			{

				//calculate and apply damage to inflict to player depending on player velocity
				float velocityFactor = 1.5 * (abs(aircraft.GetVelocity().x) + abs(aircraft.GetVelocity().y));
				float sizeFactor = 0.1 * asteroid.GetRadius();
				float damageToApply = 0.005 * velocityFactor * sizeFactor;
				int damageToAsteroid = damageToApply / 4;
				//asteroid.Damage(damageToAsteroid);
				//std::cout << "dmg for player :" << damageToApply << std::endl;
				//std::cout << "dmg for asteroid :" << damageToApply / 4 << std::endl;

				aircraft.Damage(damageToApply);

				//move the aircraft back 5 px in the opposite direction of normalized velocity
				sf::Vector2f moveBack = aircraft.GetVelocity();
				moveBack.x = moveBack.x / sqrt(pow(moveBack.x, 2) + pow(moveBack.y, 2)) * 5;
				moveBack.y = moveBack.y / sqrt(pow(moveBack.x, 2) + pow(moveBack.y, 2)) * 5;

				aircraft.setPosition(aircraft.getPosition() - moveBack);

				//apply bounced velocity
				aircraft.SetVelocity(aircraft.GetVelocity().x * -1, aircraft.GetVelocity().y * -1);
			}
		}
	}
}

void World::UpdateSounds()
{
	sf::Vector2f listener_position;

	// 0 players (multiplayer mode, until server is connected) -> view center
	if (m_player_aircraft.empty())
	{
		listener_position = m_camera.getCenter();
	}

	// 1 or more players -> mean position between all aircrafts
	else
	{
		for (Aircraft* aircraft : m_player_aircraft)
		{
			listener_position += aircraft->GetWorldPosition();
		}

		listener_position /= static_cast<float>(m_player_aircraft.size());
	}

	// Set listener's position
	m_sounds.SetListenerPosition(listener_position);

	// Remove unused sounds
	m_sounds.RemoveStoppedSounds();
}

void World::SpawnAsteroides(int nbAsteroides)
{
	//hardcoded seed for now 
	srand(1);

	//list of existing asteroides position and size
	std::vector<sf::Vector2f> m_existingAsteroides;
	std::vector<int> m_existingAsteroidesSize;

	//Spawn the asteroids
	for (int i = 0; i < nbAsteroides; i++)
	{
		//get a random size between 50px and 200px with a step of 50px
		int size = 50 * (rand() % 4 + 1);

		sf::Vector2f pos = GetRandomPosition(size, m_existingAsteroides, m_existingAsteroidesSize);

		//skip if there is no valid position for this size. Get random position return a (0,0) if there where to many attemps 
		if (pos == sf::Vector2f(0.f, 0.f)) continue;

		//add pos and size to history
		m_existingAsteroides.push_back(pos);
		m_existingAsteroidesSize.push_back(size);

		//create it and add it to the scene
		std::unique_ptr<Asteroid> asteroid(new Asteroid(size, m_textures));
		asteroid->setPosition(pos);
		//asteroid->sceneNodeName = "Asteroid " + std::to_string(i);
		m_scene_layers[static_cast<int>(Layers::kUpperAir)]->AttachChild(std::move(asteroid));

		std::cout << "Asteroid " << i << " created at " << pos.x << " " << pos.y << " with size " << size << std::endl;
	}
}

sf::Vector2f World::GetRandomPosition(int size, std::vector<sf::Vector2f> existingAsteroides, std::vector<int> existingAsteroidesSize)
{

	bool tooClose = false;
	//margin in px
	int margin = 30;

	//position is within word size
	int marginWorld = 50;
	int xRange = (m_world_bounds.width / 2.f) - marginWorld;
	int yRange = (m_world_bounds.height / 2.f) - marginWorld;
	//std::cout << "xRange " << xRange << " yRange " << yRange << std::endl;
	float x = (rand() % (2 * xRange) - xRange) + m_spawn_position.x;
	float y = (rand() % (2 * yRange) - yRange) + m_spawn_position.x;

	int attemps = 0;

	//while the position is too close from an existing asteriod, we try other position within 10 attemps limit.
	do {
		for (int j = 0; j < existingAsteroides.size(); j++)
		{
			float distance = std::sqrt((existingAsteroides[j].x - x) * (existingAsteroides[j].x - x) + (existingAsteroides[j].y - y) * (existingAsteroides[j].y - y));
			if (distance <= existingAsteroidesSize[j] + size + margin)
			{
				tooClose = true;
				x = (rand() % (2 * xRange) - xRange) + m_spawn_position.x;
				y = (rand() % (2 * yRange) - yRange) + m_spawn_position.x;
				break;
			}
			else
			{
				tooClose = false;
			}
		}
		attemps++;
	} while (tooClose && attemps < 10);

	//return (0,0) position if there is no room for the asteroid of the size given.
	if (attemps == 10)
	{
		x = 0.f;
		y = 0.f;
	}

	return sf::Vector2f(x, y);



}

sf::Vector2f World::validRespawnPosition()
{
	return GetRandomPosition(10, m_existingAsteroides, m_existingAsteroidesSize);
}

void World::SetSocket(sf::TcpSocket* socket) {
	m_socket = socket;
}
