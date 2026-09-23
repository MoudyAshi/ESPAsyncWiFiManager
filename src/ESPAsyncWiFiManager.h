/**************************************************************
   WiFiManager is a library for the ESP8266/Arduino platform
   (https://github.com/esp8266/Arduino) to enable easy
   configuration and reconfiguration of WiFi credentials using a Captive Portal
   inspired by:
   http://www.esp8266.com/viewtopic.php?f=29&t=2520
   https://github.com/chriscook8/esp-arduino-apboot
   https://github.com/esp8266/Arduino/tree/esp8266/hardware/esp8266com/esp8266/libraries/DNSServer/examples/CaptivePortalAdvanced
   Built by AlexT https://github.com/tzapu
   Ported to Async Web Server by https://github.com/alanswx
   Licensed under MIT license
 **************************************************************/

#ifndef ESPAsyncWiFiManager_h
#define ESPAsyncWiFiManager_h

#if defined(ESP8266)
#include <ESP8266WiFi.h> // https://github.com/esp8266/Arduino
#else
#include <WiFi.h>
#include "esp_wps.h"
#define ESP_WPS_MODE WPS_TYPE_PBC
#endif
#include <ESPAsyncWebServer.h>

//#define USE_EADNS               // uncomment to use ESPAsyncDNSServer
#ifdef USE_EADNS
#include <ESPAsyncDNSServer.h> // https://github.com/devyte/ESPAsyncDNSServer
                               // https://github.com/me-no-dev/ESPAsyncUDP
#else
#include <DNSServer.h>
#endif
#include <memory>

// fix crash on ESP32 (see https://github.com/alanswx/ESPAsyncWiFiManager/issues/44)
#if defined(ESP8266)
typedef int wifi_ssid_count_t;
#else
typedef int16_t wifi_ssid_count_t;
#endif

#if defined(ESP8266)
extern "C"
{
#include "user_interface.h"
}
#else
#include <rom/rtc.h>
#endif

