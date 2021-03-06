/**
 * @file bgcfservice.cpp class implementation for an BGCF service provider
 *
 * Project Clearwater - IMS in the Cloud
 * Copyright (C) 2013  Metaswitch Networks Ltd
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version, along with the "Special Exception" for use of
 * the program along with SSL, set forth below. This program is distributed
 * in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR
 * A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details. You should have received a copy of the GNU General Public
 * License along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * The author can be reached by email at clearwater@metaswitch.com or by
 * post at Metaswitch Networks Ltd, 100 Church St, Enfield EN2 6BQ, UK
 *
 * Special Exception
 * Metaswitch Networks Ltd  grants you permission to copy, modify,
 * propagate, and distribute a work formed by combining OpenSSL with The
 * Software, or a work derivative of such a combination, even if such
 * copying, modification, propagation, or distribution would otherwise
 * violate the terms of the GPL. You must comply with the GPL in all
 * respects for all of the code used other than OpenSSL.
 * "OpenSSL" means OpenSSL toolkit software distributed by the OpenSSL
 * Project and licensed under the OpenSSL Licenses, or a work based on such
 * software and licensed under the OpenSSL Licenses.
 * "OpenSSL Licenses" means the OpenSSL License and Original SSLeay License
 * under which the OpenSSL Project distributes the OpenSSL toolkit software,
 * as those licenses appear in the file LICENSE-OPENSSL.
 */

///


#include <json/reader.h>
#include <fstream>
#include <stdlib.h>

#include "bgcfservice.h"
#include "log.h"

BgcfService::BgcfService(std::string configuration)
{
  Json::Value root;
  Json::Reader reader;

  std::string jsonData;
  std::ifstream file;

  LOG_STATUS("Loading BGCF configuration from %s", configuration.c_str());

  file.open(configuration.c_str());
  if (file.is_open())
  {
    if (!reader.parse(file, root))
    {
      LOG_WARNING("Failed to read BGCF configuration data, %s",
                  reader.getFormattedErrorMessages().c_str());
      return;
    }

    file.close();

    if (root["routes"].isArray())
    {
      Json::Value routes = root["routes"];

      for (size_t ii = 0; ii < routes.size(); ++ii)
      {
        Json::Value route = routes[(int)ii];
        if ((route["domain"].isString()) &&
            (route["route"].isString()))
        {
          std::string domain = route["domain"].asString();
          std::string via = route["route"].asString();
          _routes.insert(std::make_pair(domain, via));
          LOG_STATUS("Added route to %s via %s", domain.c_str(), via.c_str());
        }
        else
        {
          LOG_WARNING("Badly formed BGCF route entry %s", route.toStyledString().c_str());
        }
      }
    }
    else
    {
      LOG_WARNING("Badly formed BGCF configuration file - missing routes object");
    }
  }
  else
  {
    LOG_WARNING("Failed to read BGCF configuration data %d", file.rdstate());
  }

}


BgcfService::~BgcfService()
{
}


std::string BgcfService::get_route(const std::string &domain) const
{
  LOG_DEBUG("Getting route for URI domain %s via BGCF lookup", domain.c_str());

  std::map<std::string, std::string>::const_iterator i = _routes.find(domain);

  if (i != _routes.end())
  {
    LOG_INFO("Found route to domain %s via %s", domain.c_str(), i->second.c_str());
    return i->second;
  }

  return std::string();
}
