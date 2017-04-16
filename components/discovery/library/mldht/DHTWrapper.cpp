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
#include "DHTWrapper.h"
#include "nativeaddr.h"
#include "rand.h"
#include <dht.h>
#include <QCryptographicHash>

extern "C" {

static void lv_dht_callback_wrapper(void* closure, int event, const unsigned char* info_hash, const void* data, size_t data_len) {
	// Set ID
	QByteArray id((const char*)info_hash, 20);

	// Set AF
	QAbstractSocket::NetworkLayerProtocol af;

	if(event == DHT_EVENT_VALUES || event == DHT_EVENT_SEARCH_DONE)
		af = QAbstractSocket::IPv4Protocol;
	else if(event == DHT_EVENT_VALUES6 || event == DHT_EVENT_SEARCH_DONE6)
		af = QAbstractSocket::IPv6Protocol;
	else
		return;

	// Set nodes
	librevault::DHTWrapper::EndpointList nodes;
	if(af == QAbstractSocket::IPv4Protocol)
		nodes = librevault::btcompat::unpackEnpointList4(QByteArray((const char*)data, data_len));
	else
		nodes = librevault::btcompat::unpackEnpointList6(QByteArray((const char*)data, data_len));

	if(event == DHT_EVENT_VALUES || event == DHT_EVENT_VALUES6)
		((librevault::DHTWrapper*)closure)->foundNodes(id, af, nodes);
	else if(event == DHT_EVENT_SEARCH_DONE || event == DHT_EVENT_SEARCH_DONE6)
		((librevault::DHTWrapper*)closure)->searchDone(id, af, nodes);
}

// DHT library overrides
int dht_blacklisted(const struct sockaddr *sa, int salen) {
//	for(int i = 0; i < salen; i++) {
//		QHostAddress peer_addr(sa+i);
//	}
	return 0;
}

void dht_hash(void *hash_return, int hash_size, const void *v1, int len1, const void *v2, int len2, const void *v3, int len3) {
	std::fill((char*)hash_return, (char*)hash_return + hash_size, 0);

	QCryptographicHash hasher(QCryptographicHash::Sha1);
	hasher.addData((const char*)v1, len1);
	hasher.addData((const char*)v2, len2);
	hasher.addData((const char*)v3, len3);
	QByteArray result = hasher.result();

	std::copy(result.begin(), result.begin()+qMin(result.size(), hash_size), (char*)hash_return);
}

int dht_random_bytes(void *buf, size_t size) {
	fillRandomBuf(buf, size);
	return size;
}

} /* extern "C" */

namespace librevault {

DHTWrapper::DHTWrapper(QUdpSocket* socket4, QUdpSocket* socket6, QByteArray own_id, QObject* parent) {
	int rc = dht_init(socket4->socketDescriptor(), socket6->socketDescriptor(), (const unsigned char*)own_id.leftJustified(20, 0, true).data(), nullptr);
	Q_ASSERT(rc != 0);

	periodic_ = new QTimer(this);
	connect(periodic_, &QTimer::timeout, this, &DHTWrapper::periodicRequest);
	periodic_->start();

	connect(socket4, &QUdpSocket::readyRead, this, [=]{processDatagram(socket4);});
	connect(socket6, &QUdpSocket::readyRead, this, [=]{processDatagram(socket6);});
}

DHTWrapper::~DHTWrapper() {
	periodic_->stop();
	dht_uninit();
}

void DHTWrapper::pingNode(QHostAddress addr, quint16 port) {
	if(!enabled()) return;
	sockaddr_storage sa = convertSockaddr(addr, port);
	dht_ping_node((const sockaddr*)&sa, getSockaddrSize(sa));
}

void DHTWrapper::startAnnounce(QByteArray id, QAbstractSocket::NetworkLayerProtocol af, quint16 port) {
	if(!enabled()) return;
	dht_search((const uint8_t*)id.data(), port, convertAF(af), lv_dht_callback_wrapper, this);
}

void DHTWrapper::startSearch(QByteArray id, QAbstractSocket::NetworkLayerProtocol af) {
	if(!enabled()) return;
	dht_search((const uint8_t*)id.data(), 0, convertAF(af), lv_dht_callback_wrapper, this);
}

void DHTWrapper::nodeCount(int& good_return, int& dubious_return, int& cached_return, int& incoming_return) {
	dht_nodes(AF_INET, &good_return, &dubious_return, &cached_return, &incoming_return);
	int good6, dubious6, cached6, incoming6;
	dht_nodes(AF_INET6, &good6, &dubious6, &cached6, &incoming6);

	good_return += good6;
	dubious_return += dubious6;
	cached_return += cached6;
	incoming_return += incoming6;
}

int DHTWrapper::goodNodeCount() {
	int good, dubious, cached, incoming;
	nodeCount(good, dubious, cached, incoming);
	return good + dubious;
}

DHTWrapper::EndpointList DHTWrapper::getNodes() {
	int good, dubious, cached, incoming;
	nodeCount(good, dubious, cached, incoming);
	int all = good + dubious + cached + incoming;

	DHTWrapper::EndpointList endpoints;
	endpoints.reserve(all);

	std::vector<sockaddr_in> sa4(all);
	std::vector<sockaddr_in6> sa6(all);

	int sa4_count = all;
	int sa6_count = all;

	dht_get_nodes(sa4.data(), &sa4_count, sa6.data(), &sa6_count);

	sa4.resize(sa4_count);
	sa6.resize(sa6_count);

	for(sockaddr_in& sa : sa4)
		endpoints.push_back({QHostAddress((sockaddr*)&sa), qFromBigEndian(sa.sin_port)});

	for(sockaddr_in6& sa : sa6)
		endpoints.push_back({QHostAddress((sockaddr*)&sa), qFromBigEndian(sa.sin6_port)});

	return endpoints;
}

int DHTWrapper::convertAF(QAbstractSocket::NetworkLayerProtocol qaf) {
	if(qaf == QAbstractSocket::IPv4Protocol)
		return AF_INET;
	if(qaf == QAbstractSocket::IPv6Protocol)
		return AF_INET6;
	return 0;
}

void DHTWrapper::processDatagram(QUdpSocket* socket) {
	if(!enabled()) return;

	QByteArray buffer(65535, 0);

	QHostAddress addr;
	quint16 port;
	qint64 datagram_size = socket->readDatagram(buffer.data(), buffer.size(), &addr, &port);
	buffer.resize(datagram_size);

	sockaddr_storage sa = convertSockaddr(addr, port);

	time_t tosleep;
	dht_periodic(buffer.data(), buffer.size()+1, (sockaddr*)&sa, getSockaddrSize(sa), &tosleep, lv_dht_callback_wrapper, this);
	updateNodeCount();

	periodic_->setInterval(tosleep*1000);
}

void DHTWrapper::periodicRequest() {
	if(!enabled()) return;

	time_t tosleep;
	dht_periodic(nullptr, 0, nullptr, 0, &tosleep, lv_dht_callback_wrapper, this);
	updateNodeCount();

	periodic_->setInterval(tosleep*1000);
}

void DHTWrapper::updateNodeCount() {
	int new_node_count = goodNodeCount();
	if(last_node_count_ != new_node_count) {
		last_node_count_ = new_node_count;
		emit nodeCountChanged(new_node_count);
	}
}

} /* namespace librevault */