const char WFM_HTTP_HEAD[] PROGMEM = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
/* Visual language from lukerobotarm.com (src/style.css): dark page, system font, orange actions, blue links. */
const char HTTP_STYLE[] PROGMEM =
"<style>"
":root{font-family:system-ui,'Segoe UI',Avenir,Helvetica,Arial,sans-serif;line-height:1.55;font-weight:400;color-scheme:dark;color:rgba(255,255,255,.9);background:#1a1a1a;font-size:17px;--accent:#ff6600;--accent-hover:#ff8800;--link:#8ab4ff;--border:#444;}"
"*{box-sizing:border-box;}"
"body{margin:0;width:100%;min-height:100vh;text-align:center;font-family:inherit;background:#1a1a1a;color:rgba(255,255,255,.9);-webkit-font-smoothing:antialiased;}"
"a{font-weight:500;color:var(--link);text-decoration:none;}"
"a:hover{color:#a8c7ff;text-decoration:underline;}"
"h1,h2{color:var(--accent);line-height:1.15;}"
"h1{font-size:1.5rem;margin:0;}"
"h2{font-size:1.35rem;margin:.2rem 0 .6rem;}"
"p{margin:0 0 .85rem;}"
"button,input[type=submit]{border-radius:8px;border:1px solid transparent;padding:.6em 1.2em;font-size:1em;font-weight:600;font-family:inherit;background:var(--accent);color:#fff;cursor:pointer;width:100%;line-height:2.4rem;margin-top:6px;}"
"button:hover,input[type=submit]:hover{background:var(--accent-hover);}"
"button:focus,button:focus-visible,input:focus{outline:2px solid var(--accent);outline-offset:2px;}"
"input{width:100%;padding:.55rem .75rem;margin-bottom:10px;border:1px solid var(--border);border-radius:6px;background:#333;color:#fff;font-size:1rem;font-family:inherit;}"
"input:focus{border-color:var(--accent);outline:none;}"
"::placeholder{color:#999;}"
".c{text-align:center;}"
"div{padding:5px;font-size:1em;}"
".q{float:right;width:64px;text-align:right;color:#ccc;}"
".l{background:url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size:1em;filter:invert(1);}"
".wrap{text-align:left;display:block;min-width:260px;max-width:440px;width:92%;margin:8px auto;background:#242424;border:1px solid #333;border-radius:12px;padding:.85rem 1rem 1rem;}"
".links{margin-top:8px;text-align:center;font-size:14px;opacity:.9;}"
".links a{margin:0 8px;}"
"dt{font-weight:600;color:var(--accent);margin-top:.55rem;}"
"dd{margin:0 0 .35rem 0;color:#ddd;}"
"#spinner{border:4px solid #444;border-top:4px solid var(--accent);border-radius:50%;width:36px;height:36px;animation:spin 1s linear infinite;margin:15px auto;}"
"@keyframes spin{0%{transform:rotate(0deg);}100%{transform:rotate(360deg);}}"
"#timer{font-size:32px;font-weight:700;color:var(--accent);margin:10px 0;}"
"#status{font-size:15px;font-weight:600;color:#ccc;}"
"#fallback{display:none;background:#2e2e2e;border:1px solid #3a3a3a;padding:15px;border-radius:12px;margin:15px 0;text-align:left;color:#eee;}"
"#fallback-title{margin:0 0 6px;font-weight:700;color:#ff9b9b;font-size:15px;}"
"#fallback-desc{margin:0 0 12px;font-size:13px;color:#ccc;}"
".btn{display:block;text-align:center;padding:10px;border-radius:8px;text-decoration:none !important;font-weight:700;color:#fff !important;}"
".btn:hover{text-decoration:none !important;color:#fff !important;}"
".btn-primary{background:var(--accent);}"
".btn-primary:hover{background:var(--accent-hover);}"
".btn-secondary{background:#444;border:1px solid var(--border);margin-bottom:8px;}"
".btn-secondary:hover{background:#555;}"
".pw-toggle{display:inline-flex;align-items:center;gap:6px;font-size:13px;color:#ccc;cursor:pointer;margin:4px 0 12px;user-select:none;}"
".pw-toggle input{width:auto !important;padding:0;margin:0;cursor:pointer;}"
".err{color:#ff9b9b;font-size:18px;}"
"</style>";
const char HTTP_SCRIPT[] PROGMEM = "<script>"
"function c(l){"
"  if(window.event) window.event.preventDefault();"
"  var ssid = l.getAttribute('data-ssid') || l.innerText || l.textContent;"
"  if(ssid) ssid = ssid.trim();"
"  var s = document.getElementById('s');"
"  var p = document.getElementById('p');"
"  if(s) s.value = ssid;"
"  if(p) p.focus();"
"  return false;"
"}"
"function tp(){"
"  var p = document.getElementById('p');"
"  if(p) p.type = (p.type === 'password') ? 'text' : 'password';"
"}"
"</script>";
//"<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTTP_HEAD_END[] PROGMEM = "</head><body><div class='wrap'>";
const char HTTP_PORTAL_OPTIONS[] PROGMEM = "<form action=\"/wifi\" method=\"get\"><button>Configure WiFi</button></form><br/><form action=\"/0wifi\" method=\"get\"><button>Configure WiFi (No Scan)</button></form><br/><form action=\"/i\" method=\"get\"><button>Info</button></form><br/><form action=\"/r\" method=\"post\"><button>Reset</button></form>";
const char HTTP_ITEM[] PROGMEM = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTTP_FORM_START[] PROGMEM = "<form method='get' action='wifisave'><input id='s' name='s' length=32 placeholder='SSID'><input id='p' name='p' length=64 type='password' placeholder='Password'><label class='pw-toggle'><input type='checkbox' onclick='tp()'> Show Password</label>";
const char HTTP_FORM_PARAM[] PROGMEM = "<br/><input id='{i}' name='{n}' length={l} placeholder='{p}' value='{v}' {c}>";
const char HTTP_FORM_END[] PROGMEM = "<br/><button type='submit'>Connect</button></form>";
const char HTTP_SCAN_LINK[] PROGMEM = "<br/><div class=\"c\"><a href=\"/wifi\">Scan</a></div>";
const char HTTP_SAVED[] PROGMEM = "<div>Credentials Saved<br />Trying to connect ESP to network.<br />If it fails reconnect to AP to try again</div>";
const char HTTP_END[] PROGMEM = "</div></body></html>";

