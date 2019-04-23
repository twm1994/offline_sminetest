#include "client.h"
#include "utility.h"
#include "map.h"
#include <iostream>
#include "main.h"
#include <jmutexautolock.h>
using namespace jthread;
Client::Client(scene::ISceneManager* smgr, video::SMaterial *materials) :
		m_env(
				new ClientMap(this, materials, smgr->getRootSceneNode(), smgr,
						666), dout_client), m_smgr(smgr) {
	m_env_mutex.Init();
	{
		JMutexAutoLock envlock(m_env_mutex);
		m_env.getMap().load(CLIENT_MAP_FILE);
		m_env.getMap().StartUpdater();
		Player *player = new Player(true, smgr->getRootSceneNode(), smgr, 0);
		f32 y = BS * 2 + BS * 20;
		player->setPosition(v3f(0, y, 0));
		m_env.addPlayer(player);
		//add 5 random npcs
		for (int i = 0; i < 5; i++) {
			Npc *npc = new Npc(smgr->getRootSceneNode(), smgr, i + 100);
			// Limit NPC position within the map
			f32 x = ((float) rand() / (float) (RAND_MAX / 2) - 1.0) * MAP_LENGTH;
			f32 z = ((float) rand() / (float) (RAND_MAX / 2) - 1.0) * MAP_WIDTH;
			npc->setPosition(v3f(x, y, z));
			npc->setRotation(
					v3f(0,
							((float) rand() / (float) (RAND_MAX / 2) - 1.0)
									* 180, 0));
			m_env.addNpc(npc);
		}
	}
}

Client::~Client() {
}

void Client::step(float dtime) {
	// -----Handle add/remove node here-----
	{
		// -----Handle one insert/remove each step-----
		JMutexAutoLock envlock(m_env_mutex);
		core::list<NodeToAdd>::Iterator i_add = m_add_cache.begin();
		if (i_add != m_add_cache.end()) {
			// -----TOCLIENT_ADDNODE-----
			v3s16 p = (*i_add).pos;
			MapNode n((*i_add).node);
			f32 light = m_env.getMap().getNode(p).light;
			m_env.getMap().setNode(p, n);
			m_env.getMap().nodeAddedUpdate(p, light);
			m_add_cache.erase(i_add);
		}
		core::list<v3s16>::Iterator i_remove = m_remove_cache.begin();
		if (i_remove != m_remove_cache.end()) {
			// -----TOCLIENT_REMOVENODE-----
			v3s16 p = *i_remove;
			m_env.getMap().removeNodeAndUpdate(p);
			m_remove_cache.erase(i_remove);
		}
	}
	{
		JMutexAutoLock lock(m_env_mutex);
		m_env.step(dtime);
	}
}

void Client::removeNode(v3s16 nodepos) {
// Test that the position exists
	MapNode node;
	try {
		JMutexAutoLock envlock(m_env_mutex);
		node = m_env.getMap().getNode(nodepos);
	} catch (InvalidPositionException &e) {
		// Fail silently
		std::cout << "No node to remove at (" << nodepos.X << "," << nodepos.Y
				<< "," << nodepos.Z << ")" << std::endl;
		return;
	}
	//-----Avoid destroying boundary and base level-----
	if (node.d != MATERIAL_IGNORE && nodepos.Y != MAP_BOTTOM) {
		// -----Add to m_remove_cache to handle removeNode event-----
		m_remove_cache.push_back(nodepos);
		dout_client << "Client::removeNode() add cache: (" << nodepos.X << ","
				<< nodepos.Y << "," << nodepos.Z << ")" << std::endl;
		dout_client.flush();
		std::cout << "Client::removeNode() add cache: (" << nodepos.X << ","
				<< nodepos.Y << "," << nodepos.Z << ")" << std::endl;
	}
}

void Client::addNode(v3s16 nodepos, MapNode n) {
// Test that the position exists
	try {
		JMutexAutoLock envlock(m_env_mutex);
		m_env.getMap().getNode(nodepos);
		// -----Add to m_add_cache to handle addNode event-----
	} catch (InvalidPositionException &e) {
		// Fail silently
		std::cout << "No node to add at (" << nodepos.X << "," << nodepos.Y
				<< "," << nodepos.Z << ")" << std::endl;
		return;
	}
	//-----Only add node inside boundary-----
	if ((nodepos.X >= -MAP_LENGTH * MAP_BLOCKSIZE)
			&& (nodepos.X < MAP_LENGTH * MAP_BLOCKSIZE)
			&& (nodepos.Y >= -MAP_HEIGHT * MAP_BLOCKSIZE)
			&& (nodepos.Y < MAP_HEIGHT * MAP_BLOCKSIZE)
			&& (nodepos.Z >= -MAP_WIDTH * MAP_BLOCKSIZE)
			&& (nodepos.Z < MAP_WIDTH * MAP_BLOCKSIZE)) {
		NodeToAdd nodeToAdd(nodepos, n);
		m_add_cache.push_back(nodeToAdd);
	}
}

void Client::updateCamera(v3f pos, v3f dir) {
	JMutexAutoLock envlock(m_env_mutex);
	m_env.getMap().updateCamera(pos, dir);
}

MapNode Client::getNode(v3s16 p) {
	JMutexAutoLock envlock(m_env_mutex);
	return m_env.getMap().getNode(p);
}

Player * Client::getLocalPlayer() {
	JMutexAutoLock envlock(m_env_mutex);
	return m_env.getLocalPlayer();
}

core::list<Npc*> Client::getNpcs() {
	JMutexAutoLock envlock(m_env_mutex);
	return m_env.getNpcs();
}
