
# App README — FW Challenge (reference implementation)

## Build & run (recommended: nRF Connect VS Code extension)
1. Open NCS workspace in VS Code using the **nRF Connect for VS Code** extension (NCS >= 3.1).
2. Open the `app` folder as a Zephyr project.
3. Select the board target **native_sim** from the extension's build/run UI, or choose **nrf5340dk_nrf5340_cpuapp** to target the DK.
4. Build and run the simulation. For native_sim the devicetree overlay provides an ADC emulator node configured with a fixed `mv` value.

### Quick steps (native_sim)
- Build: Use the extension "Build" action for the project (native_sim).
- Run: Use the "Simulate" action. Open the Serial Console to observe logs.

## Devicetree choices
- `native_sim.overlay` defines:
  - alias `led-status` (gpio LED), `btn-user` (gpio button), and `adc0` pointing to an ADC emulator node.
  - A custom `/app` node (compatible = "app,config") with properties:
    - `sample-interval-ms` (default 1000)
    - `voltage-threshold-mv` (default 3300)
    - `enable-ble` (present = enable)

- `nrf5340dk_nrf5340_cpuapp.overlay` provides platform-specific wiring for LED/button/adc aliases.

Access devicetree props in code via `DT_PROP(DT_PATH(app), sample_interval_ms)` and test presence via `DT_NODE_HAS_COMPAT(DT_PATH(app), app_config)`.

## Kconfig & precedence policy
- Kconfig symbols are defined under "FW Challenge Options".
- Policy: If `CONFIG_APP_SAMPLE_INTERVAL_MS` is set to a non-zero value at build time, **Kconfig wins** and overrides the devicetree `sample_interval_ms`. This is implemented in `app/include/app_config.h`. This approach makes it easy to change sampling behavior per-build while still allowing board-level defaults in devicetree.

## BLE UUIDs and characteristics
- Service UUID: app-specific 128-bit UUID (see `app/include/app_uuids.h`).
- Characteristics:
  - `voltage_mv`: Read / Notify. Little-endian u32.
  - `sample_interval_ms`: Read-only, little-endian u32.

Advertising name: "FW-CHALLENGE". CCC changes are logged. Notifications are gated by CCC and by the user button toggle (runtime).

## Settings / persistence
- `sample_count` is persisted in Settings with key `app/sample_count` (handler `settings_set` under "app"). Stored with `settings_save_one("app/sample_count", &sample_count, sizeof(sample_count))`.
- On boot, `settings_load()` restores the saved `sample_count` (see `adc_sampler.c`).

## Power hints
- The implementation uses `k_work_delayable` to avoid busy loops and uses `pm_device_action_run(dev, PM_DEVICE_ACTION_SUSPEND/RESUME)` around ADC access where available (documented in code).

## Tests
- Minimal `ztest` unit tests are included under `tests/`:
  - `ztest_app_config`: validates Kconfig presence / precedence (build-time check).
  - `ztest_ble_codec`: validates little-endian encode/decode of u32 for gatt values.

## AI disclosure
I (the author) used generative assistance to assemble this scaffold and to help adapt Zephyr APIs into a coherent project layout. All source code in this repository was reviewed and shaped to satisfy the assignment requirements. Be prepared to explain design choices and code when asked.

