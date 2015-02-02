/**
 * /file Persist.cpp
 * /brief Implementation of Persist.
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

#include "sys.h"

#include "Persist.h"
#include "xml/Reader.h"
#include "xml/Writer.h"

#include <boost/filesystem/fstream.hpp>

void Persist::read_from_disk(void)
{
  xml::Reader reader;
  reader.parse(m_path, 1);
  xml(reader);
}

void Persist::write_to_disk(void)
{
  boost::filesystem::ofstream stream;
  stream.open(m_path);
  xml::Writer writer(stream);
  writer.write(*this);
}
