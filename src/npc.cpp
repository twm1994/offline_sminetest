#include "npc.h"
#define NPC_RADIUS (BS*0.3)
#define NPC_HEIGHT (BS*1.7)
Npc::Npc(scene::ISceneNode* parent, scene::ISceneManager* mgr, s32 id) :
		scene::ISceneNode(parent, mgr, id), speed(0, 0, 0), touching_ground(
				false), npc_id(0), m_position(0, 0, 0) {
	m_box = core::aabbox3d<f32>(-BS, -BS, -BS, BS, BS, BS);

	video::IVideoDriver* driver = SceneManager->getVideoDriver();
	avatar = mgr->getMesh("../data/character.b3d");
	avatar_node = mgr->addAnimatedMeshSceneNode(avatar, this);
	if (avatar_node) {
		avatar_node->setMaterialFlag(video::EMF_LIGHTING, false);
		avatar_node->setMaterialTexture(0,
				driver->getTexture("../data/character.png"));
		avatar_node->setMaterialType(video::EMT_TRANSPARENT_ALPHA_CHANNEL);
		avatar_node->setFrameLoop(200, 220);
		avatar_node->setAnimationSpeed(8);
		// this->animateStand();
		avatar_node->setScale(v3f(1, 1, 1));
		avatar_node->setPosition(v3f(0, 0, 0));
		avatar_node->setRotation(v3f(0, 0, 0));
	}
	updateSceneNodePosition();
	updateSceneNodeRotation();
}

Npc::~Npc() {
	if (SceneManager != NULL)
		ISceneNode::remove();
}

// void Npc::stand()
// {
// 	avatar_node->setFrameLoop(0,80);
// }

// void Npc::sit()
// {
// 	avatar_node->setFrameLoop(81,161);
// }

void Npc::move(f32 dtime, Map &map) {
	v3f position = getPosition();
	v3f oldpos = position;
	v3s16 oldpos_i = Map::floatToInt(oldpos);
	position += speed * dtime;
// The frame length is limited to the npc going 0.1*BS per call
	f32 d = (float) BS * 0.15;
	core::aabbox3d<f32> npcbox(position.X - NPC_RADIUS, position.Y - 0.0,
			position.Z - NPC_RADIUS, position.X + NPC_RADIUS,
			position.Y + NPC_HEIGHT, position.Z + NPC_RADIUS);
	core::aabbox3d<f32> npcbox_old(oldpos.X - NPC_RADIUS, oldpos.Y - 0.0,
			oldpos.Z - NPC_RADIUS, oldpos.X + NPC_RADIUS, oldpos.Y + NPC_HEIGHT,
			oldpos.Z + NPC_RADIUS);
	touching_ground = false;
	jump_trigger = false;
	for (s16 y = oldpos_i.Y - 1; y <= oldpos_i.Y + 2; y++) {
		for (s16 z = oldpos_i.Z - 1; z <= oldpos_i.Z + 1; z++) {
			for (s16 x = oldpos_i.X - 1; x <= oldpos_i.X + 1; x++) {
				try {
					if (map.getNode(x, y, z).d == MATERIAL_AIR) {
						continue;
					}
				} catch (InvalidPositionException &e) {
					// Doing nothing here will block the npc from
					// walking over map borders
				}

				core::aabbox3d<f32> nodebox = Map::getNodeBox(v3s16(x, y, z));

				// See if the npc is touching ground
				if (
				/*(nodebox.MaxEdge.Y+d > npcbox.MinEdge.Y &&
				 nodebox.MaxEdge.Y-d < npcbox.MinEdge.Y)*/
				fabs(nodebox.MaxEdge.Y - npcbox.MinEdge.Y) < d
						&& nodebox.MaxEdge.X - d > npcbox.MinEdge.X
						&& nodebox.MinEdge.X + d < npcbox.MaxEdge.X
						&& nodebox.MaxEdge.Z - d > npcbox.MinEdge.Z
						&& nodebox.MinEdge.Z + d < npcbox.MaxEdge.Z) {
					touching_ground = true;
				}
				if (npcbox.intersectsWithBox(nodebox)) {

					v3f dirs[3] = { v3f(0, 0, 1), // back
					v3f(0, 1, 0), // top
					v3f(1, 0, 0), // right
							};
					for (u16 i = 0; i < 3; i++) {
						f32 nodemax = nodebox.MaxEdge.dotProduct(dirs[i]);
						f32 nodemin = nodebox.MinEdge.dotProduct(dirs[i]);
						f32 npcmax = npcbox.MaxEdge.dotProduct(dirs[i]);
						f32 npcmin = npcbox.MinEdge.dotProduct(dirs[i]);
						f32 npcmax_old = npcbox_old.MaxEdge.dotProduct(dirs[i]);
						f32 npcmin_old = npcbox_old.MinEdge.dotProduct(dirs[i]);

						bool main_edge_collides = ((nodemax > npcmin
								&& nodemax <= npcmin_old + d
								&& speed.dotProduct(dirs[i]) < 0)
								|| (nodemin < npcmax
										&& nodemin >= npcmax_old - d
										&& speed.dotProduct(dirs[i]) > 0));

						bool other_edges_collide = true;
						for (u16 j = 0; j < 3; j++) {
							if (j == i)
								continue;
							f32 nodemax = nodebox.MaxEdge.dotProduct(dirs[j]);
							f32 nodemin = nodebox.MinEdge.dotProduct(dirs[j]);
							f32 npcmax = npcbox.MaxEdge.dotProduct(dirs[j]);
							f32 npcmin = npcbox.MinEdge.dotProduct(dirs[j]);
							if (!(nodemax - d > npcmin && nodemin + d < npcmax)) {
								other_edges_collide = false;
								break;
							}
						}
						if (main_edge_collides && speed.dotProduct(dirs[i]) > 0
								&& z >= oldpos_i.Z && y >= oldpos_i.Y) {
							jump_trigger = true; // jump when collide with node in the front
						}
						if (main_edge_collides && other_edges_collide) {
							speed -= speed.dotProduct(dirs[i]) * dirs[i];
							position -= position.dotProduct(dirs[i]) * dirs[i];
							position += oldpos.dotProduct(dirs[i]) * dirs[i];
						}

					}
				} // if(npcbox.intersectsWithBox(nodebox))
			} // for x
		} // for z
	} // for y
	if (jump_trigger && touching_ground) {
		speed.Y = 9.81 * BS;
	}
	setPosition(position);
}

void Npc::randomWalk(f32 dtime, Map &map) {
	if (m_step_counter == 0) {
		m_yaw = ((float) rand() / (float) (RAND_MAX / 2) - 1.0) * 180;
		setRotation(v3f(0, -1 * m_yaw, 0));
		v3f speed_vector = v3f(0, 0, m_walk_speed);
		speed_vector.rotateXZBy(m_yaw);
		m_speed_vector = speed_vector;
	}
	int b_walk = rand() % 2;
	if (b_walk) {
		speed.X = m_speed_vector.X;
		//if ((rand() % 8 == 0) && touching_ground) {
		//	speed.Y += 3 * BS;
		//}
		speed.Z = m_speed_vector.Z;
		move(dtime, map);
	}
	m_step_counter = m_step_counter + 1;
	if (m_step_counter == m_step_max) {
		m_step_counter = 0;
	}
}
