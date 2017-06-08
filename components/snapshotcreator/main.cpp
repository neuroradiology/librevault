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
#include "InodeScanner.h"
#include "Snapshot.h"
#include "IInodeStorage.h"
#include <PathNormalizer.h>
#include <Secret.h>
#include <QDir>
#include <QDirIterator>
#include <QDebug>

void scanFile(QString abspath) {


}

int main() {
	librevault::Secret secret;
	qDebug() << secret;
	QString root = "/home/gamepad/Experiment";

	librevault::Snapshot snapshot;

	QDirIterator it(root, QDir::AllEntries | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
	while(it.hasNext()) {
		it.next();
		qDebug() << it.filePath() << librevault::PathNormalizer::normalizePath(it.filePath(), root);

		librevault::NullInodeStorage null_inode_storage;
		librevault::InodeScanner scanner(it.filePath(), secret, root, snapshot, &null_inode_storage);
		librevault::Inode inode = scanner.createInode();
		qDebug() << inode.chunks().size();
	}

	return 0;
}
