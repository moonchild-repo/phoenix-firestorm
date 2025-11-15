# Firestorm Inventory API - Spezifikation

## Base Information

**Base URL:** `http://127.0.0.1:8080/api`  
**Content-Type:** `application/llsd+xml` (Standard) oder `application/json` (wenn implementiert)  
**Format:** LLSD (Linden Lab Scripting Data) - XML-Format, JSON-ähnlich

## Konfiguration

Die API muss im Viewer aktiviert sein:
- **Setting:** `InventoryAPIEnabled` = `true`
- **Port:** `InventoryAPIPort` = `8080` (Standard)
- **Host:** `InventoryAPIHost` = `127.0.0.1` (Standard, nur localhost)

---

## GET Endpunkte

### 1. GET `/api/inventory/items`

Listet alle Inventory-Items auf.

**Query Parameter:**
- `type` (optional): Filter nach Typ
  - `attachment` - Nur Attachments (AT_OBJECT)
  - `clothing` - Nur Kleidung (AT_CLOTHING)
  - `bodypart` - Nur Bodyparts (AT_BODYPART)

**Response:**
```xml
<llsd>
  <map>
    <key>items</key>
    <array>
      <map>
        <key>uuid</key>
        <string>550e8400-e29b-41d4-a716-446655440000</string>
        <key>name</key>
        <string>Item Name</string>
        <key>type</key>
        <string>clothing</string>
        <key>asset_type</key>
        <string>clothing</string>
        <key>description</key>
        <string>Item description</string>
        <key>parent_uuid</key>
        <string>parent-uuid-here</string>
        <key>creation_date</key>
        <integer>1234567890</integer>
        <key>wearable_type</key>
        <string>shirt</string>  <!-- Nur wenn isWearableType() == true -->
      </map>
    </array>
  </map>
</llsd>
```

**Beispiel:**
```bash
curl http://127.0.0.1:8080/api/inventory/items
curl http://127.0.0.1:8080/api/inventory/items?type=attachment
```

---

### 2. GET `/api/inventory/outfits`

Listet alle Outfits auf.

**Response:**
```xml
<llsd>
  <map>
    <key>outfits</key>
    <array>
      <map>
        <key>uuid</key>
        <string>outfit-uuid-here</string>
        <key>name</key>
        <string>Outfit Name</string>
        <key>parent_uuid</key>
        <string>parent-uuid-here</string>
        <key>item_count</key>
        <integer>5</integer>
      </map>
    </array>
  </map>
</llsd>
```

**Beispiel:**
```bash
curl http://127.0.0.1:8080/api/inventory/outfits
```

---

### 3. GET `/api/inventory/wearing`

Listet aktuell getragene Items auf (aus dem COF - Current Outfit Folder).

**Response:**
```xml
<llsd>
  <map>
    <key>wearing</key>
    <array>
      <!-- Array von Item-Objekten (gleiche Struktur wie /items) -->
    </array>
  </map>
</llsd>
```

**Beispiel:**
```bash
curl http://127.0.0.1:8080/api/inventory/wearing
```

---

### 4. GET `/api/inventory/attachments`

Listet aktuell angehängte Objekte auf.

**Response:**
```xml
<llsd>
  <map>
    <key>attachments</key>
    <array>
      <!-- Array von Item-Objekten (nur AT_OBJECT Typ) -->
    </array>
  </map>
</llsd>
```

**Beispiel:**
```bash
curl http://127.0.0.1:8080/api/inventory/attachments
```

---

### 5. GET `/api/inventory/outfit/{outfit_id}/export`

Exportiert ein Outfit mit allen Items und Attachments.

**Path Parameter:**
- `outfit_id` - UUID des Outfits

**Response:**
```xml
<llsd>
  <map>
    <key>outfit</key>
    <map>
      <key>uuid</key>
      <string>outfit-uuid</string>
      <key>name</key>
      <string>Outfit Name</string>
      <key>parent_uuid</key>
      <string>parent-uuid</string>
      <key>item_count</key>
      <integer>5</integer>
      <key>items</key>
      <array>
        <!-- Array von Item-Objekten (Kleidung, etc.) -->
      </array>
      <key>attachments</key>
      <array>
        <!-- Array von Item-Objekten (AT_OBJECT) -->
      </array>
    </map>
  </map>
</llsd>
```

**Beispiel:**
```bash
curl http://127.0.0.1:8080/api/inventory/outfit/550e8400-e29b-41d4-a716-446655440000/export
```

---

### 6. GET `/api/avatar/screenshot`

**Status:** `501 Not Implemented` - Verwendet POST stattdessen

---

