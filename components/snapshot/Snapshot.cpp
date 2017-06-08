/* Copyright (C) 2017 Alexander Shishenko <alex@shishenko.com>
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
#include "Snapshot.h"
#include <Snapshot.pb.h>

namespace librevault {

Snapshot::Snapshot() {
	d = new SnapshotData;
	d->polynomial = 0x3DA3358B4DC173LL;
	d->polynomial_degree = 53;
	d->polynomial_shift = 53 - 8;
	d->avg_bits = 20;
	d->max_chunksize = 8 * 1024 * 1024;
	d->min_chunksize = 1 * 1024 * 1024;
}

Snapshot::Snapshot(const serialization::Snapshot& serialized) {
	d = new SnapshotData;
	d->revision = serialized.revision();
	d->inode_hash.reserve(serialized.inode_hash_size());
	for(const auto& inode_hash : serialized.inode_hash())
		d->inode_hash.push_back(QByteArray::fromStdString(inode_hash));
	d->max_chunksize = serialized.max_chunksize();
	d->min_chunksize = serialized.min_chunksize();
	d->polynomial = serialized.polynomial();
	d->polynomial_degree = serialized.polynomial_degree();
	d->polynomial_shift = serialized.polynomial_shift();
	d->avg_bits = serialized.avg_bits();
}

Snapshot::operator serialization::Snapshot() const {
	serialization::Snapshot serialized;
	serialized.set_revision(d->revision);
	for(const auto& inode_hash : qAsConst(d->inode_hash))
		serialized.add_inode_hash(inode_hash.toStdString());
	serialized.set_max_chunksize(d->max_chunksize);
	serialized.set_min_chunksize(d->min_chunksize);
	serialized.set_polynomial(d->polynomial);
	serialized.set_polynomial_degree(d->polynomial_degree);
	serialized.set_polynomial_shift(d->polynomial_shift);
	serialized.set_avg_bits(d->avg_bits);
	return serialized;
}

} /* namespace librevault */
