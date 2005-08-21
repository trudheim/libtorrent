// libTorrent - BitTorrent library
// Copyright (C) 2005, Jari Sundell
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// In addition, as a special exception, the copyright holders give
// permission to link the code of portions of this program with the
// OpenSSL library under certain conditions as described in each
// individual source file, and distribute linked combinations
// including the two.
//
// You must obey the GNU General Public License in all respects for
// all of the code used other than OpenSSL.  If you modify file(s)
// with this exception, you may extend this exception to your version
// of the file(s), but you are not obligated to do so.  If you do not
// wish to do so, delete this exception statement from your version.
// If you delete this exception statement from all source files in the
// program, then also delete it here.
//
// Contact:  Jari Sundell <jaris@ifi.uio.no>
//
//           Skomakerveien 33
//           3185 Skoppum, NORWAY

#include "config.h"

#include "torrent/exceptions.h"

#include "storage_file.h"

namespace torrent {

StorageFile::StorageFile() :
  m_meta(NULL),
  m_position(0),
  m_size(0),
  m_completed(0),
  m_priority(1)
{
}

bool
StorageFile::sync() {
  if (!m_meta->prepare(MemoryChunk::prot_read))
    return false;

  off_t pos = 0;

  while (pos != m_size) {
    uint32_t length = std::min(m_size - pos, (off_t)(128 << 20));

    MemoryChunk c = m_meta->get_file().get_chunk(pos, length, MemoryChunk::prot_read, MemoryChunk::map_shared);

    if (!c.is_valid())
      return false;

    c.sync(0, length, MemoryChunk::sync_async);
    c.unmap();

    pos += length;
  }

  return true;
}

bool
StorageFile::resize_file() {
  if (!m_meta->prepare(MemoryChunk::prot_read))
    return false;

  if (m_size == m_meta->get_file().get_size())
    return true;

  if (!m_meta->prepare(MemoryChunk::prot_read | MemoryChunk::prot_write) ||
      !m_meta->get_file().set_size(m_size))
    return false;
  
  m_meta->get_file().reserve();
  return true;
}

}