### 7. GET `/api/avatar/preview`

**Status:** `501 Not Implemented` - Verwendet POST stattdessen

---

## POST Endpunkte

### 1. POST `/api/inventory/wear`

Trägt ein einzelnes Item.

**Request Body:**
```json
{
  "item_id": "550e8400-e29b-41d4-a716-446655440000",
  "replace": false  // Optional, default: false
}
```

**Response:**
```xml
<llsd>
  <map>
    <key>success</key>
    <boolean>true</boolean>
  </map>
</llsd>
```

**Beispiel:**
```bash
curl -X POST http://127.0.0.1:8080/api/inventory/wear \
  -H "Content-Type: application/json" \
  -d '{"item_id":"550e8400-e29b-41d4-a716-446655440000","replace":false}'
```

---

### 2. POST `/api/inventory/remove`

Entfernt ein Item vom Avatar.

**Request Body:**
```json
{
  "item_id": "550e8400-e29b-41d4-a716-446655440000"
}
```

**Response:**
```xml
<llsd>
  <map>
    <key>success</key>
    <boolean>true</boolean>
  </map>
</llsd>
```

**Beispiel:**
```bash
curl -X POST http://127.0.0.1:8080/api/inventory/remove \
  -H "Content-Type: application/json" \
  -d '{"item_id":"550e8400-e29b-41d4-a716-446655440000"}'
```

---

### 3. POST `/api/inventory/wear/multiple`

Trägt mehrere Items gleichzeitig.

**Request Body:**
```json
{
  "item_ids": [
    "550e8400-e29b-41d4-a716-446655440000",
    "660e8400-e29b-41d4-a716-446655440001"
  ],
  "replace": false  // Optional, default: false
}
```

**Response:**
```xml
<llsd>
  <map>
    <key>success</key>
    <boolean>true</boolean>
    <key>worn_count</key>
    <integer>2</integer>
  </map>
</llsd>
```

**Beispiel:**
```bash
curl -X POST http://127.0.0.1:8080/api/inventory/wear/multiple \
  -H "Content-Type: application/json" \
  -d '{"item_ids":["uuid1","uuid2"],"replace":false}'
```

---

### 4. POST `/api/inventory/outfit/wear`

Trägt ein Outfit.

**Request Body:**
```json
{
  "outfit_id": "550e8400-e29b-41d4-a716-446655440000",
  "append": false  // Optional, default: false. true = hinzufügen, false = ersetzen
}
```

**Response:**
```xml
<llsd>
  <map>
    <key>success</key>
    <boolean>true</boolean>
  </map>
</llsd>
```

**Beispiel:**
```bash
curl -X POST http://127.0.0.1:8080/api/inventory/outfit/wear \
  -H "Content-Type: application/json" \
  -d '{"outfit_id":"550e8400-e29b-41d4-a716-446655440000","append":false}'
```

---

### 5. POST `/api/inventory/outfit/replace`

Ersetzt das aktuelle Outfit komplett.

**Request Body:**
```json
{
  "outfit_id": "550e8400-e29b-41d4-a716-446655440000"
}
```

**Response:**
```xml
<llsd>
  <map>
    <key>success</key>
    <boolean>true</boolean>
  </map>
</llsd>
```

**Beispiel:**
```bash
curl -X POST http://127.0.0.1:8080/api/inventory/outfit/replace \
  -H "Content-Type: application/json" \
  -d '{"outfit_id":"550e8400-e29b-41d4-a716-446655440000"}'
```

---

### 6. POST `/api/inventory/outfit/create`

**Status:** `501 Not Implemented` - Asynchrones Problem, muss refactored werden

**Geplante Request Body:**
```json
{
  "name": "My New Outfit",
  "item_ids": ["uuid1", "uuid2"],  // Optional
  "attachment_ids": ["uuid3", "uuid4"]  // Optional
}
```

---

### 7. POST `/api/inventory/outfit/{outfit_id}/save`

Speichert/exportiert ein Outfit (gleiche Antwort wie GET export).

**Path Parameter:**
- `outfit_id` - UUID des Outfits

**Request Body:** (optional, leer erlaubt)

**Response:** (gleiche Struktur wie GET `/api/inventory/outfit/{outfit_id}/export`)

**Beispiel:**
```bash
curl -X POST http://127.0.0.1:8080/api/inventory/outfit/550e8400-e29b-41d4-a716-446655440000/save \
  -H "Content-Type: application/json" \
  -d '{}'
```

---

### 8. POST `/api/inventory/outfit/{outfit_id}/preview`

**Status:** `501 Not Implemented`

---

### 9. POST `/api/avatar/screenshot`

