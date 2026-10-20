#include "ble_bbp.h"

#include <Arduino.h>
#include <NimBLEDevice.h>

#include "config.h"

namespace spinlite {
namespace ble {
namespace {

constexpr const char *SERVICE_UUID = "55c40000-f8eb-11ec-b939-0242ac120002";
constexpr const char *CHR_F001_UUID = "55c4f001-f8eb-11ec-b939-0242ac120002";
constexpr const char *CHR_F002_UUID = "55c4f002-f8eb-11ec-b939-0242ac120002";

// Azioni messe in coda dalle callback ed eseguite da loop().
enum Action : uint8_t { ACT_STATUS = 1, ACT_DUMP = 2, ACT_CLEAR = 3 };
constexpr UBaseType_t QUEUE_DEPTH = 8;

Provider provider;
NimBLECharacteristic *chrNotify = nullptr; // f002
QueueHandle_t queue = nullptr;

// Stato scritto dalle callback (task NimBLE) e letto da loop() (task principale).
volatile bool gConnected = false;
volatile bool gSubscribed = false;
volatile bool gAbandon = false; // connessione persa: scarta dump e comandi
volatile uint32_t gCommands = 0;
volatile bool gQuiet = false;

// Invio in corso: le pagine del dump sono uno snapshot preso una volta sola.
uint8_t dumpPages[bbp::DUMP_PAGES][bbp::PACKET];
uint8_t dumpNext = bbp::DUMP_PAGES; // == DUMP_PAGES: nessun dump in corso
uint32_t lastSendMs = 0;
bool sentOnce = false;

void post(uint8_t act) {
  if (queue) xQueueSend(queue, &act, 0); // coda piena: il comando si perde, l'app ritenta
}

void handleCommand(NimBLECharacteristic *c) {
  const NimBLEAttValue &v = c->getValue();
  if (v.size() < 1) return;
  switch (v.data()[0]) {
    case bbp::CMD_STATUS:
      gCommands = gCommands + 1;
      post(ACT_STATUS);
      break;
    case bbp::CMD_DUMP:
      gCommands = gCommands + 1;
      post(ACT_DUMP);
      break;
    case bbp::CMD_CLEAR:
      gCommands = gCommands + 1;
      post(ACT_CLEAR);
      break;
    default:
      break; // comando sconosciuto: ignorato
  }
}

class ServerCb : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer *, NimBLEConnInfo &) override {
    gConnected = true;
  }
  void onDisconnect(NimBLEServer *server, NimBLEConnInfo &, int) override {
    gConnected = false;
    gSubscribed = false;
    gAbandon = true;
    // L'advertising riparte da solo (advertiseOnDisconnect), questa e' una cintura.
    server->startAdvertising();
  }
};

class ChrCb : public NimBLECharacteristicCallbacks {
  void onWrite(NimBLECharacteristic *c, NimBLEConnInfo &) override {
    handleCommand(c);
  }
  void onSubscribe(NimBLECharacteristic *, NimBLEConnInfo &, uint16_t subValue) override {
    bool on = (subValue & 1) != 0; // bit 0 = notifiche
    gSubscribed = on;
    if (on) post(ACT_STATUS); // autonotify: A0 appena l'app si iscrive
  }
};

// Imposta il valore (cosi' READ restituisce l'ultimo) e notifica se iscritti.
void sendPacket(const uint8_t *buf) {
  if (!chrNotify) return;
  chrNotify->setValue(buf, bbp::PACKET);
  if (gSubscribed && gConnected) chrNotify->notify();
  lastSendMs = millis();
  sentOnce = true;
}

void resetPending() {
  if (queue) xQueueReset(queue);
  dumpNext = bbp::DUMP_PAGES;
}

} // namespace

bool begin(const char *name, int8_t txPowerDbm, const Provider &p) {
  provider = p;
  if (!queue) queue = xQueueCreate(QUEUE_DEPTH, sizeof(uint8_t));
  if (!queue) return false;

  if (!NimBLEDevice::init(name)) return false;
  NimBLEDevice::setPower(txPowerDbm);

  NimBLEServer *server = NimBLEDevice::createServer();
  server->setCallbacks(new ServerCb()); // una volta sola, vive per sempre
  server->advertiseOnDisconnect(true);

  NimBLEService *svc = server->createService(SERVICE_UUID);

  // L'ordine conta: sul Battle Pass vero i comandi vanno all'handle 15 e le
  // notifiche arrivano dall'handle 17. Con f001 creata per prima si ottiene
  // f001 = dichiarazione 14 / valore 15 e f002 = dichiarazione 16 / valore 17
  // / CCCD 18. Invertendo le due, gli handle non tornano.
  NimBLECharacteristic *f001 =
      svc->createCharacteristic(CHR_F001_UUID, NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  chrNotify = svc->createCharacteristic(CHR_F002_UUID, NIMBLE_PROPERTY::NOTIFY | NIMBLE_PROPERTY::WRITE |
                                                           NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::READ);
  ChrCb *cb = new ChrCb(); // condivisa, una volta sola
  f001->setCallbacks(cb);
  chrNotify->setCallbacks(cb);

  if (!server->start()) return false;

  // Advertising: flags + UUID completo del service; il nome nello scan response.
  NimBLEAdvertising *adv = NimBLEDevice::getAdvertising();
  NimBLEAdvertisementData advData;
  advData.setFlags(0x06); // LE general discoverable, BR/EDR non supportato
  advData.addServiceUUID(NimBLEUUID(SERVICE_UUID));
  NimBLEAdvertisementData scanData;
  scanData.setName(name);
  adv->setAdvertisementData(advData);
  adv->setScanResponseData(scanData);
  adv->enableScanResponse(true);
  return adv->start();
}

void setQuiet(bool quiet) { gQuiet = quiet; }

void loop() {
  if (!queue) return;
  if (gAbandon) { // client sparito (anche a meta' dump)
    gAbandon = false;
    resetPending();
  }
  if (gQuiet) return;
  if (sentOnce && (uint32_t)(millis() - lastSendMs) < BLE_NOTIFY_SPACING_MS) return;

  uint8_t buf[bbp::PACKET];

  // Dump in corso: una pagina per chiamata.
  if (dumpNext < bbp::DUMP_PAGES) {
    if (!gConnected) {
      dumpNext = bbp::DUMP_PAGES;
      return;
    }
    sendPacket(dumpPages[dumpNext++]);
    return;
  }

  uint8_t act;
  if (xQueueReceive(queue, &act, 0) != pdTRUE) return;

  switch (act) {
    case ACT_CLEAR:
      if (provider.clear) provider.clear();
      // poi A0 aggiornato, come per ACT_STATUS
      /* fallthrough */
    case ACT_STATUS:
      if (!provider.status) return;
      provider.status(buf);
      sendPacket(buf);
      break;
    case ACT_DUMP:
      if (!provider.dump) return;
      provider.dump(dumpPages); // snapshot coerente, preso una volta sola
      dumpNext = 0;
      sendPacket(dumpPages[dumpNext++]);
      break;
    default:
      break;
  }
}

void notifyLaunch() {
  if (!gSubscribed) return;
  post(ACT_STATUS);
  post(ACT_DUMP);
}

void notifyStatus() {
  if (gSubscribed) post(ACT_STATUS);
}

bool connected() { return gConnected; }
bool subscribed() { return gSubscribed; }
uint32_t commandsReceived() { return gCommands; }

} // namespace ble
} // namespace spinlite
