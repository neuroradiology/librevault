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
#include "DHTGroup.h"
#include "DHTProvider.h"

namespace librevault {

DHTGroup::DHTGroup(QByteArray discovery_id, DHTProvider* provider, QObject* parent)
    : GenericGroup(std::move(discovery_id), provider, parent), provider_(provider) {
  timer_ = new QTimer(this);

  timer_->setInterval(std::chrono::seconds(60));
  timer_->setTimerType(Qt::VeryCoarseTimer);

  connect(provider_, &DHTProvider::discovered, this, &DHTGroup::handleDiscovered);
  connect(timer_, &QTimer::timeout, this, &DHTGroup::startSearches);
}

void DHTGroup::start() {
  QTimer::singleShot(0, this, &DHTGroup::startSearches);
  timer_->start();
  started();
}

void DHTGroup::stop() { timer_->stop(); }

void DHTGroup::startSearches() {
  if (!isOperational()) return;

  qCDebug(log_dht) << "Starting DHT searches for:" << getInfoHash().toHex()
                   << "on port:" << provider_->getAnnouncePort();
  provider_->startAnnounce(
      getInfoHash(), QAbstractSocket::IPv4Protocol, provider_->getAnnouncePort());
  provider_->startAnnounce(
      getInfoHash(), QAbstractSocket::IPv6Protocol, provider_->getAnnouncePort());
}

void DHTGroup::handleDiscovered(const QByteArray& ih, const Endpoint& endpoint) {
  if (!isOperational() || ih != getInfoHash()) return;
  emit discovered(endpoint);
}

} /* namespace librevault */
