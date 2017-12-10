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
#include "hiersvc/HierarchicalService.h"
#include <QAbstractSocket>
#include <QHostAddress>
#include <QLoggingCategory>
#include <QNetworkInterface>
#include <QPointer>

namespace librevault {

Q_DECLARE_LOGGING_CATEGORY(log_portmapping)

using Protocol = QAbstractSocket::SocketType;
using Duration = std::chrono::seconds;
using TimePoint = std::chrono::system_clock::time_point;
class PortMapping;

struct MappingRequest {
  quint16 internal_port = 0;
  quint16 external_port = 0;
  Protocol protocol;
  QString description;
  Duration ttl;
  QNetworkInterface interface;
};

class GenericNatService : public HierarchicalService {
  Q_OBJECT

 public:
  explicit GenericNatService(QObject* parent = nullptr) : HierarchicalService(nullptr, parent) {}
  ~GenericNatService() override = default;

  virtual PortMapping* createMapping(const MappingRequest& request) = 0;
};

class PortMapping : public HierarchicalService {
  Q_OBJECT

 public:
  PortMapping(const MappingRequest& request, GenericNatService* service, QObject* parent = nullptr);
  ~PortMapping() override = default;

  Q_SIGNAL void mapped(quint16 external_port, QHostAddress external_address, TimePoint expiration);

  quint16 internalPort() const { return request_.internal_port; }
  quint16 externalPort() const {
    return isMapped() ? actual_external_port_ : request_.internal_port;
  }
  Protocol protocol() const { return request_.protocol; }
  virtual QHostAddress externalAddress() const { return external_address_; }
  QString description() const { return request_.description; }
  virtual TimePoint expiration() const { return expiration_; }
  Duration requestTtl() const { return request_.ttl; }
  QNetworkInterface interface() const { return request_.interface; }

  bool isMapped() const { return actual_external_port_ != 0; }

 protected:
  QPointer<GenericNatService> service_;

  quint16 actual_external_port_ = 0;
  QHostAddress external_address_;
  TimePoint expiration_;
  const MappingRequest request_;
};

}  // namespace librevault
