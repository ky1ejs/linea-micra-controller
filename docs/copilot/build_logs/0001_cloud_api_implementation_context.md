# Cloud API Implementation Context

**Development Log**: LaMarzocco Cloud API Integration  
**Date**: July 2025  
**Context**: Implementation of direct Cloud API access for LineaMicra Controller

## Architecture Overview

Implemented dual-client architecture:
- **Home Assistant Client**: Real-time webhook subscriptions for state updates
- **LaMarzocco Cloud Client**: Direct API access for initial state and control operations

## Key Components Implemented

### HTTPClientWrapper
- General-purpose HTTP client with JSON handling
- Built on ESP32's native HTTPClient library
- Authentication header management and error handling

### LaMarzoccoCloudClient
- OAuth2-style authentication with automatic token refresh
- All machine control endpoints implemented
- Robust error handling and timeout management

### Enhanced LineaMicra Class
- Dual-client support with optimistic updates
- 1-second debouncing to prevent API spam
- Visual pending state indicators (asterisk in UI)
- Automatic state confirmation via HA webhooks

## API Endpoints Implemented
- `POST /auth/signin` - Authentication
- `POST /auth/refreshtoken` - Token refresh
- `GET /things/{serial}/dashboard` - Machine state
- `POST /things/{serial}/command/CoffeeMachineChangeMode` - Power control
- `POST /things/{serial}/command/CoffeeMachineSettingCoffeeBoilerTargetTemperature` - Temperature
- `POST /things/{serial}/command/CoffeeMachinePreBrewingChangeMode` - Pre-brew mode
- `POST /things/{serial}/command/CoffeeMachinePreBrewingSettingTimes` - Pre-brew timing

## Implementation Flow
1. User calls LineaMicra method to update value
2. Optimistic UI update with pending indicator
3. 1-second debounce before API call
4. Cloud API call to set value
5. HA webhook confirms change and removes pending state
6. 10-second timeout fallback for pending states

## Dependencies
- No third-party libraries added
- Uses existing ArduinoJson library
- Built-in ESP32 Arduino framework libraries only

## Build and Quality Notes
- Network restrictions in CI prevent PlatformIO registry access
- Use offline tools: cppcheck instead of `pio check`
- cppcheck-*.txt files should be gitignored as build artifacts
- Maintain Google C++ style with 2-space indentation

## Key Files Modified
- `src/HTTPClientWrapper.{cpp,h}` - HTTP client implementation
- `src/LaMarzoccoCloudClient.{cpp,h}` - Cloud API client
- `src/LineaMicra.{cpp,h}` - Enhanced with dual-client support
- `src/main.cpp` - Dual-client initialization
- `src/config.example.h` - Added Cloud API configuration

## Error Handling Strategy
- Network failures logged and handled gracefully
- Authentication failures trigger automatic re-authentication
- Invalid responses detected and reported
- State updates only on successful API calls
- Timeout handling for slow connections

## Performance Characteristics
- **HA Mode**: Real-time WebSocket updates, low latency
- **Cloud Mode**: On-demand updates, internet dependent, rate limited
- **Dual Mode**: Best of both - real-time updates + direct control