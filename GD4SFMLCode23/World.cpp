//HUGO REY D00262075 :
//apply friction to the player every update call and load the texture for the player and bg
//add 2 player to the game
//Make the camera follow a center point between 2 players
//add asteroids in the game.
//changes textures for aircraft and background
//in every update call, check for collision in the air layer and apply damage
//Added the IsGameOver function to check if the game is over and return the dead player.

#include "World.hpp"
#include <iostream>

World::World(sf::RenderWindow& window, FontHolder& font)
	:m_window(window)
	,m_camera(window.getDefaultView())
	,m_textures()
	,m_fonts(font)
	,m_scenegraph()
	,m_scene_layers()
	,m_world_bounds(0.f,0.f, 5000.f, 5000.f) //make the background image bigger that canvas
	,m_spawn_position()
	,m_scrollspeed(-50.f)
	, m_player_1(nullptr)
	, m_player_2(nullptr)
{

	//center spawn position
	m_spawn_position.x = m_world_bounds.width / 2.f;
	m_spawn_position.y = m_world_bounds.height / 2.f;
	

	LoadTextures();
	BuildScene();

	m_camera.setCenter(m_spawn_position);
	m_player_1->SetVelocity(0.f, 0.f);
}



void World::Update(sf::Time dt)
{
	//get camera to center on the player
	m_camera.setCenter(m_player_1->getPosition());

	//get mouse position
	sf::Vector2i mousePos = sf::Mouse::getPosition(m_window);
	
	//Forward the commands to the scenegraph, sort out velocity
	while (!m_command_queue.IsEmpty())
	{
		std::cout << "Command queue is not empty" << std::endl;
		m_scenegraph.OnCommand(m_command_queue.Pop(), dt);
	}
	
	//apply friction to the player movement
	AdaptPlayerPosition(m_player_1);
	//AdaptPlayerPosition(m_player_2);

	m_player_1->RotateInMouseDirection(mousePos,m_window);
	
	m_player_1->ApplyFriction();
	//m_player_2->ApplyFriction();

	//calculate damage & collision in air layer
	m_air_layer->DetectCollisionAndApplyDamage();

	m_scenegraph.Update(dt, m_command_queue);
	
}

void World::Draw()
{
	m_window.setView(m_camera);
	m_window.draw(m_scenegraph);
}

CommandQueue& World::GetCommandQueue()
{
	return m_command_queue;
}

AircraftType World::IsGameOver()
{
	if (m_player_1->IsDestroyed())
		return AircraftType::kPlayer1;
	//else if (m_player_2->IsDestroyed())
	//	return AircraftType::kPlayer2;
	else
		return AircraftType::kNone;
}

void World::LoadTextures()
{
	//load background texture
	m_textures.Load(Texture::kBackground, "Media/Textures/greenNebula.png");
	//load player 1 & 2
	m_textures.Load(Texture::kPlayer1, "Media/Textures/Spaceships/01/Spaceship_01_BLUE.png");
	m_textures.Load(Texture::kPlayer2, "Media/Textures/Spaceships/01/Spaceship_01_RED.png");

	//load asteroide texture
	m_textures.Load(Texture::kAsteroid, "Media/Textures/asteroid.png");

	//loads bullet texture
	m_textures.Load(Texture::kBullet, "Media/Textures/Bullet.png");


}

void World::BuildScene()
{
	//Initialize the different layers
	for(std::size_t i=0; i < static_cast<int>(Layers::kLayerCount); ++i)
	{ 
		SceneNode::Ptr layer(new SceneNode());
		m_scene_layers[i] = layer.get();
		layer->sceneNodeName = "Layer " + std::to_string(i);
		m_scenegraph.AttachChild(std::move(layer));
	}

	//Prepare the background
	sf::Texture& texture = m_textures.Get(Texture::kBackground);
	sf::IntRect textureRect(m_world_bounds);
	//expand textureRect to make the background bigger than the canvas
	int sizeIncrease = 1920;
	textureRect.width += sizeIncrease;
	textureRect.height += sizeIncrease;
	texture.setRepeated(true);

	//Add the background sprite to the world
	std::unique_ptr<SpriteNode> background_sprite(new SpriteNode(texture, textureRect));
	background_sprite->setPosition(m_world_bounds.left - sizeIncrease/2, m_world_bounds.top - sizeIncrease / 2);
	background_sprite->sceneNodeName = "Background";
	m_scene_layers[static_cast<int>(Layers::kBackground)]->AttachChild(std::move(background_sprite));

	//air layer
	m_air_layer = m_scene_layers[static_cast<int>(Layers::kAir)];
	m_air_layer->sceneNodeName = "airlayer";

	//Add player's 1 aircraft
	std::unique_ptr<Aircraft> player1(new Aircraft(AircraftType::kPlayer1, m_textures, m_fonts, m_air_layer));
	m_player_1 = player1.get();
	m_player_1->setPosition(m_spawn_position);
	
	////Add player's 2 aircraft
	//sf::Vector2f spawnPosition2 = m_spawn_position + sf::Vector2f(100.f, 0.f);
	//std::unique_ptr<Aircraft> player2(new Aircraft(AircraftType::kPlayer2, m_textures, m_fonts, m_air_layer));
	//m_player_2 = player2.get();
	//m_player_2->setPosition(spawnPosition2);

	m_scene_layers[static_cast<int>(Layers::kAir)]->AttachChild(std::move(player1));
	//m_scene_layers[static_cast<int>(Layers::kAir)]->AttachChild(std::move(player2));

	//add 30 asteroids
	SpawnAsteroides(60);

}

