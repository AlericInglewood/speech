/**
 * /file Persist.h
 * /brief Declaration of Persist.
 *
 * Copyright (C) 2015 Aleric Inglewood.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef PERSIST_H
#define PERSIST_H

#include "xml/Bridge.h"
#include <boost/filesystem.hpp>

class Persist
{
  private:
    boost::filesystem::path m_path;

  public:
    Persist() { }
    Persist(boost::filesystem::path const& path) : m_path(path) { }

    void set_path(boost::filesystem::path const& path) { m_path = path; }
    void read_from_disk();
    void write_to_disk();

  public:
    virtual void xml(xml::Bridge& xml) = 0;
};

#endif // PERSIST_H
