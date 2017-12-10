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
#include "DirectoryWatcher.h"
#include "config/FolderSettings.h"
#include "folder/IgnoreList.h"
#include "util/conv_fspath.h"
#include <path_normalizer.h>
#include <QTimer>
#include <QLoggingCategory>

namespace librevault {

Q_LOGGING_CATEGORY(log_watcher, "folder.storage.watcher")

DirectoryWatcherThread::DirectoryWatcherThread(QString abspath, QObject* parent)
    : QThread(parent), monitor_(monitor_ios_) {
  qRegisterMetaType<boost::asio::dir_monitor_event>("boost::asio::dir_monitor_event");
  monitor_.add_directory(abspath.toStdString());
  start(QThread::LowPriority);
}

DirectoryWatcherThread::~DirectoryWatcherThread() {
  monitor_ios_.stop();
  this->wait();
}

void DirectoryWatcherThread::run() {
  monitorLoop();
  monitor_ios_.run();
}

void DirectoryWatcherThread::monitorLoop() {
  monitor_.async_monitor([this](boost::system::error_code ec, boost::asio::dir_monitor_event ev) {
    if (ec == boost::asio::error::operation_aborted) return;

    emit dirEvent(ev);
    monitorLoop();
  });
}

DirectoryWatcher::DirectoryWatcher(const models::FolderSettings& params, IgnoreList* ignore_list, QObject* parent)
    : QObject(parent), params_(params), ignore_list_(ignore_list) {
  qRegisterMetaType<boost::asio::dir_monitor_event>("boost::asio::dir_monitor_event");

  watcher_thread_ = new DirectoryWatcherThread(params_.path, this);
  connect(watcher_thread_, &DirectoryWatcherThread::dirEvent, this, &DirectoryWatcher::handleDirEvent,
          Qt::QueuedConnection);
}

DirectoryWatcher::~DirectoryWatcher() = default;

void DirectoryWatcher::handleDirEvent(boost::asio::dir_monitor_event ev) {
  switch (ev.type) {
    case boost::asio::dir_monitor_event::added:
    case boost::asio::dir_monitor_event::modified:
    case boost::asio::dir_monitor_event::renamed_old_name:
    case boost::asio::dir_monitor_event::renamed_new_name:
    case boost::asio::dir_monitor_event::removed:
    case boost::asio::dir_monitor_event::null: {
      QString abspath = conv_fspath(ev.path);
      QByteArray normpath = metakit::normalizePath(abspath, params_.path);

      if (!ignore_list_->isIgnored(normpath)) emit newPath(abspath);
    }
    default:
      break;
  }
}

} /* namespace librevault */
