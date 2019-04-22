#include "client.h"
#include "utility.h"
#include "map.h"
#include <iostream>
//#include "clientserver.h"
#include "main.h"
#include <jmutexautolock.h>
using namespace jthread;
//#ifdef _WIN32
//#include <windows.h>
//#define sleep_ms(x) Sleep(x)
//#else
//#include <unistd.h>
//#define sleep_ms(x) usleep(x*1000)
//#endif

/*
 FIXME: This thread can access the environment at any time
 */

//void * ClientUpdateThread::Thread() {
//	ThreadStarted();
//
//	while (getRun()) {
//		m_client->AsyncProcessData();
//		sleep_ms(200);
//	}
//
//	return NULL;
//}
//Client::Client(scene::ISceneManager* smgr, video::SMaterial *materials) :
//		m_thread(this), m_env(
//				new ClientMap(this, materials, smgr->getRootSceneNode(), smgr,
//						666), dout_client), m_con(PROTOCOL_ID, 512), m_smgr(
//				smgr) {
//m_incoming_queue_mutex.Init();
//m_env_mutex.Init();
//	m_con_mutex.Init();
//	m_thread.Start();
Client::Client(scene::ISceneManager* smgr, video::SMaterial *materials) :
		m_env(
				new ClientMap(this, materials, smgr->getRootSceneNode(), smgr,
						666), dout_client), m_smgr(smgr) {
	std::cout << "Client::Client() generating map" << std::endl;
	m_env.getMap().load();
	m_env_mutex.Init();
	{
//		loadMap();
		JMutexAutoLock envlock(m_env_mutex);
		m_env.getMap().StartUpdater();
		Player *player = new Player(true, smgr->getRootSceneNode(), smgr, 0);
		//f32 y = BS*2 + m_env.getMap().getGroundHeight(v2s16(0,0));
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
//	m_thread.setRun(false);
//	while (m_thread.IsRunning())
//		sleep_ms(100);
}

//void Client::connect(Address address) {
//	{
//		JMutexAutoLock lock(m_con_mutex);
//		m_con.setTimeoutMs(0);
//		m_con.Connect(address);
//	}
//}

void Client::step(float dtime) {
	// -----Handle add/remove node here-----
	{
		// -----Handle one insert/remove each step-----
		JMutexAutoLock envlock(m_env_mutex);
//		dout_client << "Client::step() handle m_add_cache" << std::endl;
		core::list<NodeToAdd>::Iterator i_add = m_add_cache.begin();
		if (i_add != m_add_cache.end()) {
			// -----TOCLIENT_ADDNODE-----
			v3s16 p = (*i_add).pos;
			MapNode n((*i_add).node);
//			dout_client << "Client::step() add node at (" << p.X << "," << p.Y
//					<< "," << p.Z << "), type:" << int(n.d) << std::endl;
//			std::cout << "Client::step() add node at (" << p.X << "," << p.Y
//					<< "," << p.Z << "), type:" << int(n.d) << std::endl;

			f32 light = m_env.getMap().getNode(p).light;
//			dout_client << "Client::step() set node at (" << p.X << "," << p.Y
//					<< "," << p.Z << "), type:" << int(n.d) << " to environment"
//					<< std::endl;
//			dout_client.flush();
			m_env.getMap().setNode(p, n);
			m_env.getMap().nodeAddedUpdate(p, light);
			m_add_cache.erase(i_add);
//			dout_client << "Client::step() set node at (" << p.X << "," << p.Y
//					<< "," << p.Z << "), type:" << int(n.d)
//					<< " to environment finish" << std::endl;
//			dout_client << "Client::step() node added at (" << p.X << "," << p.Y
//					<< "," << p.Z << "), type:" << int(n.d) << std::endl;
//			dout_client.flush();
//			std::cout << "Client::step() Node added  at (" << p.X << "," << p.Y
//					<< "," << p.Z << "), type:" << n.d << std::endl;
		}
//		dout_client << "Client::step() handle m_add_cache done" << std::endl;
//		dout_client.flush();
//		dout_client << "Client::step() handle m_remove_cache" << std::endl;
		core::list<v3s16>::Iterator i_remove = m_remove_cache.begin();
		if (i_remove != m_remove_cache.end()) {
			// -----TOCLIENT_REMOVENODE-----
			v3s16 p = *i_remove;
//			dout_client << "Client::step() remove node at (" << p.X << ","
//					<< p.Y << "," << p.Z << ")" << std::endl;
//			dout_client.flush();
//			std::cout << "Client::step() remove node at (" << p.X << "," << p.Y
//					<< "," << p.Z << ")" << std::endl;
			m_env.getMap().removeNodeAndUpdate(p);
			m_remove_cache.erase(i_remove);
//			dout_client << "Client::step() Node removed at (" << p.X << ","
//					<< p.Y << "," << p.Z << ")" << std::endl;
//			dout_client.flush();
//			std::cout << "Client::step() Node removed  at (" << p.X << ","
//					<< p.Y << "," << p.Z << ")" << std::endl;
		}
//		dout_client << "Client::step() handle m_remove_cache done" << std::endl;
//		dout_client.flush();
//		dout_client << "Client::step() loop through m_add_cache"<< std::endl;
//		for (i_add = m_add_cache.begin(); i_add != m_add_cache.end(); i_add++) {
//			// -----TOCLIENT_ADDNODE-----
//			v3s16 p = (*i_add).pos;
//			MapNode n((*i_add).node);
//			dout_client << "Client::step() add node at (" << p.X << "," << p.Y
//					<< "," << p.Z << "), type:" << int(n.d) << std::endl;
//			std::cout << "Client::step() add node at (" << p.X << "," << p.Y
//					<< "," << p.Z << "), type:" << int(n.d) << std::endl;
//
//			f32 light = m_env.getMap().getNode(p).light;
//			dout_client << "Client::step() set node at (" << p.X << "," << p.Y
//					<< "," << p.Z << "), type:" << int(n.d) <<" to environment" <<std::endl;
//			dout_client.flush();
//			m_env.getMap().setNode(p, n);
//			m_env.getMap().nodeAddedUpdate(p, light);
//			// ensure the iterator points to valid element after erase
//			i_add=m_add_cache.erase(i_add);
//			dout_client << "Client::step() set node at (" << p.X << "," << p.Y
//					<< "," << p.Z << "), type:" << int(n.d) <<" to environment finish" <<std::endl;
//			dout_client << "Client::step() node added at (" << p.X << "," << p.Y
//					<< "," << p.Z << "), type:" << int(n.d) << std::endl;
//			dout_client.flush();
//			std::cout << "Client::step() Node added  at (" << p.X << "," << p.Y
//					<< "," << p.Z << "), type:" << n.d << std::endl;
//		}
//		dout_client << "Client::step() loop through m_add_cache done"<< std::endl;
//		dout_client.flush();
//		dout_client << "Client::step() loop through m_remove_cache"<< std::endl;
//		for (i_remove = m_remove_cache.begin();
//				i_remove != m_remove_cache.end(); i_remove++) {
//			// -----TOCLIENT_REMOVENODE-----
//			v3s16 p = *i_remove;
//			dout_client << "Client::step() remove node at (" << p.X << ","
//					<< p.Y << "," << p.Z << ")" << std::endl;
//			dout_client.flush();
//			std::cout << "Client::step() remove node at (" << p.X << "," << p.Y
//					<< "," << p.Z << ")" << std::endl;
//			m_env.getMap().removeNodeAndUpdate(p);
//			// ensure the iterator points to valid element after erase
//			i_remove=m_remove_cache.erase(i_remove);
//			dout_client << "Client::step() Node removed at (" << p.X << ","
//					<< p.Y << "," << p.Z << ")" << std::endl;
//			dout_client.flush();
//			std::cout << "Client::step() Node removed  at (" << p.X << ","
//					<< p.Y << "," << p.Z << ")" << std::endl;
//		}
//		dout_client << "Client::step() loop through m_remove_cache done"<< std::endl;
//		dout_client.flush();
	}
//	ReceiveAll();
//	bool connected = false;
//	{
//		JMutexAutoLock lock(m_con_mutex);
//		m_con.RunTimeouts(dtime);
//		connected = m_con.Connected();
//	}
	{
		JMutexAutoLock lock(m_env_mutex);
		m_env.step(dtime);
	}
	/*
	 Send stuff if connected
	 */
//	if (connected) {
//		sendPlayerPos(dtime);
//	}
//
//	/*
//	 Clear old entries from fetchblock history
//	 */
//	{
//		JMutexAutoLock lock(m_fetchblock_mutex);
//
//		core::list<v3s16> remove_queue;
//		core::map<v3s16, float>::Iterator i;
//		i = m_fetchblock_history.getIterator();
//		for (; i.atEnd() == false; i++) {
//			float value = i.getNode()->getValue();
//			value += dtime;
//			i.getNode()->setValue(value);
//			if (value >= 60.0)
//				remove_queue.push_back(i.getNode()->getKey());
//		}
//		core::list<v3s16>::Iterator j;
//		j = remove_queue.begin();
//		for (; j != remove_queue.end(); j++) {
//			m_fetchblock_history.remove(*j);
//		}
//	}
}

//void Client::ReceiveAll() {
//	for (;;) {
//		try {
////			Receive();
//			ProcessData();
//		} catch (std::exception &e) {
//			dout_client << "Client::ReceiveAll() Exception: what()=" << e.what()
//					<< std::endl;
//			std::cout << "Client::ReceiveAll() Exception: what()=" << e.what()
//					<< std::endl;
//			break;
//		}
////		catch (con::NoIncomingDataException &e) {
////			break;
////		} catch (con::InvalidIncomingDataException &e) {
////			dout_client << "Client::ReceiveAll(): "
////					"InvalidIncomingDataException: what()=" << e.what()
////					<< std::endl;
////		}
//	}
//}

//void Client::Receive() {
//	u32 data_maxsize = 10000;
//	Buffer<u8> data(data_maxsize);
//	u16 peer_id;
//	u32 datasize;
//	{
//		JMutexAutoLock lock(m_con_mutex);
//		datasize = m_con.Receive(peer_id, *data, data_maxsize);
//	}
//	ProcessData(*data, datasize, peer_id);
//}
//void Client::ProcessData(u8 *data, u32 datasize, u16 peer_id) {
//// Ignore packets that don't even fit a command
//	if (datasize < 2)
//		return;
//
//	ToClientCommand command = (ToClientCommand) readU16(&data[0]);
//
//// Execute fast commands straight away
//
//	if (command == TOCLIENT_REMOVENODE) {
//		// -----Client::removeNode handle the logic-----
//		if (datasize < 8)
//			return;
//		v3s16 p;
//		p.X = readS16(&data[2]);
//		p.Y = readS16(&data[4]);
//		p.Z = readS16(&data[6]);
//
//		// -----Don't save air node to reduce file size-----
//		core::map<v3s16, s16>::Node *n = m_nodes.find(p);
//		if (n != NULL) {
//			m_nodes.delink(p);
//		}
//
//		{
//			JMutexAutoLock envlock(m_env_mutex);
//			m_env.getMap().removeNodeAndUpdate(p);
//		}
//	} else if (command == TOCLIENT_ADDNODE) {
//		// -----Client::addNode handle the logic---
//		if (datasize < 8 + MapNode::serializedLength())
//			return;
//		v3s16 p;
//		p.X = readS16(&data[2]);
//		p.Y = readS16(&data[4]);
//		p.Z = readS16(&data[6]);
//		MapNode n;
//		n.deSerialize(&data[8]);
//		{
//			JMutexAutoLock envlock(m_env_mutex);
//			f32 light = m_env.getMap().getNode(p).light;
//			m_env.getMap().setNode(p, n);
//			m_env.getMap().nodeAddedUpdate(p, light);
//		}
//	} else if (command == TOCLIENT_PLAYERPOS) {
////		u16 our_peer_id;
////		{
////			JMutexAutoLock lock(m_con_mutex);
////			our_peer_id = m_con.GetPeerID();
////		}
////		// Cancel if we don't have a peer id
////		if (our_peer_id == PEER_ID_NEW) {
////			dout_client << "TOCLIENT_PLAYERPOS cancelled: "
////					"we have no peer id" << std::endl;
////			return;
////		}
//
////		{ //envlock
////			JMutexAutoLock envlock(m_env_mutex);
////
////			u32 player_count = (datasize - 2) / (2 + 12 + 12);
////			u32 start = 2;
////			for (u32 i = 0; i < player_count; i++) {
////				u16 peer_id = readU16(&data[start]);
////
////				// Don't update the position of the local player
////				if (peer_id == our_peer_id) {
////					/*dout_client<<"Client: TOCLIENT_PLAYERPOS: "
////					 "player peer_id="<<peer_id;
////					 dout_client<<" is ours"<<std::endl;
////					 dout_client<<std::endl;*/
////
////					start += (2 + 12 + 12);
////					continue;
////				} else {
////					/*dout_client<<"Client: TOCLIENT_PLAYERPOS: "
////					 "player peer_id="<<peer_id;
////					 dout_client<<std::endl;*/
////				}
////
////				Player *player = m_env.getPlayer(peer_id);
////
////				// Create a player if it doesn't exist
////				if (player == NULL) {
////					player = new Player(false, m_smgr->getRootSceneNode(),
////							m_smgr, peer_id);
////					player->peer_id = peer_id;
////					m_env.addPlayer(player);
////					dout_client << "Client: Adding new player " << peer_id
////							<< std::endl;
////				}
////
////				player->timeout_counter = 0.0;
////
////				v3s32 ps = readV3S32(&data[start + 2]);
////				v3s32 ss = readV3S32(&data[start + 2 + 12]);
////				v3f position((f32) ps.X / 100., (f32) ps.Y / 100.,
////						(f32) ps.Z / 100.);
////				v3f speed((f32) ss.X / 100., (f32) ss.Y / 100.,
////						(f32) ss.Z / 100.);
////				player->setPosition(position);
////				player->speed = speed;
////
////				start += (2 + 12 + 12);
////			}
////		} //envlock
//	}
//// Default to queueing it (for slow commands)
//	else {
//		JMutexAutoLock lock(m_incoming_queue_mutex);
//
//		IncomingPacket packet(data, datasize);
//		m_incoming_queue.push_back(packet);
//	}
//}

//bool Client::AsyncProcessData() {
////-----This function
//	getdata: IncomingPacket packet = getPacket();
//	u8 *data = packet.m_data;
//	u32 datasize = packet.m_datalen;
//
//// An empty packet means queue is empty
//	if (data == NULL) {
//		return false;
//	}
//
//	if (datasize < 2)
//		goto getdata;
//
//	ToClientCommand command = (ToClientCommand) readU16(&data[0]);
//
//	if (command == TOCLIENT_BLOCKDATA) {
//		// Ignore too small packet
//		if (datasize < 8 + MapBlock::serializedLength())
//			goto getdata;
//
//		v3s16 p;
//		p.X = readS16(&data[2]);
//		p.Y = readS16(&data[4]);
//		p.Z = readS16(&data[6]);
////		s16 minX = minVal(p.X);
////		s16 minY = minVal(p.Y);
////		s16 minZ = minVal(p.Z);
////
////		// -----Indexing in block: z*MAP_BLOCKSIZE*MAP_BLOCKSIZE + y*MAP_BLOCKSIZE + x-----
////		for (int z = 0; z < MAP_BLOCKSIZE; z++) {
////			for (int y = 0; y < MAP_BLOCKSIZE; y++) {
////				for (int x = 0; x < MAP_BLOCKSIZE; x++) {
////					// First element of &data[8] is probably_dark
////					int val = getNodeType(
////							(&data[8])[z * MAP_BLOCKSIZE * MAP_BLOCKSIZE
////									+ y * MAP_BLOCKSIZE + x + 1]);
////					v3s16 nodepos = v3s16(minX + x, minY + y, minZ + z);
////					core::map<v3s16, s16>::Node *n = m_nodes.find(nodepos);
////					// Don't overwrite existing node or save air node
////					if ((n == NULL) && (val != 254)) {
////						m_nodes.insert(nodepos, val);
////					}
////				} // for(int x=0;x<MAP_BLOCKSIZE;x++
////			} // for(int y=0;y<MAP_BLOCKSIZE;y++)
////		} // for(int z=0;z<MAP_BLOCKSIZE;z++)
//
////		dout_client << "Client: Thread: BLOCKDATA for (" << p.X << "," << p.Y
////				<< "," << p.Z << ")" << std::endl;
//		{ //envlock
//			JMutexAutoLock envlock(m_env_mutex);
//
//			v2s16 p2d(p.X, p.Z);
//			MapSector *sector = m_env.getMap().getSectorNoGenerate(p2d);
//
//			try {
//				MapBlock *block = sector->getBlockNoCreate(p.Y);
//				block->deSerialize(&data[8]);
//			} catch (InvalidPositionException &e) {
//				MapBlock *block = new MapBlock(&m_env.getMap(), p);
//				block->deSerialize(&data[8]);
//				sector->insertBlock(block);
//			}
//		} //envlock
//
//	} else {
//		dout_client << "Client: Thread: Ingoring unknown command " << command
//				<< std::endl;
//	}
//	goto getdata;
//}

//void Client::Send(u16 channelnum, SharedBuffer<u8> data, bool reliable) {
//	JMutexAutoLock lock(m_con_mutex);
//	m_con.Send(PEER_ID_SERVER, channelnum, data, reliable);
//}

//void Client::fetchBlock(v3s16 p) {
//	JMutexAutoLock conlock(m_con_mutex);
//	if (m_con.Connected() == false)
//		return;
//
//	JMutexAutoLock lock(m_fetchblock_mutex);
//
//// If fetch request was recently sent, cancel
//	if (m_fetchblock_history.find(p) != NULL)
//		return;
//
//	con::Peer *peer = m_con.GetPeer(PEER_ID_SERVER);
//	con::Channel *channel = &(peer->channels[1]);
//
//// Don't allow endless amounts of buffered reliable packets
//	if (channel->incoming_reliables.size() >= 100)
//		return;
//// Don't allow endless amounts of non-acked requests
//	if (channel->outgoing_reliables.size() >= 10)
//		return;
//
//	m_fetchblock_history.insert(p, 0.0);
//
//	SharedBuffer<u8> data(8);
//	writeU16(&data[0], TOSERVER_GETBLOCK);
//	writeS16(&data[2], p.X);
//	writeS16(&data[4], p.Y);
//	writeS16(&data[6], p.Z);
//	m_con.Send(PEER_ID_SERVER, 1, data, true);
//}
//
//IncomingPacket Client::getPacket() {
//	JMutexAutoLock lock(m_incoming_queue_mutex);
//
//	core::list<IncomingPacket>::Iterator i;
//// Refer to first one
//	i = m_incoming_queue.begin();
//
//// If queue is empty, return empty packet
//	if (i == m_incoming_queue.end()) {
//		IncomingPacket packet;
//		return packet;
//	}
//
//// Pop out first packet and return it
//	IncomingPacket packet = *i;
//	m_incoming_queue.erase(i);
//	return packet;
//}

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
//	core::map<v3s16, s16>::Node *n = m_nodes.find(p);
//	if (n != NULL) {
//		m_nodes.delink(p);
//	}
//	SharedBuffer<u8> data(8);
//	writeU16(&data[0], TOSERVER_REMOVENODE);
//	writeS16(&data[2], nodepos.X);
//	writeS16(&data[4], nodepos.Y);
//	writeS16(&data[6], nodepos.Z);
//	Send(0, data, true);
}

