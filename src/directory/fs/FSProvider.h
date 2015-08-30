/* Copyright (C) 2015 Alexander Shishenko <GamePad64@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include "../../pch.h"
#include "../Abstract.h"

namespace librevault {

class FSDirectory;
class FSProvider : public AbstractProvider {
	std::map<fs::path, std::shared_ptr<FSDirectory>> path_dir_;

	void register_directory(std::shared_ptr<FSDirectory> dir_ptr);
	void unregister_directory(std::shared_ptr<FSDirectory> dir_ptr);
public:
	FSProvider(Session& session, Exchanger& exchanger);
	virtual ~FSProvider();

	void add_directory(ptree dir_options);
	std::shared_ptr<FSDirectory> get_directory(const fs::path& path){return path_dir_[path];}
};

} /* namespace librevault */