#ifndef NPC_HEADER
#define NPC_HEADER

#include <irrlicht.h>
#include "common_irrlicht.h"
#include "map.h"
//#include "connection.h"

using namespace irr;
//typedef core::vector3df v3f;
//typedef core::vector3d<s16> v3s16;

class Map;

/*
 TODO: Make this a scene::ISceneNode
 */

class Npc: public scene::ISceneNode {
public:

	Npc(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id);

	~Npc();

	//void animateStand()
	//{
	//	// avatar_node->setFrameLoop(0,80);
	//	avatar_node->setFrameLoop(168,188);
	//	avatar_node->setAnimationSpeed(32);
	//}

	//void animateMove()
	//{
	//	// avatar_node->setFrameLoop(168,188);
	//	avatar_node->setFrameLoop(0,80);
	//	avatar_node->setAnimationSpeed(32);
	//}

	void move(f32 dtime, Map &map);

	void randomWalk(f32 dtime, Map &map);

	/*
	 ISceneNode methods
	 */

	virtual void OnRegisterSceneNode() {
		if (IsVisible)
			SceneManager->registerNodeForRendering(this);

		ISceneNode::OnRegisterSceneNode();
	}

	virtual void render() {
		// Do nothing
	}

	virtual const core::aabbox3d<f32>& getBoundingBox() const {
		return m_box;
	}

	v3f getPosition() {
		return m_position;
	}

	void setPosition(v3f position) {
		m_position = position;
		updateSceneNodePosition();
	}

	v3f getRotation() {
		return m_rotation;
	}

	void setRotation(v3f rotation) {
		m_rotation = rotation;
		updateSceneNodeRotation();
	}

	int getStepCounter() {
		return m_step_counter;
	}

	void increaseStepCounter() {
		m_step_counter++;
	}

	void resetStepCounter() {
		m_step_counter = 0;
	}

	int getStepMax() {
		return m_step_max;
	}

	void updateSceneNodePosition() {
		ISceneNode::setPosition(m_position);
	}

	void updateSceneNodeRotation() {
		ISceneNode::setRotation(m_rotation);
	}

	v3f speed;
	bool touching_ground;
	u16 npc_id;

private:
	int m_step_counter = 0;
	int m_step_max = 400; // max game step to reset walking direction
	f32 m_walk_speed = 4.0 * BS;
	v3f m_speed_vector = v3f(0, 0, m_walk_speed);
	f32 m_yaw;
	v3f m_position;
	v3f m_rotation;
	bool jump_trigger; // set NPC jumping  
	scene::IAnimatedMesh* avatar;
	scene::IAnimatedMeshSceneNode* avatar_node;
	core::aabbox3d<f32> m_box;
};

#endif

