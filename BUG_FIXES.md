# Bug Fixes for Smart Parking System (智能停车场系统)

## Project Information
- **Authors**: 王子柏 (2023132316), 张智研 (2023132319)
- **Platform**: STM32F103C8T6
- **Date**: 2026-03-18

## Bugs Fixed

### Bug 1: Sensor Cooldown Timer Timing Issue
**Location**: `TCC/sensor.c` lines 125-130

**Problem**:
The cooldown timer decrement was being called inside `Sensor_Scan()`, which executes every 10ms. However, the `COOLDOWN_TIME` constant is defined as 50 units, intended to represent 5 seconds when decremented every 100ms (50 * 100ms = 5000ms = 5s). With the timer being decremented every 10ms, the actual cooldown time was only 500ms (50 * 10ms) instead of the intended 5 seconds.

**Solution**:
1. Removed the cooldown timer decrement code from `Sensor_Scan()` (which runs every 10ms)
2. Created a new function `Sensor_Update_Cooldown()` to handle the cooldown timer decrement
3. Added the function declaration to `sensor.h`
4. Updated `main.c` to call `Sensor_Update_Cooldown()` in the 100ms task (line 244)

**Impact**:
The cooldown timer now correctly implements a 5-second delay between successive sensor triggers, preventing false triggers and ensuring the system properly filters out repeated vehicle detections.

---

### Bug 2: Parking Empty Count Not Synchronized
**Location**: `TCC/parking.c`

**Problem**:
The global variable `g_parking_empty` (displayed on OLED screens) was not being synchronized with `g_emptyCount` (calculated from sensor readings in `sensor.c`). This caused the display to show incorrect available parking spot counts, as `g_parking_empty` was only initialized once and never updated during runtime.

**Solution**:
Added synchronization code in `Parking_Process()` function at line 131:
```c
// 同步空车位数量
g_parking_empty = g_emptyCount;
```

**Impact**:
The OLED displays now correctly show real-time available parking spot counts based on actual sensor readings, improving user experience and system accuracy.

---

## Additional Improvements

### Added .gitignore File
Created `.gitignore` to exclude build artifacts (*.o, *.d, *.axf, build directories, etc.) from version control, keeping the repository clean and focused on source code.

---

## Testing Recommendations

1. **Cooldown Timer Test**:
   - Trigger the entry sensor multiple times within 5 seconds
   - Verify that only the first trigger opens the gate
   - Verify that triggers after 5 seconds work correctly

2. **Parking Count Display Test**:
   - Park vehicles in different spots
   - Verify OLED displays update correctly
   - Check "车位已满" (Parking Full) warning appears when all spots occupied
   - Verify count increases when vehicles exit

3. **System Integration Test**:
   - Test automatic mode vehicle entry/exit flow
   - Test manual mode gate control
   - Verify alarm system triggers when parking is full

---

## Code Quality Notes

The fixes maintain the existing code structure and style:
- Followed existing naming conventions
- Added appropriate Chinese and English comments
- Maintained separation of concerns (sensor logic in sensor.c, parking logic in parking.c)
- Used existing timer infrastructure (10ms, 100ms, 1s tasks)
