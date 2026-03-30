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

#include "DHTRecord.h"

using nlohmann::json;

/*
 * Serialize LastHeard to JSON
 */
void to_json(json& j, const LastHeard& lh)
{
    j = json{
        {"repeater", lh.repeater},
        {"gateway", lh.gateway},
        {"at", lh.at}
    };
}

/*
 * Deserialize LastHeard from JSON
 */
void from_json(const json& j, LastHeard& lh)
{
    j.at("repeater").get_to(lh.repeater);
    j.at("gateway").get_to(lh.gateway);
    j.at("at").get_to(lh.at);
}

/*
 * Serialize DHTRecord to JSON
 */
void to_json(json& j, const DHTRecord& rec)
{
    j = json{
        {"version", rec.version},
        {"kind", rec.kind},
        {"callsign", rec.callsign},
        {"gateway", rec.gateway},
        {"publisher", rec.publisher},
        {"instance_id", rec.instance_id},
        {"published_at", rec.published_at},
        {"expires_at", rec.expires_at},
        {"seq", rec.seq}
    };

    // Add optional last_heard block only if present
    if (rec.last_heard.has_value()) {
        j["last_heard"] = rec.last_heard.value();
    }
}

/*
 * Deserialize DHTRecord from JSON
 */
void from_json(const json& j, DHTRecord& rec)
{
    j.at("version").get_to(rec.version);
    j.at("kind").get_to(rec.kind);
    j.at("callsign").get_to(rec.callsign);
    j.at("gateway").get_to(rec.gateway);
    j.at("publisher").get_to(rec.publisher);
    j.at("instance_id").get_to(rec.instance_id);
    j.at("published_at").get_to(rec.published_at);
    j.at("expires_at").get_to(rec.expires_at);
    j.at("seq").get_to(rec.seq);

    // Optional field
    if (j.contains("last_heard")) {
        rec.last_heard = j.at("last_heard").get<LastHeard>();
    } else {
        rec.last_heard.reset();
    }
}