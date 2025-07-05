# LaMarzocco Cloud API Integration

This project now supports both Home Assistant integration and direct LaMarzocco Cloud API access, allowing you to control your Linea Micra coffee machine directly without requiring Home Assistant as an intermediary.

## Overview

The implementation includes:
- **HTTPClientWrapper**: A general-purpose HTTP client for making authenticated requests
- **LaMarzoccoCloudClient**: A specialized client for the LaMarzocco Cloud API
- **Enhanced LineaMicra class**: Supports both HomeAssistant and Cloud API backends
- **Backward compatibility**: Existing HomeAssistant integration remains unchanged

## Setup

1. Copy `src/config.example.h` to `src/config.h`
2. Fill in your credentials and settings
3. Choose your preferred mode by setting `USE_CLOUD_API` in `main.cpp`

### Home Assistant Mode (Default)
Configure the Home Assistant settings in `config.h`:
```c
#define HA_HOST "192.168.1.100"
#define HA_PORT 8123
#define HA_TOKEN "your_long_lived_access_token_here"
```

### LaMarzocco Cloud API Mode
Configure the LaMarzocco credentials in `config.h`:
```c
#define LM_USERNAME "your_lamarzocco_username"
#define LM_PASSWORD "your_lamarzocco_password"  
#define LM_SERIAL_NUMBER "your_machine_serial"
```

In `src/main.cpp`, set:
```c
#define USE_CLOUD_API true
```

## API Features

### Read Operations (Both modes)
- `isOn()` - Machine on/off state
- `getBoilerTemperature()` - Current boiler temperature
- `isPreBrewOn()` - Pre-brew mode status
- `getPreBrewTime()` - Pre-brew on time in seconds
- `getPreBrewWait()` - Pre-brew wait time in seconds

### Control Operations (Cloud API only)
- `setPower(bool enabled)` - Turn machine on/off
- `setBoilerTemperature(float temp)` - Set target temperature (°C)
- `setPreBrewMode(bool enabled)` - Enable/disable pre-brew
- `setPreBrewTimes(float onTime, float waitTime)` - Set pre-brew timing (seconds)
- `refreshFromCloud()` - Manually refresh state from cloud

## Usage Examples

### Basic Cloud API Usage
```cpp
#include "LaMarzoccoCloudClient.h"
#include "LineaMicra.h"

LaMarzoccoCloudClient cloudClient(username, password, serialNumber);
LineaMicra machine(&cloudClient);

// Authenticate
cloudClient.authenticate();

// Read state
bool isOn = machine.isOn();
float temp = machine.getBoilerTemperature();

// Control machine
machine.setPower(true);
machine.setBoilerTemperature(93.0);
machine.setPreBrewMode(true);
machine.setPreBrewTimes(2.0, 1.5);
```

### HomeAssistant Mode (Existing)
```cpp
#include "HomeAssistantClient.h"
#include "LineaMicra.h"

HomeAssistantClient haClient(host, port, token);
LineaMicra machine(&haClient);

// Connect to HA
haClient.connect();

// Read state (automatically updated via WebSocket)
bool isOn = machine.isOn();
float temp = machine.getBoilerTemperature();
```

See `examples/cloud_api_example.cpp` for a complete working example.

## Implementation Details

### HTTPClientWrapper
- Built on Arduino ESP32's built-in HTTPClient library
- Automatic JSON serialization/deserialization with ArduinoJson
- Configurable timeouts and headers
- Comprehensive error handling and logging

### LaMarzoccoCloudClient
- OAuth2-style authentication with automatic token refresh
- Implements all required LaMarzocco Cloud API endpoints
- Robust error handling for network and API failures
- Thread-safe token management

### Authentication Flow
1. Sign in with username/password to get access token
2. Use Bearer token for authenticated API requests
3. Automatically refresh tokens before expiration
4. Fall back to re-authentication if refresh fails

### API Endpoints Implemented
- `POST /auth/signin` - Initial authentication
- `POST /auth/refreshtoken` - Token refresh
- `GET /things/{serial}/dashboard` - Machine state
- `POST /things/{serial}/command/CoffeeMachineChangeMode` - Power control
- `POST /things/{serial}/command/CoffeeMachineSettingCoffeeBoilerTargetTemperature` - Temperature
- `POST /things/{serial}/command/CoffeeMachinePreBrewingChangeMode` - Pre-brew mode
- `POST /things/{serial}/command/CoffeeMachinePreBrewingSettingTimes` - Pre-brew timing

## Error Handling

- Network failures are logged and handled gracefully
- Authentication failures trigger automatic re-authentication
- Invalid API responses are detected and reported
- State updates only occur on successful API calls
- Timeout handling for slow network connections

## Performance Characteristics

### Home Assistant Mode
- **Real-time updates**: WebSocket subscriptions provide instant state changes
- **Low latency**: Direct local network connection
- **Continuous connection**: Maintains persistent WebSocket connection

### Cloud API Mode  
- **On-demand updates**: State refreshed when requested or on timer
- **Internet dependent**: Requires stable internet connection
- **Rate limited**: Respects cloud API rate limits (5-second refresh intervals)

## Dependencies

**No additional dependencies required** - uses only libraries already included:
- `HTTPClient` (built-in ESP32 Arduino framework)
- `ArduinoJson` (already included for HomeAssistant integration)
- `WiFi` (built-in ESP32 Arduino framework)

## Troubleshooting

### Authentication Issues
- Verify username/password are correct for LaMarzocco app
- Check that machine serial number is accurate
- Ensure stable internet connection during authentication

### Network Issues
- Check WiFi connection stability
- Verify firewall settings allow HTTPS traffic
- Monitor Serial output for detailed error messages

### API Errors
- Commands may fail if machine is in an incompatible state
- Some settings require machine to be powered on
- Rate limiting may cause temporary failures

## Security Considerations

- Credentials are stored in `config.h` (excluded from git)
- Access tokens are automatically refreshed and managed securely
- HTTPS is used for all cloud API communications
- No credentials are logged or transmitted in plain text