#include "mapsector.h"
#include <jmutexautolock.h>
using namespace jthread;
#include "exceptions.h"
#include "main.h"

// -----Keep-----
MapBlock * MapSector::getBlockBuffered(s16 y) {
	MapBlock *block;
	if (m_block_cache != NULL && y == m_block_cache_y) {
		return m_block_cache;
	}
	// If block doesn't exist, return NULL
	core::map<s16, MapBlock*>::Node *n = m_blocks.find(y);
	if (n == NULL) {
		/*
		 TODO: Check if block is stored on disk
		 and load dynamically
		 */
		block = NULL;
	}
	// If block exists, return it
	else {
		block = n->getValue();
	}
	// Cache the last result
	m_block_cache_y = y;
	m_block_cache = block;
	return block;
}

// -----Keep-----
MapBlock * MapSector::getBlockNoCreate(s16 y) {
	/*std::cout<<"MapSector("<<m_pos.X<<","<<m_pos.Y<<")::getBlock("<<y
	 <<")"<<std::endl;*/
	JMutexAutoLock lock(m_mutex);
	MapBlock *block = getBlockBuffered(y);
	if (block == NULL)
		throw InvalidPositionException();
	return block;
}

void MapSector::insertBlock(MapBlock *block) {
	s16 block_y = block->getPos().Y;
	{
		JMutexAutoLock lock(m_mutex);
		MapBlock *block = getBlockBuffered(block_y);
		if (block != NULL) {
			throw AlreadyExistsException("Block already exists");
		}
	}
	v2s16 p2d(block->getPos().X, block->getPos().Z);
	assert(p2d == m_pos);
	block->propagateSunlight();
	// Insert to container
	{
		JMutexAutoLock lock(m_mutex);
		m_blocks.insert(block_y, block);
	}
}

core::list<MapBlock*> MapSector::getBlocks() {
	JMutexAutoLock lock(m_mutex);
	core::list<MapBlock*> ref_list;
	core::map<s16, MapBlock*>::Iterator bi;
	bi = m_blocks.getIterator();
	for (; bi.atEnd() == false; bi++) {
		MapBlock *b = bi.getNode()->getValue();
		ref_list.push_back(b);
	}
	return ref_list;
}
//END
