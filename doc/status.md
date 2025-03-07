# Status codes (register 0)

| code | Description                   | Details                                                                 |
|------|-------------------------------|-------------------------------------------------------------------------|
| 0x00 | Standby                       |                                                                         |
| 0x01 | Fault                         |                                                                         |
| 0x02 | Programming                   |                                                                         |
| 0x04 | PV                            | PV power feeds back to grid. AC energy storage does not have this variable. |
| 0x08 | PV charging                   | PV power charges the battery. AC energy storage does not have this variable. |
| 0x0C | PV charging + grid            | A portion of the PV power is used for battery charging, while another portion is used for feeding back to the grid, and AC energy storage does not have this variable. |
| 0x10 | Battery + grid                | The power from battery discharge feeds back to the grid.                 |
| 0x14 | (PV + battery) + grid         | Battery discharge and PV energy are connected to the grid together, and AC energy storage does not have this variable. |
| 0x20 | AC charging                   | Grid charges the battery.                                               |
| 0x28 | (PV + AC) charging            | PV and grid power charge the battery together, and AC energy storage does not have this variable. |
| 0x40 | Battery off-grid              | Off-grid mode, battery discharge.                                       |
| 0x60 | Off-grid + battery charging   | On-grid system charges the battery (AC Coupled).                        |
| 0x80 | PV off-grid                   | PV power fluctuates randomly, the off-grid output is unstable, and the inverter is prohibited from working in this state, and AC energy storage does not have this variable. |
| 0xC0 | (PV + battery) off-grid       | In off-grid mode, PV power and battery discharge, and AC energy storage does not have this variable. |
| 0x88 | PV charging + off-grid        | A portion of the PV power is used for off-grid output, while another portion is used for battery charging, and AC energy storage does not have this variable. |