**Status:** `501 Not Implemented`

---

## Datenstrukturen

### Item Object

```json
{
  "uuid": "string (UUID)",
  "name": "string",
  "type": "string (asset type name)",
  "asset_type": "string (actual asset type)",
  "description": "string",
  "parent_uuid": "string (UUID)",
  "creation_date": "integer (Unix timestamp)",
  "wearable_type": "string (optional, nur bei Wearables)"
}
```

**Asset Types (type/asset_type):**
- `clothing` - Kleidung
- `bodypart` - Bodypart
- `object` - Attachment/Objekt
- `texture` - Texture
- `sound` - Sound
- `animation` - Animation
- etc.

**Wearable Types (wearable_type, wenn vorhanden):**
- `shirt`, `pants`, `shoes`, `socks`, `jacket`, `gloves`, `undershirt`, `underpants`, `skirt`, `alpha`, `tattoo`, `physics`, `universal`

---

### Outfit Object

```json
{
  "uuid": "string (UUID)",
  "name": "string",
  "parent_uuid": "string (UUID)",
  "item_count": "integer"
}
```

---

## HTTP Status Codes

| Code | Bedeutung |
|------|-----------|
| `200` | OK - Erfolgreich |
| `400` | Bad Request - Fehlende oder ungültige Parameter |
| `404` | Not Found - Resource nicht gefunden |
| `501` | Not Implemented - Endpunkt noch nicht implementiert |
| `503` | Service Unavailable - Inventory/Avatar nicht bereit |

---

## Fehlerbehandlung

**Standard-Fehlerantwort:**
```xml
<llsd>
  <map>
    <key>error</key>
    <string>Error message</string>
  </map>
</llsd>
```

Oder als HTTP Status mit Message im Body.

---

## Wichtige Hinweise

1. **Viewer muss laufen:** Die API ist nur verfügbar, wenn der Firestorm Viewer gestartet ist.
2. **Inventory muss geladen sein:** Warten Sie, bis das Inventory vollständig geladen ist, bevor Sie API-Calls machen.
3. **UUID-Format:** Alle UUIDs sind im Standard-UUID-Format (mit Bindestrichen).
4. **Asynchrone Operationen:** Einige Operationen (wie `wearOutfit`) sind asynchron - die API gibt sofort `success: true` zurück, aber das Tragen kann einige Sekunden dauern.
5. **COF (Current Outfit Folder):** Das "wearing" Endpoint basiert auf dem COF, nicht auf direktem Avatar-Zustand.

---

## Beispiel-Workflow

### 1. Alle Outfits abrufen
```bash
curl http://127.0.0.1:8080/api/inventory/outfits
```

### 2. Ein Outfit tragen
```bash
curl -X POST http://127.0.0.1:8080/api/inventory/outfit/wear \
  -H "Content-Type: application/json" \
  -d '{"outfit_id":"OUTFIT_UUID_HERE","append":false}'
```

### 3. Outfit exportieren
```bash
curl http://127.0.0.1:8080/api/inventory/outfit/OUTFIT_UUID_HERE/export
```

### 4. Aktuell getragene Items abrufen
```bash
curl http://127.0.0.1:8080/api/inventory/wearing
```

---

## Client-Implementierungshinweise

### LLSD Parsing

Die API gibt standardmäßig LLSD-XML zurück. Für Client-Implementierungen:

1. **XML-Parser verwenden:** LLSD-XML kann mit jedem Standard-XML-Parser gelesen werden
2. **JSON-Alternative:** Falls implementiert, kann `Accept: application/json` Header verwendet werden
3. **Struktur:** LLSD ist JSON-ähnlich - Maps werden zu Objekten, Arrays bleiben Arrays

### Beispiel: Python Client

```python
import requests
import xml.etree.ElementTree as ET

def parse_llsd_xml(xml_string):
    """Einfacher LLSD-XML Parser (vereinfacht)"""
    root = ET.fromstring(xml_string)
    # ... LLSD-spezifisches Parsing ...
    return data

# API Call
response = requests.get('http://127.0.0.1:8080/api/inventory/items')
data = parse_llsd_xml(response.text)
```

### Beispiel: JavaScript/Node.js Client

```javascript
const axios = require('axios');
const xml2js = require('xml2js');

async function getItems() {
  const response = await axios.get('http://127.0.0.1:8080/api/inventory/items');
  const parser = new xml2js.Parser();
  const result = await parser.parseStringPromise(response.data);
  return result;
}
```

---

**Version:** 1.0  
**Datum:** 2025-01-15  
**Firestorm Viewer Version:** Entwicklungsversion mit Inventory API