void World::AdaptPlayerPosition(Aircraft* player)
{
	sf::FloatRect view_bounds(m_camera.getCenter() - m_camera.getSize() / 2.f, m_camera.getSize());
	sf::FloatRect world_bounds = (m_spawn_position, m_world_bounds);
	const float border_distance = 40.f;

	//keep player in the world
	sf::Vector2f position = player->getPosition();
	position.x = std::max(position.x, world_bounds.left + border_distance);
	position.x = std::min(position.x, world_bounds.left + world_bounds.width - border_distance);
	position.y = std::max(position.y, world_bounds.top + border_distance);
	position.y = std::min(position.y, world_bounds.top + world_bounds.height - border_distance);
	player->setPosition(position);


	//Keep the player on the view scene 
	position = player->getPosition();
	position.x = std::max(position.x, view_bounds.left + border_distance);
	position.x = std::min(position.x, view_bounds.left + view_bounds.width - border_distance);
	position.y = std::max(position.y, view_bounds.top + border_distance);
	position.y = std::min(position.y, view_bounds.top + view_bounds.height - border_distance);
	player->setPosition(position);

}


void World::AdaptPlayerVelocity()
{
	sf::Vector2f velocity = m_player_1->GetVelocity();

	//If they are moving diagonally divide by root 2
	if (velocity.x != 0.f && velocity.y != 0.f)
	{
		m_player_1->SetVelocity(velocity / std::sqrt(2.f));
	}

	//Add scrolling velocity
	//m_player_1->Accelerate(0.f, m_scrollspeed);
}

void World::SpawnAsteroides(int nbAsteroides)
{
	//list of existing asteroides position and size
	std::vector<sf::Vector2f> existingAsteroides;
	std::vector<int> existingAsteroidesSize;

	//Spawn the asteroids
	for (int i = 0; i < nbAsteroides; i++)
	{
		//get a random size between 50px and 200px with a step of 50px
		int size = 50 * (rand() % 4 + 1);
		
		sf::Vector2f pos = GetRandomPosition(size, existingAsteroides, existingAsteroidesSize);

		//skip if there is no valid position for this size. Get random position return a (0,0) if there where to many attemps 
		if (pos == sf::Vector2f(0.f, 0.f)) continue;

		//add pos and size to history
		existingAsteroides.push_back(pos);
		existingAsteroidesSize.push_back(size);
		
		//create it and add it to the scene
		std::unique_ptr<Asteroid> asteroid(new Asteroid(size, m_textures));
		asteroid->setPosition(pos);
		asteroid->sceneNodeName = "Asteroid " + std::to_string(i);
		m_scene_layers[static_cast<int>(Layers::kAir)]->AttachChild(std::move(asteroid));
	}
}

sf::Vector2f World::GetRandomPosition(int size,std::vector<sf::Vector2f> existingAsteroides, std::vector<int> existingAsteroidesSize)
{
	
	bool tooClose = false;
	//margin in px
	int margin = 30;

	//position is within word size
	int marginWorld = 50;
	int xRange = (m_world_bounds.width / 2.f)  - marginWorld;
	int yRange = (m_world_bounds.height / 2.f) - marginWorld;
	float x = (rand() % (2 * xRange) - xRange) + m_spawn_position.x;
	float y = (rand() % (2 * yRange) - yRange) + m_spawn_position.x;

	int attemps = 0;

	//while the position is too close from an existing asteriod, we try other position within 10 attemps limit.
	do {
		for (int j = 0; j < existingAsteroides.size(); j++)
		{
			float distance = std::sqrt((existingAsteroides[j].x - x) * (existingAsteroides[j].x - x) + (existingAsteroides[j].y - y) * (existingAsteroides[j].y - y));
			if (distance <= existingAsteroidesSize[j] + size+ margin)
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
	} while (tooClose && attemps<10);

	//return (0,0) position if there is no room for the asteroid of the size given.
	if (attemps == 10)
	{
		x = 0.f;
		y = 0.f;
	}

	return sf::Vector2f(x, y);

	
	
}



