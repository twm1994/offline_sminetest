#ifndef CLIENT_HEADER
#define CLIENT_HEADER
#include "environment.h"
#include "common_irrlicht.h"
#include <jmutex.h>
#include <exception>

class Client;
struct NodeToAdd {
	NodeToAdd(v3s16 p, MapNode n) {
		pos = p;
		node = n;
	}
	~NodeToAdd() {
		delete &pos;
		delete &node;
	}
	v3s16 pos;
	MapNode node;
};

class Client {
public:
	/*
	 NOTE: Every public method should be thread-safe
	 */
	Client(scene::ISceneManager* smgr, video::SMaterial *materials);
	~Client();
	/*
	 Stuff that references the environment is valid only as
	 long as this is called. (eg. Players)
	 */
	void step(float dtime);
	void removeNode(v3s16 nodepos);
	void addNode(v3s16 nodepos, MapNode n);
	void updateCamera(v3f pos, v3f dir);
	MapNode getNode(v3s16 p);
	// Return value is valid until client is destroyed
	Player * getLocalPlayer();
	// Return value is valid until step()
//	core::list<Player*> getPlayers();
	core::list<Npc*> getNpcs();
	// -----For loading and saving map-----
//	void loadMap();
	void saveMap() {
		m_env.getMap().save(CLIENT_MAP_FILE);
	}

	bool isLoading() {
		return m_env.getMap().isLoading();
	}

private:
	Environment m_env;
	JMutex m_env_mutex;
	scene::ISceneManager* m_smgr;
	core::list<v3s16> m_remove_cache;
	core::list<NodeToAdd> m_add_cache;
};

#endif

