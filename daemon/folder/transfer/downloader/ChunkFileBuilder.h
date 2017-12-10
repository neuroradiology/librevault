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
#pragma once
#include "util/AvailabilityMap.h"
#include <QCache>
#include <QFile>

namespace librevault {

/* ChunkFileBuilderFdPool is a singleton class, used to open/close files automatically to reduce
 * simultaneously open file descriptors */
class ChunkFileBuilderFdPool {
 public:
  static ChunkFileBuilderFdPool* get_instance() {
    static ChunkFileBuilderFdPool* instance;
    if (!instance) instance = new ChunkFileBuilderFdPool();
    return instance;
  }

  QFile* getFile(QString path, bool release = false);

 private:
  QCache<QString, QFile> opened_files_;
};

/* ChunkFileBuilder constructs a chunk in a file. If complete(), then an encrypted chunk is located
 * in  */
class ChunkFileBuilder {
 public:
  ChunkFileBuilder(const QString& system_path, const QByteArray& ct_hash, quint32 size);
  ~ChunkFileBuilder();

  QFile* releaseChunk();
  void putBlock(quint32 offset, const QByteArray& content);

  uint64_t size() const { return file_map_.size_original(); }
  bool complete() const { return file_map_.full(); }

  const AvailabilityMap<quint32>& fileMap() const { return file_map_; }

 private:
  AvailabilityMap<quint32> file_map_;
  QString chunk_location_;
};

} /* namespace librevault */