void Client::addNode(v3s16 nodepos, MapNode n) {
// Test that the position exists
	try {
		JMutexAutoLock envlock(m_env_mutex);
		std::cout << "Client::addNode() try to (" << nodepos.X << ","
				<< nodepos.Y << "," << nodepos.Z << "), type:" << int(n.d)
				<< std::endl;
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
		dout_client << "Client::addNode() add cache: (" << nodepos.X << ","
				<< nodepos.Y << "," << nodepos.Z << "), type:" << int(n.d)
				<< std::endl;
		dout_client.flush();
		std::cout << "Client::addNode() add cache: (" << nodepos.X << ","
				<< nodepos.Y << "," << nodepos.Z << "), type:" << int(n.d)
				<< std::endl;

	}
// -----Don't save air node to reduce file size-----
//	if (getNodeType(n.d) != 254)
//		m_nodes.insert(nodepos, getNodeType(n.d));
//
//
//	u8 datasize = 8 + MapNode::serializedLength();
//	SharedBuffer<u8> data(datasize);
//	writeU16(&data[0], TOSERVER_ADDNODE);
//	writeS16(&data[2], nodepos.X);
//	writeS16(&data[4], nodepos.Y);
//	writeS16(&data[6], nodepos.Z);
//	n.serialize(&data[8]);
//	Send(0, data, true);
}

