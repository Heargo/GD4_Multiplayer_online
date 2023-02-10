#pragma once

#ifndef BOOK_SCENENODE_HPP
#define BOOK_SCENENODE_HPP

#include <SFML/System/NonCopyable.hpp>
#include <SFML/System/Time.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <SFML/Graphics/Drawable.hpp>
#include "CommandQueue.hpp"
#include <memory>
#include <vector>
#include <string>

class Command;


class SceneNode : public sf::Transformable, public sf::Drawable, private sf::NonCopyable
{
public:
	typedef std::unique_ptr<SceneNode> Ptr;
	std::string sceneNodeName;

public:
	SceneNode();
	void AttachChild(Ptr child);
	Ptr DetachChild(const SceneNode& node);

	void Update(sf::Time dt, CommandQueue& commands);

	sf::Vector2f GetWorldPosition() const;
	sf::Transform GetWorldTransform() const;

	void OnCommand(const Command& command, sf::Time dt);

	void DetectCollisionAndApplyDamage(); //const TextureHolder& textures

private:
	virtual void UpdateCurrent(sf::Time dt, CommandQueue& commands);
	void UpdateChildren(sf::Time dt, CommandQueue& commands);

	//Note draw if from sf::Drawable hence the name
	//Do not be tempted to call this Draw
	virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const;
	virtual void DrawCurrent(sf::RenderTarget& target, sf::RenderStates states) const;
	void DrawChildren(sf::RenderTarget& target, sf::RenderStates states) const;
	virtual unsigned int GetCategory() const;
	
private:
	std::vector<Ptr> m_children;
	std::vector<Ptr> m_children_to_remove;
	SceneNode* m_parent;

};
#endif // BOOK_SCENENODE_HPP

