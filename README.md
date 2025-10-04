
# nRF Connect + Zephyr Firmware Take-Home — FW Challenge
**Repository scaffold and reference implementation**

**What this bundle contains**
- `app/` — main application sources, headers, prj.conf, Kconfig, overlays for `native_sim` and `nrf5340dk`.
- `tests/` — minimal ztest tests for config precedence and little-endian encoding/decoding of GATT values.
- `README.md` — this file (notes, how to build, and what was implemented).
- A small, pragmatic implementation that demonstrates:
  - DeviceTree overlays and custom `/app` node usage
  - Kconfig symbols with precedence handling (Kconfig overrides DT if set)
  - k_work_delayable sampling loop, button interrupt + deferred work, logging
  - settings/NVS persistence of `sample_count`
  - BLE peripheral with a custom 128-bit service (voltage_mv, sample_interval_ms)
  - LED state machine (idle / sampling / error)

**Important:** This is a focused reference implementation intended to be run and iterated on in the **nRF Connect VS Code extension** (NCS >= 3.1). You may need to adjust minor device-tree labels (ADC channel alias, LED mapping) depending on local VS Code/NCS versions. See *README-notes* in `app/README.md` for specifics and rationale.

**Download the ZIP**: [nrf-zephyr-challenge.zip](sandbox:/mnt/data/nrf-zephyr-challenge.zip)

---
