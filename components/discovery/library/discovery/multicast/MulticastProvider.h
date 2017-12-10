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

#include "../GenericProvider.h"
#include <QUdpSocket>

namespace librevault {

class MulticastGroup;

class MulticastProvider : public GenericProvider {
  Q_OBJECT

 public:
  explicit MulticastProvider(QObject* parent);

  Endpoint endpoint() const { return group_endpoint_; }
  QUdpSocket* getSocket() { return socket_; }

  Q_SIGNAL void discovered(const QByteArray& id, const Endpoint& endpoint);
  Q_SLOT void setGroupEndpoint(const Endpoint& endpoint) { group_endpoint_ = endpoint; }

 protected:
  void start() override;
  void stop() override;

 private:
  Endpoint group_endpoint_;
  QUdpSocket* socket_;

  static constexpr size_t buffer_size_ = 65535;

  Q_SLOT void processDatagram(QUdpSocket* socket);
};

} /* namespace librevault */
