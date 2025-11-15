/**
 * @file llinventoryapi.cpp
 * @brief HTTP API implementation for Inventory and Outfit management
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

#include "llviewerprecompiledheaders.h"

#include "llinventoryapi.h"
#include "llrefcount.h"

#include "llagent.h"
#include "llagentwearables.h"
#include "llappearancemgr.h"
#include "llfoldertype.h"
#include "llhttpnode.h"
#include "llimagejpeg.h"
#include "llimagepng.h"
#include "llinventorymodel.h"
#include "llinventoryfunctions.h"
#include "llinventoryobserver.h"
#include "llnotificationsutil.h"
#include "llsnapshotlivepreview.h"
#include "llviewerfoldertype.h"
#include "llviewerinventory.h"
#include "llviewerobject.h"
#include "llviewerobjectlist.h"
#include "llviewerregion.h"
#include "llviewertexture.h"
#include "llviewerwindow.h"
#include "llvoavatarself.h"
#include "llsdserialize.h"

void LLInventoryAPINode::describe(Description& desc) const
{
    desc.shortInfo("Inventory and Outfit management API");
    desc.getAPI();
    desc.postAPI();
    desc.source(__FILE__, __LINE__);
}

bool LLInventoryAPINode::handles(const LLSD& remainder, LLSD& context) const
{
    // Accept requests with remainder (e.g., "items", "outfits", etc.)
    // This allows the node to handle paths like /api/inventory/items
    return remainder.size() > 0;
}

void LLInventoryAPINode::get(ResponsePtr response, const LLSD& context) const
{
    const LLSD& remainder = context[CONTEXT_REQUEST]["remainder"];
    const LLSD& path = context[CONTEXT_REQUEST][CONTEXT_PATH];
    
    if (remainder.size() == 0)
    {
        response->notFound("Invalid API endpoint");
        return;
    }

    std::string endpoint = remainder[0].asString();

    if (endpoint == "items")
    {
        handleGetItems(response, context);
    }
    else if (endpoint == "outfits")
    {
        handleGetOutfits(response, context);
    }
    else if (endpoint == "wearing")
    {
        handleGetWearing(response, context);
    }
    else if (endpoint == "attachments")
    {
        handleGetAttachments(response, context);
    }
    else if (endpoint == "outfit" && remainder.size() >= 3 && remainder[2].asString() == "export")
    {
        handleGetOutfitExport(response, context);
    }
    else if (endpoint == "avatar")
    {
        if (remainder.size() >= 2)
        {
            std::string sub_endpoint = remainder[1].asString();
            if (sub_endpoint == "screenshot")
            {
                handleGetScreenshot(response, context);
            }
            else if (sub_endpoint == "preview")
            {
                handleGetPreview(response, context);
            }
            else
            {
                response->notFound("Invalid avatar endpoint");
            }
        }
        else
        {
            response->notFound("Invalid avatar endpoint");
        }
    }
    else
    {
        response->notFound("Invalid endpoint");
    }
}

void LLInventoryAPINode::post(ResponsePtr response, const LLSD& context, const LLSD& input) const
{
    const LLSD& remainder = context[CONTEXT_REQUEST]["remainder"];
    
    if (remainder.size() == 0)
    {
        response->notFound("Invalid API endpoint");
        return;
    }

    std::string endpoint = remainder[0].asString();

    if (endpoint == "wear")
    {
        handlePostWear(response, input);
    }
    else if (endpoint == "remove")
    {
        handlePostRemove(response, input);
    }
    else if (endpoint == "wear" && remainder.size() >= 2 && remainder[1].asString() == "multiple")
    {
        handlePostWearMultiple(response, input);
    }
    else if (endpoint == "outfit")
    {
        if (remainder.size() >= 2)
        {
            std::string sub_endpoint = remainder[1].asString();
            if (sub_endpoint == "wear")
            {
                handlePostOutfitWear(response, input);
            }
            else if (sub_endpoint == "replace")
            {
                handlePostOutfitReplace(response, input);
            }
            else if (sub_endpoint == "create")
            {
                handlePostOutfitCreate(response, input);
            }
            else if (sub_endpoint == "save" && remainder.size() >= 2)
            {
                handlePostOutfitSave(response, context, input);
            }
            else if (sub_endpoint == "preview" && remainder.size() >= 2)
            {
                handlePostOutfitPreview(response, context, input);
            }
            else
            {
                response->notFound("Invalid outfit endpoint");
            }
        }
        else
        {
            response->notFound("Invalid outfit endpoint");
        }
    }
    else if (endpoint == "avatar")
    {
        if (remainder.size() >= 2 && remainder[1].asString() == "screenshot")
        {
            handlePostScreenshot(response, input);
        }
        else
        {
            response->notFound("Invalid avatar endpoint");
        }
    }
    else
    {
        response->notFound("Invalid endpoint");
    }
}

// GET /api/inventory/items
void LLInventoryAPINode::handleGetItems(ResponsePtr response, const LLSD& context) const
{
    if (!gInventory.isInventoryUsable())
    {
        response->status(503, "Inventory not ready");
        return;
    }

    LLSD result;
    LLSD items_array;

    // Get query parameters
    const LLSD& query = context[CONTEXT_REQUEST][CONTEXT_QUERY_STRING];
    std::string type_filter = query.has("type") ? query["type"].asString() : "";

    LLInventoryModel::item_array_t items;
    LLInventoryModel::cat_array_t cats;
    gInventory.collectDescendents(gInventory.getRootFolderID(), cats, items, LLInventoryModel::EXCLUDE_TRASH);

    for (LLInventoryModel::item_array_t::const_iterator it = items.begin(); it != items.end(); ++it)
    {
        const LLViewerInventoryItem* item = *it;
        if (!item) continue;

        // Apply type filter if specified
        if (!type_filter.empty())
        {
            LLAssetType::EType asset_type = item->getType();
            if (type_filter == "attachment" && asset_type != LLAssetType::AT_OBJECT)
                continue;
            if (type_filter == "clothing" && asset_type != LLAssetType::AT_CLOTHING)
                continue;
            if (type_filter == "bodypart" && asset_type != LLAssetType::AT_BODYPART)
                continue;
        }

        items_array.append(serializeInventoryItem(item));
    }

    result["items"] = items_array;
    response->result(result);
}

// GET /api/inventory/outfits
void LLInventoryAPINode::handleGetOutfits(ResponsePtr response, const LLSD& context) const
{
    if (!gInventory.isInventoryUsable())
    {
        response->status(503, "Inventory not ready");
        return;
    }

    LLSD result;
    LLSD outfits_array;

    LLInventoryModel::cat_array_t cats;
    LLInventoryModel::item_array_t items;
    gInventory.collectDescendents(gInventory.getRootFolderID(), cats, items, LLInventoryModel::EXCLUDE_TRASH);

    for (LLInventoryModel::cat_array_t::const_iterator it = cats.begin(); it != cats.end(); ++it)
    {
        const LLViewerInventoryCategory* cat = *it;
        if (!cat) continue;

        if (cat->getPreferredType() == LLFolderType::FT_OUTFIT)
        {
            outfits_array.append(serializeOutfit(cat));
        }
    }

    result["outfits"] = outfits_array;
    response->result(result);
}

// GET /api/inventory/wearing
void LLInventoryAPINode::handleGetWearing(ResponsePtr response, const LLSD& context) const
{
    if (!gInventory.isInventoryUsable() || !isAgentAvatarValid())
    {
        response->status(503, "Inventory or avatar not ready");
        return;
    }

    LLSD result;
    LLSD wearing_array;

    LLUUID cof_id = LLAppearanceMgr::getInstance()->getCOF();
    if (cof_id.isNull())
    {
        result["wearing"] = wearing_array;
        response->result(result);
        return;
    }

    LLInventoryModel::item_array_t items;
    LLInventoryModel::cat_array_t cats;
    gInventory.collectDescendents(cof_id, cats, items, LLInventoryModel::EXCLUDE_TRASH);

    for (LLInventoryModel::item_array_t::const_iterator it = items.begin(); it != items.end(); ++it)
    {
        const LLViewerInventoryItem* item = *it;
        if (!item) continue;

        wearing_array.append(serializeInventoryItem(item));
    }

    result["wearing"] = wearing_array;
    response->result(result);
}

// GET /api/inventory/attachments
void LLInventoryAPINode::handleGetAttachments(ResponsePtr response, const LLSD& context) const
{
    if (!isAgentAvatarValid())
    {
        response->status(503, "Avatar not ready");
        return;
    }

    LLSD result;
    LLSD attachments_array;

    if (gAgentAvatarp)
    {
        // Get attachments from COF (Current Outfit Folder) instead of directly from avatar
        // This is more reliable and matches the inventory structure
        LLUUID cof_id = LLAppearanceMgr::getInstance()->getCOF();
        if (!cof_id.isNull())
        {
            LLInventoryModel::item_array_t items;
            LLInventoryModel::cat_array_t cats;
            gInventory.collectDescendents(cof_id, cats, items, LLInventoryModel::EXCLUDE_TRASH);
            
            for (LLInventoryModel::item_array_t::const_iterator it = items.begin(); it != items.end(); ++it)
            {
                const LLViewerInventoryItem* item = *it;
                if (item && item->getType() == LLAssetType::AT_OBJECT)
                {
                    attachments_array.append(serializeInventoryItem(item));
                }
            }
        }
    }

    result["attachments"] = attachments_array;
    response->result(result);
}

// GET /api/inventory/outfit/{outfit_id}/export
void LLInventoryAPINode::handleGetOutfitExport(ResponsePtr response, const LLSD& context) const
{
    if (!gInventory.isInventoryUsable())
    {
        response->status(503, "Inventory not ready");
        return;
    }

    LLUUID outfit_id = getOutfitIdFromPath(context);
    if (outfit_id.isNull())
    {
        response->notFound("Invalid outfit ID");
        return;
    }

    const LLViewerInventoryCategory* outfit = gInventory.getCategory(outfit_id);
    if (!outfit)
    {
        response->notFound("Outfit not found");
        return;
    }

    LLSD result;
    result["outfit"] = serializeOutfit(outfit);

    // Get all items in the outfit
    LLInventoryModel::item_array_t items;
    LLInventoryModel::cat_array_t cats;
    gInventory.collectDescendents(outfit_id, cats, items, LLInventoryModel::EXCLUDE_TRASH);

    LLSD items_array;
    LLSD attachments_array;

    for (LLInventoryModel::item_array_t::const_iterator it = items.begin(); it != items.end(); ++it)
    {
        const LLViewerInventoryItem* item = *it;
        if (!item) continue;

        LLSD item_data = serializeInventoryItem(item);
        if (item->getType() == LLAssetType::AT_OBJECT)
        {
            attachments_array.append(item_data);
        }
        else
        {
            items_array.append(item_data);
        }
    }

    result["outfit"]["items"] = items_array;
    result["outfit"]["attachments"] = attachments_array;

    response->result(result);
}

// GET /api/avatar/screenshot
void LLInventoryAPINode::handleGetScreenshot(ResponsePtr response, const LLSD& context) const
{
    // This is a simplified version - full implementation would use LLSnapshotLivePreview
    response->status(501, "Screenshot via GET not yet implemented - use POST");
}

// GET /api/avatar/preview
void LLInventoryAPINode::handleGetPreview(ResponsePtr response, const LLSD& context) const
{
    // This is a simplified version - full implementation would use LLSnapshotLivePreview
    response->status(501, "Preview via GET not yet implemented - use POST");
}

// POST /api/inventory/wear
void LLInventoryAPINode::handlePostWear(ResponsePtr response, const LLSD& input) const
{
    if (!gInventory.isInventoryUsable() || !isAgentAvatarValid())
    {
        response->status(503, "Inventory or avatar not ready");
        return;
    }

    if (!input.has("item_id"))
    {
        response->status(400, "Missing item_id parameter");
        return;
    }

    LLUUID item_id(input["item_id"].asString());
    if (item_id.isNull())
    {
        response->status(400, "Invalid item_id");
        return;
    }

    bool replace = input.has("replace") ? input["replace"].asBoolean() : false;

    LLAppearanceMgr::instance().wearItemOnAvatar(item_id, true, replace);

    LLSD result;
    result["success"] = true;
    response->result(result);
}

// POST /api/inventory/remove
void LLInventoryAPINode::handlePostRemove(ResponsePtr response, const LLSD& input) const
{
    if (!gInventory.isInventoryUsable() || !isAgentAvatarValid())
    {
        response->status(503, "Inventory or avatar not ready");
        return;
    }

    if (!input.has("item_id"))
    {
        response->status(400, "Missing item_id parameter");
        return;
    }

    LLUUID item_id(input["item_id"].asString());
    if (item_id.isNull())
    {
        response->status(400, "Invalid item_id");
        return;
    }

    LLAppearanceMgr::instance().removeItemFromAvatar(item_id);

    LLSD result;
    result["success"] = true;
    response->result(result);
}

// POST /api/inventory/wear/multiple
void LLInventoryAPINode::handlePostWearMultiple(ResponsePtr response, const LLSD& input) const
{
    if (!gInventory.isInventoryUsable() || !isAgentAvatarValid())
    {
        response->status(503, "Inventory or avatar not ready");
        return;
    }

    if (!input.has("item_ids") || !input["item_ids"].isArray())
    {
        response->status(400, "Missing or invalid item_ids array");
        return;
    }

    bool replace = input.has("replace") ? input["replace"].asBoolean() : false;
    uuid_vec_t item_ids;

    for (LLSD::array_const_iterator it = input["item_ids"].beginArray();
         it != input["item_ids"].endArray();
         ++it)
    {
        LLUUID item_id(it->asString());
        if (!item_id.isNull())
        {
            item_ids.push_back(item_id);
        }
    }

    if (item_ids.empty())
    {
        response->status(400, "No valid item IDs provided");
        return;
    }

    LLAppearanceMgr::instance().wearItemsOnAvatar(item_ids, true, replace);

    LLSD result;
    result["success"] = true;
    result["worn_count"] = (S32)item_ids.size();
    response->result(result);
}

// POST /api/inventory/outfit/wear
void LLInventoryAPINode::handlePostOutfitWear(ResponsePtr response, const LLSD& input) const
{
    if (!gInventory.isInventoryUsable() || !isAgentAvatarValid())
    {
        response->status(503, "Inventory or avatar not ready");
        return;
    }

    if (!input.has("outfit_id"))
    {
        response->status(400, "Missing outfit_id parameter");
        return;
    }

    LLUUID outfit_id(input["outfit_id"].asString());
    if (outfit_id.isNull())
    {
        response->status(400, "Invalid outfit_id");
        return;
    }

    bool append = input.has("append") ? input["append"].asBoolean() : false;

    // wearOutfit expects LLSD with folder_id
    LLSD query_map;
    query_map["folder_id"] = outfit_id.asString();
    LLAppearanceMgr::instance().wearOutfit(query_map, append);

    LLSD result;
    result["success"] = true;
    response->result(result);
}

// POST /api/inventory/outfit/replace
void LLInventoryAPINode::handlePostOutfitReplace(ResponsePtr response, const LLSD& input) const
{
    if (!gInventory.isInventoryUsable() || !isAgentAvatarValid())
    {
        response->status(503, "Inventory or avatar not ready");
        return;
    }

    if (!input.has("outfit_id"))
    {
        response->status(400, "Missing outfit_id parameter");
        return;
    }

    LLUUID outfit_id(input["outfit_id"].asString());
    if (outfit_id.isNull())
    {
        response->status(400, "Invalid outfit_id");
        return;
    }

    LLAppearanceMgr::instance().replaceCurrentOutfit(outfit_id);

    LLSD result;
    result["success"] = true;
    response->result(result);
}

// POST /api/inventory/outfit/create
void LLInventoryAPINode::handlePostOutfitCreate(ResponsePtr response, const LLSD& input) const
{
    if (!gInventory.isInventoryUsable() || !isAgentAvatarValid())
    {
        response->status(503, "Inventory or avatar not ready");
        return;
    }

    if (!input.has("name"))
    {
        response->status(400, "Missing name parameter");
        return;
    }

    std::string outfit_name = input["name"].asString();
    if (outfit_name.empty())
    {
        response->status(400, "Outfit name cannot be empty");
        return;
    }

    uuid_vec_t item_ids;
    uuid_vec_t attachment_ids;

    if (input.has("item_ids") && input["item_ids"].isArray())
    {
        for (LLSD::array_const_iterator it = input["item_ids"].beginArray();
             it != input["item_ids"].endArray();
             ++it)
        {
            LLUUID item_id(it->asString());
            if (!item_id.isNull())
            {
                item_ids.push_back(item_id);
            }
        }
    }

    if (input.has("attachment_ids") && input["attachment_ids"].isArray())
    {
        for (LLSD::array_const_iterator it = input["attachment_ids"].beginArray();
             it != input["attachment_ids"].endArray();
             ++it)
        {
            LLUUID attachment_id(it->asString());
            if (!attachment_id.isNull())
            {
                attachment_ids.push_back(attachment_id);
            }
        }
    }

    // Combine item_ids and attachment_ids
    item_ids.insert(item_ids.end(), attachment_ids.begin(), attachment_ids.end());

    if (item_ids.empty())
    {
        response->status(400, "No items or attachments provided");
        return;
    }

    // Create outfit folder
    LLUUID my_outfits_id = gInventory.findCategoryUUIDForType(LLFolderType::FT_MY_OUTFITS);
    if (my_outfits_id.isNull())
    {
        response->status(500, "My Outfits folder not found");
        return;
    }

    // createNewCategory uses a callback - we'll use a shared pointer to capture the result
    struct OutfitCreateData : public LLRefCount
    {
        LLUUID outfit_id;
        bool created;
        OutfitCreateData() : created(false) {}
        void setOutfitId(const LLUUID& id) { outfit_id = id; created = true; }
    };
    LLPointer<OutfitCreateData> create_data = new OutfitCreateData;
    
    gInventory.createNewCategory(
        my_outfits_id,
        LLFolderType::FT_OUTFIT,
        outfit_name,
        [create_data](const LLUUID& new_cat_id) mutable
        {
            create_data->setOutfitId(new_cat_id);
        }
    );
    
    // Note: createNewCategory is asynchronous, so we can't immediately use the outfit_id
    // For now, we'll create the outfit differently - by using the COF directly
    // This is a limitation of the synchronous API design
    // Note: createNewCategory is asynchronous, so outfit_id will be null initially
    // The outfit will be created asynchronously, but we can't return it immediately
    // For now, return an error indicating this needs async handling
    response->status(501, "Outfit creation is asynchronous - this endpoint needs to be refactored to support async operations");
    return;
    
    LLUUID outfit_id = create_data->outfit_id;

    // Add items to COF (Current Outfit Folder) to create the outfit
    LLUUID cof_id = LLAppearanceMgr::getInstance()->getCOF();
    if (!cof_id.isNull())
    {
        for (uuid_vec_t::const_iterator it = item_ids.begin(); it != item_ids.end(); ++it)
        {
            LLAppearanceMgr::instance().addCOFItemLink(*it);
        }
    }

    // Copy COF contents to outfit folder
    LLInventoryModel::item_array_t cof_items;
    LLInventoryModel::cat_array_t cof_cats;
    gInventory.collectDescendents(cof_id, cof_cats, cof_items, LLInventoryModel::EXCLUDE_TRASH);

    for (LLInventoryModel::item_array_t::const_iterator it = cof_items.begin(); it != cof_items.end(); ++it)
    {
        const LLViewerInventoryItem* item = *it;
        if (item)
        {
            link_inventory_object(outfit_id, LLConstPointer<LLInventoryObject>(item), NULL);
        }
    }

    LLSD result;
    result["success"] = true;
    result["outfit_id"] = outfit_id.asString();
    result["outfit_uuid"] = outfit_id.asString();
    response->result(result);
}

// POST /api/inventory/outfit/{outfit_id}/save
void LLInventoryAPINode::handlePostOutfitSave(ResponsePtr response, const LLSD& context, const LLSD& input) const
{
    if (!gInventory.isInventoryUsable())
    {
        response->status(503, "Inventory not ready");
        return;
    }

    LLUUID outfit_id = getOutfitIdFromPath(context);
    if (outfit_id.isNull())
    {
        response->notFound("Invalid outfit ID");
        return;
    }

    // Export outfit data (same as GET export)
    const LLViewerInventoryCategory* outfit = gInventory.getCategory(outfit_id);
    if (!outfit)
    {
        response->notFound("Outfit not found");
        return;
    }

    LLSD result;
    result["outfit"] = serializeOutfit(outfit);

    LLInventoryModel::item_array_t items;
    LLInventoryModel::cat_array_t cats;
    gInventory.collectDescendents(outfit_id, cats, items, LLInventoryModel::EXCLUDE_TRASH);

    LLSD items_array;
    LLSD attachments_array;

    for (LLInventoryModel::item_array_t::const_iterator it = items.begin(); it != items.end(); ++it)
    {
        const LLViewerInventoryItem* item = *it;
        if (!item) continue;

        LLSD item_data = serializeInventoryItem(item);
        if (item->getType() == LLAssetType::AT_OBJECT)
        {
            attachments_array.append(item_data);
        }
        else
        {
            items_array.append(item_data);
        }
    }

    result["outfit"]["items"] = items_array;
    result["outfit"]["attachments"] = attachments_array;
    result["success"] = true;

    response->result(result);
}

// POST /api/inventory/outfit/{outfit_id}/preview
void LLInventoryAPINode::handlePostOutfitPreview(ResponsePtr response, const LLSD& context, const LLSD& input) const
{
    // This would require wearing the outfit first, then taking a screenshot
    // For now, return not implemented
    response->status(501, "Outfit preview not yet fully implemented");
}

// POST /api/avatar/screenshot
void LLInventoryAPINode::handlePostScreenshot(ResponsePtr response, const LLSD& input) const
{
    // This is a placeholder - full implementation would use LLSnapshotLivePreview
    // and LLImageJPEG/LLImagePNG for encoding
    response->status(501, "Screenshot functionality not yet fully implemented");
}

// Utility methods
LLSD LLInventoryAPINode::serializeInventoryItem(const LLViewerInventoryItem* item) const
{
    LLSD result;
    if (!item) return result;

    // Use getLinkedUUID() for stable identifier - this points to the original item
    // even if this is a link in the COF
    result["uuid"] = item->getLinkedUUID().asString();
    
    // Also include the link UUID if this is a link (for reference)
    if (item->getIsLinkType())
    {
        result["link_uuid"] = item->getUUID().asString();
    }
    
    result["name"] = item->getName();
    result["type"] = LLAssetType::lookup(item->getType());
    result["asset_type"] = LLAssetType::lookup(item->getActualType());
    result["description"] = item->getDescription();
    result["parent_uuid"] = item->getParentUUID().asString();
    result["creation_date"] = (S32)item->getCreationDate();

    if (item->isWearableType())
    {
        LLWearableType::EType wearable_type = item->getWearableType();
        result["wearable_type"] = LLWearableType::instance().getTypeName(wearable_type);
    }

    return result;
}

LLSD LLInventoryAPINode::serializeOutfit(const LLViewerInventoryCategory* outfit) const
{
    LLSD result;
    if (!outfit) return result;

    result["uuid"] = outfit->getUUID().asString();
    result["name"] = outfit->getName();
    result["parent_uuid"] = outfit->getParentUUID().asString();

    // Count items in outfit
    LLInventoryModel::item_array_t items;
    LLInventoryModel::cat_array_t cats;
    gInventory.collectDescendents(outfit->getUUID(), cats, items, LLInventoryModel::EXCLUDE_TRASH);
    result["item_count"] = (S32)items.size();

    return result;
}

LLSD LLInventoryAPINode::serializeAttachment(const LLViewerObject* attachment) const
{
    // This method is no longer used - attachments are now serialized as inventory items
    // Keeping for API compatibility but returning empty result
    LLSD result;
    return result;
    
    // Old implementation (not used):
    /*
    if (!attachment) return result;

    result["uuid"] = attachment->getID().asString();
    result["name"] = attachment->getName();
    result["attachment_point"] = (S32)attachment->getAttachmentPoint();

    LLUUID item_id = attachment->getAttachmentItemID();
    if (!item_id.isNull())
    {
        result["item_id"] = item_id.asString();
        const LLViewerInventoryItem* item = gInventory.getItem(item_id);
        if (item)
        {
            result["item_name"] = item->getName();
        }
    }

    return result;
    */
}

std::string LLInventoryAPINode::getPathComponent(const LLSD& context, S32 index) const
{
    const LLSD& remainder = context[CONTEXT_REQUEST]["remainder"];
    if (remainder.isArray() && index < (S32)remainder.size())
    {
        return remainder[index].asString();
    }
    return "";
}

LLUUID LLInventoryAPINode::getOutfitIdFromPath(const LLSD& context) const
{
    // Path format: /api/inventory/outfit/{outfit_id}/export or /save
    // remainder[0] = "outfit", remainder[1] = outfit_id, remainder[2] = "export" or "save"
    std::string outfit_id_str = getPathComponent(context, 1);
    if (!outfit_id_str.empty())
    {
        return LLUUID(outfit_id_str);
    }
    return LLUUID::null;
}

