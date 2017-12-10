/* Copyright (C) 2015 Alexander Shishenko <alex@shishenko.com>
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
 */
#pragma once

#include <QByteArray>
#include <QSharedDataPointer>

namespace librevault {

class Secret;
class ChunkInfoPrivate;
class ChunkInfo {
 public:
  ChunkInfo();
  ChunkInfo(const ChunkInfo& r);
  ChunkInfo(ChunkInfo&& r) noexcept;
  ~ChunkInfo();
  ChunkInfo& operator=(const ChunkInfo& r);
  ChunkInfo& operator=(ChunkInfo&& r) noexcept;

  QByteArray ctHash() const;
  void ctHash(const QByteArray& ct_hash);

  quint64 size() const;
  void size(quint64 size);

  QByteArray iv() const;
  void iv(const QByteArray& iv);

  QByteArray ptKeyedHash() const;
  void ptKeyedHash(const QByteArray& pt_keyed_hash);

  //
  static QByteArray encrypt(const QByteArray& chunk, const Secret& secret, const QByteArray& iv);
  static QByteArray decrypt(const QByteArray& chunk, const Secret& secret, const QByteArray& iv);

  static QByteArray computeHash(const QByteArray& chunk);
  static QByteArray computeKeyedHash(const QByteArray& chunk, const Secret& secret);

  static bool verifyChunk(const QByteArray& ct_hash, const QByteArray& chunk_pt);

 private:
  QSharedDataPointer<ChunkInfoPrivate> d;
};

QDebug operator<<(QDebug debug, const ChunkInfo& info);

}  // namespace librevault