//void Client::sendPlayerPos(float dtime) {
//	JMutexAutoLock envlock(m_env_mutex);
//
//	Player *myplayer = m_env.getLocalPlayer();
//	if (myplayer == NULL)
//		return;
//
//	u16 our_peer_id;
//	{
//		JMutexAutoLock lock(m_con_mutex);
//		our_peer_id = m_con.GetPeerID();
//	}
//
//// Set peer id if not set already
//	if (myplayer->peer_id == PEER_ID_NEW)
//		myplayer->peer_id = our_peer_id;
//// Check that an existing peer_id is the same as the connection's
//	assert(myplayer->peer_id == our_peer_id);
//
//// Update at reasonable intervals (0.2s)
//	static float counter = 0.0;
//	counter += dtime;
//	if (counter < 0.2)
//		return;
//	counter = 0.0;
//
//	v3f pf = myplayer->getPosition();
//	v3s32 position(pf.X * 100, pf.Y * 100, pf.Z * 100);
//	v3f sf = myplayer->speed;
//	v3s32 speed(sf.X * 100, sf.Y * 100, sf.Z * 100);
//
//	/*static v3s32 oldpos(-104300300,-10004300,-13234344);
//	 static v3s32 oldspeed(-1234344,-2323424,-2343241);
//	 if(position == oldpos && speed == oldspeed)
//	 return;
//	 oldpos = position;
//	 oldspeed = speed;*/
//
//	SharedBuffer<u8> data(2 + 12 + 12);
//	writeU16(&data[0], TOSERVER_PLAYERPOS);
//	writeV3S32(&data[2], position);
//	writeV3S32(&data[2 + 12], speed);
//// Send as unreliable
//	Send(0, data, false);
//}

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
//
//core::list<Player*> Client::getPlayers() {
//	JMutexAutoLock envlock(m_env_mutex);
//	return m_env.getPlayers();
//}

core::list<Npc*> Client::getNpcs() {
	JMutexAutoLock envlock(m_env_mutex);
	return m_env.getNpcs();
}

//void Client::loadMap() {
//	std::ifstream ifs("gened_map.json");
//	Json::CharReaderBuilder reader;
//	Json::Value map;
//	JSONCPP_STRING errs;
//	Json::parseFromStream(reader, ifs, &map, &errs);
//	for (Json::Value::const_iterator i = map.begin(); i != map.end(); i++) {
//		Json::Value pos = (*i)["0"];
//		v3s16 nodePos = v3s16(pos[0].asInt(), pos[1].asInt(), pos[2].asInt());
//		s16 d = (*i)["1"].asInt();
//		m_nodes.insert(nodePos, d);
//	}
//	std::cout << "-----Client::loadMap() generating map-----" << std::endl;
	//	m_env.getMap().addBoundary();
//	m_env.getMap().setSectors();
//}

void Client::saveMap() {
	m_env.getMap().save();
}
