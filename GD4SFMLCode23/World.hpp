#pragma once
#include "State.hpp"
#include "ResourceHolder.hpp"
#include "ResourceIdentifiers.hpp"
#include "SceneNode.hpp"
#include "SpriteNode.hpp"
#include "Aircraft.hpp"
#include "Layers.hpp"
#include "NetworkNode.hpp"
#include "NetworkProtocol.hpp"
#include "PickupType.hpp"

#include <SFML/System/NonCopyable.hpp>
#include <SFML/Graphics/View.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>


#include <array>
#include "CommandQueue.hpp"

#include "BloomEffect.hpp"
#include "SoundPlayer.hpp"

#include <SFML/Network/IpAddress.hpp>
#include <SFML/Network/Packet.hpp>





class World : private sf::NonCopyable
{
public:
	explicit World(sf::RenderTarget& window, FontHolder& font, SoundPlayer& sounds, State::Context context, bool networked=false);
	void Update(sf::Time dt);
	void Draw();

	sf::FloatRect GetViewBounds() const;
	CommandQueue& GetCommandQueue();
	
	Aircraft* AddAircraft(int identifier,bool isLocalPlayer);
	void RemoveAircraft(int identifier);
	void SetCurrentBattleFieldPosition(float line_y);
	void SetWorldHeight(float height);
	
	void AddEnemy(AircraftType type, float rel_x, float rel_y);
	void SortEnemies();
	
	bool HasAlivePlayer() const;
	bool HasPlayerReachedEnd() const;
	
	void SetWorldScrollCompensation(float compensation);
	Aircraft* GetAircraft(int identifier) const;
	sf::FloatRect GetBattlefieldBounds() const;
	void CreatePickup(sf::Vector2f position, PickupType type);
	bool PollGameAction(GameActions::Action& out);

	bool IsLocalPlayer(int identifier);
	
	void SpawnAsteroides(int nbAsteroides);

	void SetSocket(sf::TcpSocket* socket);

	sf::Vector2f validRespawnPosition();

	

private:
	void LoadTextures();
	void BuildScene();
	void AdaptPlayerPosition();
	void AdaptPlayerVelocity();


	void SpawnEnemies();
	void AddEnemies();
	void DestroyEntitiesOutsideView();

	void GuideMissiles();

	void HandleCollisions();

	void UpdateSounds();

	sf::Vector2f GetRandomPosition(int size, std::vector<sf::Vector2f> existingAsteroides, std::vector<int> existingAsteroidesSize);

private:
	struct SpawnPoint
	{
		SpawnPoint(AircraftType type, float x, float y) :m_type(type), m_x(x), m_y(y)
		{

		}
		AircraftType m_type;
		float m_x;
		float m_y;
	};

private:
	sf::RenderTarget& m_target;
	sf::RenderTexture m_scene_texture;
	sf::View m_camera;
	TextureHolder m_textures;
	FontHolder& m_fonts;
	SoundPlayer& m_sounds;
	SceneNode m_scenegraph;
	std::array<SceneNode*, static_cast<int>(Layers::kLayerCount)> m_scene_layers;

	CommandQueue m_command_queue;
	sf::TcpSocket* m_socket;

	sf::FloatRect m_world_bounds;
	sf::Vector2f m_spawn_position;
	float m_scrollspeed;
	float m_scrollspeed_compensation;

	std::vector<Aircraft*> m_player_aircraft;
	std::vector<SpawnPoint> m_enemy_spawn_points;
	std::vector<Aircraft*> m_active_enemies;

	BloomEffect m_bloom_effect;
	bool m_networked_world;
	NetworkNode* m_network_node;
	SpriteNode* m_finish_sprite;

	std::vector<sf::Int32> m_local_player_identifiers;
	State::Context m_context;

	std::vector<sf::Vector2f> m_existingAsteroides;
	std::vector<int> m_existingAsteroidesSize;
};

