#ifndef CONFIG_H
#define CONFIG_H

// WiFi Configuration
// Copy this file to config.h and fill in your actual credentials
#define WIFI_SSID "Your_WiFi_Network_Name"
#define WIFI_PASSWORD "Your_WiFi_Password"
#define WIFI_TIMEOUT 10000  // Connection timeout in milliseconds

// Home Assistant Configuration
#define HA_HOST "192.168.1.100"                       // Your Home Assistant IP address
#define HA_PORT 8123                                  // Home Assistant WebSocket port (usually 8123)
#define HA_TOKEN "your_long_lived_access_token_here"  // Generate from HA Profile > Security

// La Marzocco Linea Micra
#define LINEA_MICRA_ENTITY_ID "switch.kyles_linea_micra"  // Entity
#define LINEA_MICRA_BOILER_TEMP_ENTITY_ID "number.kyles_linea_micra_coffee_target_temperature"
#define LINEA_MICRA_PREBREW_TIME_ENTITY_ID "number.kyles_linea_micra_prebrew_on_time"  // Entity ID for pre-brew time
#define LINEA_MICRA_PREBREW_WAIT_ENTITY_ID \
  "number.kyles_linea_micra_prebrew_off_time"  // Entity ID for pre-brew wait time
#define LINEA_MICRA_PREBREW_MODE_ENTITY_ID \
  "select.kyles_linea_micra_prebrew_infusion_mode"  // Entity ID for pre-brew infusion mode

// Example entities you might want to control
// #define EXAMPLE_SWITCH "switch.bedroom_light"
// #define EXAMPLE_LIGHT "light.living_room"
// #define EXAMPLE_INPUT_NUMBER "input_number.temperature_setpoint"

#endif