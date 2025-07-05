# LaMarzocco Cloud API Integration

This project now supports both Home Assistant integration and direct LaMarzocco Cloud API access.

## Setup

1. Copy `src/config.example.h` to `src/config.h`
2. Fill in your credentials and settings

### Home Assistant Mode (Default)
Configure the Home Assistant settings:
```c
#define HA_HOST "192.168.1.100"
#define HA_PORT 8123
#define HA_TOKEN "your_long_lived_access_token_here"
```

### LaMarzocco Cloud API Mode
Configure the LaMarzocco credentials:
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

### Read-only Operations (Both modes)
- Machine on/off state
- Boiler temperature
- Pre-brew mode (enabled/disabled)
- Pre-brew on time
- Pre-brew wait time

### Control Operations (Cloud API only)
- `setPower(bool enabled)` - Turn machine on/off
- `setBoilerTemperature(float temp)` - Set target temperature
- `setPreBrewMode(bool enabled)` - Enable/disable pre-brew
- `setPreBrewTimes(float onTime, float waitTime)` - Set pre-brew timing

## Implementation Details

### HTTP Client
- `HTTPClientWrapper` - Generic HTTP client with JSON support
- Handles authentication headers
- Built on Arduino HTTPClient library

### LaMarzocco Cloud Client  
- `LaMarzoccoCloudClient` - Specific API client for LaMarzocco cloud
- Handles authentication (signin/token refresh)
- Implements all required API endpoints

### LineaMicra Class
- Updated to support both client types
- Auto-refreshes state from cloud API when needed
- Maintains backward compatibility with Home Assistant mode

## Authentication

The LaMarzocco Cloud API uses OAuth2-style authentication:
1. Sign in with username/password to get access token
2. Use Bearer token for authenticated requests  
3. Automatically refresh tokens before expiration

## Error Handling

- Network failures are logged to Serial
- Authentication failures fall back to re-authentication
- Invalid responses are handled gracefully
- State updates only occur on successful API calls

## Performance

- Home Assistant mode: Real-time updates via WebSocket subscriptions
- Cloud API mode: Polling every 5 seconds for state updates, on-demand for getters

## Dependencies

No additional dependencies required - uses built-in Arduino ESP32 libraries:
- HTTPClient (built-in)
- ArduinoJson (already included)
- WiFi (built-in)