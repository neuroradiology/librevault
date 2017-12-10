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

#include "config/AbstractConfig.h"
#include "config/FolderSettings.h"
#include <QJsonObject>
#include <QObject>
#include <QVariant>
#include "util/exception.hpp"

namespace librevault {

class Config : public AbstractConfig {
  Q_OBJECT
 public:
  ~Config();

  /* Exceptions */
  DECLARE_EXCEPTION(samekey_error,
      "Multiple directories with the same key (or derived from the same key) are not supported");

  static Config* get() {
    if (!instance_) instance_ = new Config();
    return instance_;
  }
  static void deinit() {
    delete instance_;
    instance_ = nullptr;
  }

  /* Global configuration */
  void patchGlobals(const QJsonObject& patch) override;
  QJsonObject getGlobals() override;

  /* Folder configuration */
  void addFolder(QJsonObject fconfig) override;
  void removeFolder(QByteArray folderid) override;

  QJsonObject getFolder(QByteArray folderid) override;
  QList<QByteArray> listFolders() override;

  /* Export/Import */
  QJsonObject exportUserGlobals();
  QJsonObject exportGlobals() override;
  void importGlobals(QJsonDocument globals_conf);

  QJsonArray exportUserFolders();
  QJsonArray exportFolders() override;
  void importFolders(QJsonDocument folders_conf);

 protected:
  Config();

  static Config* instance_;

 private:
  QJsonObject globals_custom_, globals_defaults_, folders_defaults_;
  QMap<QByteArray, QJsonObject> folders_custom_;

  QJsonObject readDefault(const QString& path);
  QJsonDocument readConfig(const QString& source);
  void writeConfig(const QJsonDocument& doc, const QString& target);

  // File config
  void load();
  void save();
};

}  // namespace librevault