static uint8_t _wifiConnectStatus = 0; // 0 = Connecting, 1 = Success, 2 = Failed
#define WIFI_MANAGER_MAX_PARAMS 10

class AsyncWiFiManagerParameter
{
public:
  AsyncWiFiManagerParameter(const char *custom);
  AsyncWiFiManagerParameter(const char *id,
                            const char *placeholder,
                            const char *defaultValue,
                            unsigned int length);
  AsyncWiFiManagerParameter(const char *id,
                            const char *placeholder,
                            const char *defaultValue,
                            unsigned int length,
                            const char *custom);

  const char *getID();
  const char *getValue();
  const char *getPlaceholder();
  unsigned int getValueLength();
  const char *getCustomHTML();

private:
  const char *_id;
  const char *_placeholder;
  char *_value;
  unsigned int _length;
  const char *_customHTML;

  void init(const char *id,
            const char *placeholder,
            const char *defaultValue,
            unsigned int length,
            const char *custom);

  friend class AsyncWiFiManager;
};

class WiFiResult
{
public:
  bool duplicate;
  String SSID;
  uint8_t encryptionType;
  int32_t RSSI;
  uint8_t *BSSID;
  int32_t channel;
  bool isHidden;

  WiFiResult()
  {
  }
};

class AsyncWiFiManager
{
public:
#ifdef USE_EADNS
  AsyncWiFiManager(AsyncWebServer *server, AsyncDNSServer *dns);
#else
  AsyncWiFiManager(AsyncWebServer *server, DNSServer *dns);
#endif

  void scan(boolean async = false);
  String scanModal();
  void loop();
  void safeLoop();
  void criticalLoop();

  boolean autoConnect(unsigned long maxConnectRetries = 1,
                      unsigned long retryDelayMs = 1000);
  boolean autoConnect(char const *apName,
                      char const *apPassword = NULL,
                      unsigned long maxConnectRetries = 1,
                      unsigned long retryDelayMs = 1000);

  // if you want to always start the config portal, without trying to connect first
  boolean startConfigPortal(char const *apName, char const *apPassword = NULL);
  void startConfigPortalModeless(char const *apName, char const *apPassword);

  // get the AP name of the config portal, so it can be used in the callback
  String getConfigPortalSSID();

  void resetSettings();

  // sets timeout before webserver loop ends and exits even if there has been no setup.
  // usefully for devices that failed to connect at some point and got stuck in a webserver loop.
  // in seconds, setConfigPortalTimeout is a new name for setTimeout
  void setConfigPortalTimeout(unsigned long seconds);
  void setTimeout(unsigned long seconds);

  // sets timeout for which to attempt connecting, usefull if you get a lot of failed connects
  void setConnectTimeout(unsigned long seconds);

  // wether or not the wifi manager tries to connect to configured access point even when
  // configuration portal (ESP as access point) is running [default true/on]
  void setTryConnectDuringConfigPortal(boolean v);

  void setDebugOutput(boolean debug);
  // defaults to not showing anything under 8% signal quality if called
  void setMinimumSignalQuality(unsigned int quality = 8);
  // sets a custom ip /gateway /subnet configuration
  void setAPStaticIPConfig(IPAddress ip, IPAddress gw, IPAddress sn);
  // sets config for a static IP
  void setSTAStaticIPConfig(IPAddress ip,
                            IPAddress gw,
                            IPAddress sn,
                            IPAddress dns1 = (uint32_t)0x00000000,
                            IPAddress dns2 = (uint32_t)0x00000000);
  // called when AP mode and config portal is started
  void setAPCallback(std::function<void(AsyncWiFiManager *)>);
  // called when settings have been changed and connection was successful
  void setSaveConfigCallback(std::function<void()> func);
  //adds a custom parameter
  void addParameter(AsyncWiFiManagerParameter *p);
  // if this is set, it will exit after config, even if connection is unsucessful
  void setBreakAfterConfig(boolean shouldBreak);
  // if this is set, try WPS setup when starting (this will delay config portal for up to 2 mins)
  // TODO
  // if this is set, customise style
  void setCustomHeadElement(const char *element);
  // if this is true, remove duplicated Access Points - defaut true
  void setRemoveDuplicateAPs(boolean removeDuplicates);
  // sets a custom element to add to options page
  void setCustomOptionsElement(const char *element);

