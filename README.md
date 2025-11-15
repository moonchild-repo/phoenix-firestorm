<img align="left" width="100" height="100" src="doc/firestorm_256.png" alt="Logo of Firestorm viewer"/>

**[Firestorm](https://www.firestormviewer.org) is a free client for 3D virtual worlds such as Second Life and various OpenSim worlds where users can create, connect and chat with others from around the world.**

This repository contains the official source code for the Firestorm viewer.

## Open Source

Firestorm is a third party viewer derived from the official [Second Life](https://github.com/secondlife/viewer) client. The client codebase has been open source since 2007 and is available under the LGPL license.

## Download

Pre-built versions of the viewer releases for Windows, Mac and Linux can be downloaded from the [official website](https://www.firestormviewer.org/choose-your-platform/).

## Build Instructions

Build instructions for each operating system can be found using the links below and in the official [wiki](https://wiki.firestormviewer.org).

- [Windows](doc/building_windows.md)
- [Mac](doc/building_macos.md)
- [Linux](doc/building_linux.md)

> [!NOTE]
> We do not provide support for compiling the viewer or issues resulting from using a self-compiled viewer. However, there is a self-compilers group within Second Life that can be joined to ask questions related to compiling the viewer: [Firestorm Self Compilers](https://tinyurl.com/firestorm-self-compilers)

## Contribute

Help make Firestorm better! You can get involved with improvements by filing bugs and suggesting enhancements via [JIRA](https://jira.firestormviewer.org) or [creating pull requests](CONTRIBUTING.md).

## Community respect

This section is guided by the [TPV Policy](https://secondlife.com/corporate/third-party-viewers) and the [Second Life Code of Conduct](https://github.com/secondlife/viewer?tab=coc-ov-file).

Firestorm code is made available during ongoing development, with the **master** branch representing the current nightly build. Developers and self-compilers are encouraged to work on their own forks and contribute back via pull requests, as detailed in the [contributing guide](CONTRIBUTING.md).

If you intend to use our code for your own viewer beyond personal use, please only use code from official release branches (for example, `Firestorm_7.1.13`), rather than from pre-release/preview or nightly builds.

## Inventory API

This version of Firestorm includes an experimental HTTP API for accessing and managing your local inventory. The API allows external applications to interact with your inventory, outfits, and avatar appearance.

### Features

- **Inventory Management**: List all inventory items, filter by type (clothing, attachments, bodyparts)
- **Outfit Management**: List outfits, export outfit data, wear/replace outfits
- **Item Operations**: Wear/remove individual items or multiple items at once
- **Attachment Management**: List currently attached objects
- **Outfit Export**: Export complete outfit data including items and attachments for external storage

### Configuration

The API is disabled by default. To enable it, set the following viewer settings:

- `InventoryAPIEnabled` = `true`
- `InventoryAPIPort` = `8080` (default)
- `InventoryAPIHost` = `127.0.0.1` (default, localhost only)

### API Documentation

For complete API documentation including all endpoints, request/response formats, and examples, see [API_SPECIFICATION.md](API_SPECIFICATION.md).

### Quick Start

1. Enable the API in viewer settings (see Configuration above)
2. Start the Firestorm viewer
3. Wait for inventory to fully load
4. Access the API at `http://127.0.0.1:8080/api`

**Example:**
```bash
# List all outfits
curl http://127.0.0.1:8080/api/inventory/outfits

# Wear an outfit
curl -X POST http://127.0.0.1:8080/api/inventory/outfit/wear \
  -H "Content-Type: application/json" \
  -d '{"outfit_id":"YOUR_OUTFIT_UUID","append":false}'
```

### Security Note

⚠️ **Important**: The API currently has no authentication and is intended for local use only. Do not expose the API to external networks without implementing proper authentication.

### Implementation Details

The API is implemented using:
- `LLIOHTTPServer` for HTTP server functionality
- `LLHTTPNode` for request routing
- LLSD (Linden Lab Scripting Data) format for data serialization
- Standard Firestorm inventory and appearance management systems

**Files Added:**
- `indra/newview/llinventoryapi.h` - API node header
- `indra/newview/llinventoryapi.cpp` - API implementation
- `API_SPECIFICATION.md` - Complete API documentation

**Files Modified:**
- `indra/newview/llappviewer.cpp` - API server initialization
- `indra/newview/app_settings/settings.xml` - API configuration settings
- `indra/newview/CMakeLists.txt` - Build configuration
