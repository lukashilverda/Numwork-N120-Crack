# N0120 (N120) Bootloader Unlock & Custom Firmware  

> **⚠️ WORK IN PROGRESS — DO NOT FOLLOW THESE INSTRUCTIONS YET**  
> This guide is still being developed and tested. Following it blindly **may permanently brick your calculator**.  
> Proceed only if you fully understand the risks and are prepared to recover using external hardware (ST-Link).

---

## What is this?

This repository contains everything related to unlocking the NumWorks N0120 (also referred to as N120) and running custom firmware on it.  
It includes:

- A **custom bootloader** (source code provided)  
- Hardware documentation (unofficial PCB, chip datasheets)  
- A step‑by‑step unlocking procedure (work in progress)

The bootloader is a proof‑of‑concept: once flashed, the calculator’s LED cycles through rainbow colours instead of the normal green/red behaviour.

**If you’d like to contribute** (especially if you speak Dutch, but English will also work), please reach out on Discord: **`LukasH`**. Or join the [Omega Discord Server](https://discord.gg/KMWPR4MJrq)

---

## Hardware Information

- **Model:** NumWorks N0120 (marketed as N120)  
- **Unofficial PCB reference:** [View on EasyEDA](https://pro.easyeda.com/editor#id=6ccc4bf0c31b4ff1b9705d23d34b74a4) *(may contain errors, use at your own risk)*  

| Component         | Specification                                                                 |
|-------------------|-------------------------------------------------------------------------------|
| **MCU**           | STM32H725VET6, ARM Cortex‑M7 @ 550 MHz, LQFP‑100 package                      |
| **RAM**           | 564 KB internal                                                               |
| **Storage**       | 512 KB on‑chip Flash + 8 MB external Quad‑SPI Flash                           |
| **Display**       | 2.8" (7.1 cm) 320×240 pixel, 16‑bit colour, 140 PPI, ST7789V controller       |
| **Battery**       | 1450 mAh Li‑Po                                                                |

**Datasheets:**  
- [STM32H725AE](https://www.st.com/resource/en/datasheet/stm32h725ae.pdf)  
- [ST7789V display controller](https://newhavendisplay.com/content/datasheets/ST7789V.pdf)

---

## Prerequisites

- A **NumWorks N0120** calculator (check the model on the back sticker)  
- **ST‑Link V2** or compatible debug probe (only needed if the calculator is “locked”)  
- **USB‑C cable** (data capable)  
- A **small Phillips screwdriver** to open the case  
- Software tools (see below)

---

## Step‑by‑Step Unlock (EXPERIMENTAL)

> **Important:**  
> Before you begin, back up any scripts, data, or settings from your calculator using the official NumWorks web interface. Opening the device will disconnect the battery and erase all unsaved data.

### 1. Open the calculator

Carefully remove the back cover and **unplug the battery**.  
*You can find teardown guides online — the N0120 is very similar to earlier models.*

### 2. Enter BOOT0 mode (for unlocked calculators)

- Keep the battery disconnected.  
- Connect the calculator to your PC via USB‑C.  
- Use a wire to **short the BOOT0 pin to 3.3 V** (refer to the image below / in the repo).  
- While holding the short, **press and release the RESET button**.  
- **LED turns red** → BOOT0 mode successfully entered.  
- **LED stays green** → your calculator is locked; proceed to section [5. Unlocking a locked calculator](#5-unlocking-a-locked-calculator).

![BOOT0 pin location placeholder – insert actual image](link/to/boot0_image.png)

### 3. Flash the custom bootloader (unlocked only)

1. Go to the **Releases** section of this repository and download the precompiled `internal.bin` bootloader.  
2. Visit the [WebDFU tool](https://ti-planet.github.io/webdfu_numworks/n0110/) (originally made for N0110, may work with N0120).  
3. Click **“Flash Internal”** and select the downloaded `internal.bin`.  
4. Wait for the process to finish.  
5. Unplug the USB cable, reconnect the battery, and power on.  
   - **If successful:** the LED will cycle through a rainbow pattern.  
   - **If the calculator resets normally:** the flash did not work, or your device is locked.

---

## 5. Unlocking a locked calculator

If your N0120 is locked, the internal Flash is protected and cannot be written over USB. You’ll need an **ST‑Link** debug probe.

### Required hardware

- An ST‑Link V2 (e.g. [this one from AliExpress](https://fr.aliexpress.com/item/1005005273159580.html) or any official clone)  
- Three female‑to‑female jumper wires

### Wiring

| ST‑Link pin | Calculator PCB test point |
|-------------|----------------------------|
| SWCLK      | SWCLK (bottom‑left header) |
| SWDIO      | SWDIO (bottom‑left header) |
| GND        | GND (e.g. USB‑C shield)    |

*Refer to the image below for the exact test point locations.*

![ST-Link wiring placeholder](link/to/wiring_image.png)

### Software: STM32CubeProgrammer

1. Download and install [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html).  
2. Connect the ST‑Link to your PC and to the calculator (battery **disconnected**, USB‑C also **disconnected** from PC).  
3. Launch STM32CubeProgrammer, select **ST‑Link** as the interface, and click **Connect**.  
4. Once connected, go to the **“OB” (Option Bytes)** tab.  
   - **Read Out Protection:** change from `CC` (Level 1) to **`AA`** (Level 0 – no protection).  
   - **Write Protection:** disable **all** sectors (uncheck or set to “No protection”).  
5. Click **Apply**. The chip will be mass‑erased and unlocked.  
6. **Disconnect** the ST‑Link and remove the wires.

After this procedure, the calculator behaves like an unlocked device. You can now:

- Go back to [Step 2](#2-enter-boot0-mode-for-unlocked-calculators) and flash the custom bootloader via WebDFU, **or**  
- Directly flash the bootloader `.bin` using STM32CubeProgrammer (select **Download** → choose the file → start).

---

## 6. After flashing the bootloader

Your N0120 now has a custom bootloader installed!  

**Next steps (at your own risk):**

- Try installing a custom firmware like **Omega** or **Upsilon**.  
  *Note: I have not confirmed whether these images work directly on the N0120; you may need to build them specifically for the H725 target.*  
- Develop your own firmware or tools. The source code in this repo is a good starting point.

---

## Troubleshooting & Community

- Double‑check all connections — loose wires are the most common problem.  
- If WebDFU fails, try a different browser (Chromium‑based works best) or use STM32CubeProgrammer directly.  
- Need help? Join the discussion or contact me on Discord: **`dotmazy`**  
  *French speakers are particularly welcome!*

---

## Disclaimer

This project is **not affiliated with NumWorks** in any way.  
Modifying your calculator’s bootloader **voids the warranty** and can permanently damage the device.  
All information is provided for educational and research purposes only. **Use at your own risk.**

---

## License

*(Add a license here — e.g., MIT, GPL, or “All rights reserved”)*