  String getConfiguredSTASSID(){
      return _ssid;
  }
  String getConfiguredSTAPassword(){
      return _pass;
  }

private:
  AsyncWebServer *server;
#ifdef USE_EADNS
  AsyncDNSServer *dnsServer;
#else
  DNSServer *dnsServer;
#endif

  boolean _modeless;
  unsigned long scannow;
  boolean shouldscan = true;
  boolean needInfo = true;

  //const int     WM_DONE                 = 0;
  //const int     WM_WAIT                 = 10;
  //const String  HTTP_HEAD = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\"/><title>{v}</title>";

  void setupConfigPortal();
#ifdef NO_EXTRA_4K_HEAP
  void startWPS();
#endif
  String pager;
  wl_status_t wifiStatus;
  const char *_apName = "no-net";
  const char *_apPassword = NULL;
  String _ssid = "";
  String _pass = "";
  unsigned long _configPortalTimeout = 0;
  unsigned long _connectTimeout = 0;
  unsigned long _configPortalStart = 0;

  IPAddress _ap_static_ip;
  IPAddress _ap_static_gw;
  IPAddress _ap_static_sn;
  IPAddress _sta_static_ip;
  IPAddress _sta_static_gw;
  IPAddress _sta_static_sn;
  IPAddress _sta_static_dns1 = (uint32_t)0x00000000;
  IPAddress _sta_static_dns2 = (uint32_t)0x00000000;

  unsigned int _paramsCount = 0;
  unsigned int _minimumQuality = 0;
  boolean _removeDuplicateAPs = true;
  boolean _shouldBreakAfterConfig = false;
#ifdef NO_EXTRA_4K_HEAP
  boolean _tryWPS = false;
#endif
  const char *_customHeadElement = "";
  const char *_customOptionsElement = "";

  //String        getEEPROMString(int start, int len);
  //void          setEEPROMString(int start, int len, String string);

  uint8_t status = WL_IDLE_STATUS;
  uint8_t connectWifi(String ssid, String pass);
  uint8_t waitForConnectResult();
  void setInfo();
  void copySSIDInfo(wifi_ssid_count_t n);
  String networkListAsString();

  void handleRoot(AsyncWebServerRequest *);
  void handleWifi(AsyncWebServerRequest *, boolean scan);
  void handleWifiSave(AsyncWebServerRequest *);
  void handleNotFound(AsyncWebServerRequest *);
  boolean captivePortal(AsyncWebServerRequest *);

  // DNS server
  const byte DNS_PORT = 53;

  // helpers
  unsigned int getRSSIasQuality(int RSSI);
  boolean isIp(String str);
  String toStringIp(IPAddress ip);
  String getPageHeader(const String& pageTitle);

  boolean connect;
  boolean _debug = true;

  WiFiResult *wifiSSIDs;
  wifi_ssid_count_t wifiSSIDCount;
  boolean wifiSSIDscan;

  boolean _tryConnectDuringConfigPortal = true;

  std::function<void(AsyncWiFiManager *)> _apcallback;
  std::function<void()> _savecallback;

  AsyncWiFiManagerParameter *_params[WIFI_MANAGER_MAX_PARAMS];

  template <typename Generic>
  void DEBUG_WM(Generic text);

  template <class T>
  auto optionalIPFromString(T *obj, const char *s) -> decltype(obj->fromString(s))
  {
    return obj->fromString(s);
  }
  auto optionalIPFromString(...) -> bool
  {
    DEBUG_WM(F("NO fromString METHOD ON IPAddress, you need ESP8266 core 2.1.0 or newer for Custom IP configuration to work."));
    return false;
  }
};

#endif
