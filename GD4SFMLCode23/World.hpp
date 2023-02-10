// HUGO REY D00262075 : add SpawnAsteroides function to spawn the asteroids in the limit of the world.

#pragma once
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "SceneNode.hpp"
#include "SpriteNode.hpp"
#include "Aircraft.hpp"
#include "Asteroid.hpp"
#include "Layers.hpp"

#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>


#include <array>
#include "CommandQueue.hpp"



class World : private sf::NonCopyable
{
public:
	explicit World(sf::RenderWindow& window, FontHolder& font);
	void Update(sf::Time dt);
	void Draw();
	CommandQueue& GetCommandQueue();
	AircraftType IsGameOver();
	
private:
	void LoadTextures();
	void BuildScene();
	void AdaptPlayerPosition(Aircraft* player);
	void AdaptPlayerVelocity();
	void SpawnAsteroides(int nbAsteroides);
	sf::Vector2f GetRandomPosition(int size,std::vector<sf::Vector2f> existingAsteroides,std::vector<int> existingAsteroidesSize);

private:
	sf::RenderWindow& m_window;
	sf::View m_camera;
	TextureHolder m_textures;
	FontHolder& m_fonts;
	SceneNode m_scenegraph;
	std::array<SceneNode*, static_cast<int>(Layers::kLayerCount)> m_scene_layers;
	SceneNode* m_air_layer;

	CommandQueue m_command_queue;

	sf::FloatRect m_world_bounds;
	sf::Vector2f m_spawn_position;
	float m_scrollspeed;
	Aircraft* m_player_1;
	Aircraft* m_player_2;
};

