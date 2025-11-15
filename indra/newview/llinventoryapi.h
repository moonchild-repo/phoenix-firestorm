/**
 * @file llinventoryapi.h
 * @brief HTTP API for Inventory and Outfit management
 *
 * $LicenseInfo:firstyear=2024&license=viewerlgpl$
 * Phoenix Firestorm Viewer Source Code
 * Copyright (C) 2024, Phoenix Firestorm Project
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation;
 * version 2.1 of the License only.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Phoenix Firestorm Project
 * $/LicenseInfo$
 */

#ifndef LL_LLINVENTORYAPI_H
#define LL_LLINVENTORYAPI_H

#include "llhttpnode.h"
#include "lluuid.h"
#include "llsd.h"

class LLViewerInventoryItem;
class LLViewerInventoryCategory;
class LLViewerObject;

class LLInventoryAPINode : public LLHTTPNode
{
public:
    virtual void describe(Description& desc) const;

    // Override handles to accept requests with remainder
    virtual bool handles(const LLSD& remainder, LLSD& context) const override;

    // GET endpoints
    virtual void get(ResponsePtr response, const LLSD& context) const;

    // POST endpoints
    virtual void post(ResponsePtr response, const LLSD& context, const LLSD& input) const;

private:
    // Helper methods for GET requests
    void handleGetItems(ResponsePtr response, const LLSD& context) const;
    void handleGetOutfits(ResponsePtr response, const LLSD& context) const;
    void handleGetWearing(ResponsePtr response, const LLSD& context) const;
    void handleGetAttachments(ResponsePtr response, const LLSD& context) const;
    void handleGetOutfitExport(ResponsePtr response, const LLSD& context) const;
    void handleGetScreenshot(ResponsePtr response, const LLSD& context) const;
    void handleGetPreview(ResponsePtr response, const LLSD& context) const;

    // Helper methods for POST requests
    void handlePostWear(ResponsePtr response, const LLSD& input) const;
    void handlePostRemove(ResponsePtr response, const LLSD& input) const;
    void handlePostWearMultiple(ResponsePtr response, const LLSD& input) const;
    void handlePostOutfitWear(ResponsePtr response, const LLSD& input) const;
    void handlePostOutfitReplace(ResponsePtr response, const LLSD& input) const;
    void handlePostOutfitCreate(ResponsePtr response, const LLSD& input) const;
    void handlePostOutfitSave(ResponsePtr response, const LLSD& context, const LLSD& input) const;
    void handlePostOutfitPreview(ResponsePtr response, const LLSD& context, const LLSD& input) const;
    void handlePostScreenshot(ResponsePtr response, const LLSD& input) const;

    // Utility methods
    LLSD serializeInventoryItem(const LLViewerInventoryItem* item) const;
    LLSD serializeOutfit(const LLViewerInventoryCategory* outfit) const;
    LLSD serializeAttachment(const LLViewerObject* attachment) const;
    std::string getPathComponent(const LLSD& context, S32 index) const;
    LLUUID getOutfitIdFromPath(const LLSD& context) const;
};

#endif // LL_LLINVENTORYAPI_H

