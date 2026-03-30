/*
 *   Copyright (c) 2026 by Geoffrey Merck F4FXL / KC3FRA
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#pragma once

#include <string>
#include <optional>
#include <cstdint>
#include <nlohmann/json.hpp>

/*
 * Represents the "last heard" information for a user.
 * This block is optional and only present for user records.
 */
struct LastHeard
{
    std::string repeater;   // Repeater callsign (e.g. "F1ZXX B")
    std::string gateway;    // Gateway callsign (e.g. "F1ZXX G")
    std::string at;         // ISO8601 timestamp (UTC)
};

/*
 * Represents a generic DHT record.
 *
 * This structure is used for both:
 *  - repeater presence (kind = "repeater")
 *  - user presence     (kind = "user")
 *
 * The difference is that "user" records include the optional last_heard block.
 */
struct DHTRecord
{
    uint32_t version = 1;           // Schema version

    std::string kind;               // "repeater" or "user"
    std::string callsign;           // Subject callsign (user or repeater)

    std::string gateway;            // Gateway associated to the subject
    std::string publisher;          // Gateway that published this record

    std::string instance_id;        // Unique instance identifier (per process)

    std::string published_at;       // ISO8601 timestamp (UTC)
    std::string expires_at;         // ISO8601 timestamp (UTC)

    uint64_t seq = 0;               // Monotonic sequence number

    std::optional<LastHeard> last_heard; // Present only for user records
};

/*
 * JSON serialization helpers (nlohmann::json)
 */
void to_json(nlohmann::json& j, const LastHeard& lh);
void from_json(const nlohmann::json& j, LastHeard& lh);

void to_json(nlohmann::json& j, const DHTRecord& rec);
void from_json(const nlohmann::json& j, DHTRecord& rec);