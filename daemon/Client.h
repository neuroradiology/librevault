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

#define EXIT_RESTART 451

#include "util/BandwidthCounter.h"
#include "config/FolderSettings_fwd.h"
#include <QCoreApplication>
#include <QMap>

namespace librevault {

/* Components */
class NodeKey;
class PeerServer;
class GenericNatService;

class FolderGroup;

class BandwidthCounter;

class BTProvider;
class DHTProvider;
class MulticastProvider;

class Webserver;

class Client : public QCoreApplication {
  Q_OBJECT
 public:
  Client(int argc, char** argv);
  virtual ~Client();

 public slots:
  void restart();
  void shutdown();

 private:
  BandwidthCounter bc_all_, bc_blocks_;
  NodeKey* node_key_;
  GenericNatService* portmanager_;
  PeerServer* peerserver_;

  BTProvider* bt_;
  DHTProvider* dht_;
  MulticastProvider* mcast_;

  Webserver* webserver_;

  // Folders
  QMap<QByteArray, FolderGroup*> groups_;

 private slots:
  void initDiscovery();
  void deinitDiscovery();

  void initFolder(const models::FolderSettings& params);
  void deinitFolder(const QByteArray& folderid);

  void initializeAll();
  void deinitializeAll();
};

} /* namespace librevault */
