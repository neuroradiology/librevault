/* Copyright (C) 2016 Alexander Shishenko <alex@shishenko.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
#include "Uploader.h"
#include "folder/storage/ChunkStorage.h"
#include "p2p/Peer.h"
#include <QHash>
#include <QLoggingCategory>

namespace librevault {

Q_LOGGING_CATEGORY(log_uploader, "folder.transfer.uploader")

Uploader::Uploader(ChunkStorage* chunk_storage, QObject* parent)
    : QObject(parent), chunk_storage_(chunk_storage) {}

void Uploader::handleInterested(Peer* peer) {
  // TODO: write good choking algorithm.
  all_interested_.insert(peer, peer->getUnchokeGuard());
}
void Uploader::handleNotInterested(Peer* peer) {
  // TODO: write good choking algorithm.
  all_interested_.remove(peer);
}

void Uploader::untrackPeer(Peer* peer) { all_interested_.remove(peer); }

void Uploader::handleBlockRequest(
    Peer* peer, const QByteArray& ct_hash, quint32 offset, quint32 size) noexcept {
  try {
    if (peer->amChoking() || !peer->peerInterested()) throw ChokeMismatch();

    protocol::v2::Message response;
    response.header.type = protocol::v2::MessageType::BLOCKRESPONSE;
    response.blockresponse.offset = offset;
    response.blockresponse.ct_hash = ct_hash;
    response.blockresponse.content = getBlock(ct_hash, offset, size);
    peer->send(response);
  } catch (const ChunkStorage::NoSuchChunk& e) {
    qCDebug(log_uploader) << "Requested nonexistent block";
  } catch (const BlockOutOfBounds& e) {
    qCDebug(log_uploader) << e.what();
  } catch (const ChokeMismatch& e) {
    qCDebug(log_uploader) << e.what();
  }
}

QByteArray Uploader::getBlock(const QByteArray& ct_hash, quint32 offset, quint32 size) {
  auto chunk = chunk_storage_->getChunk(ct_hash);  // throws ChunkStorage::NoSuchChunk

  if ((int)offset >= chunk.size() || (int)size > (chunk.size() - (int)offset))
    throw BlockOutOfBounds();

  return chunk.mid(offset, size);
}

} /* namespace librevault */
