# examine: serial.h

Wave 6 classification. Upstream: `ref/memtest86+-2.00/serial.h` (163 lines).
Classify-only — NOT compiled into the PPC build; `src/` and the Makefile unchanged.

## File & purpose
`serial.h` (Ts'o's `include/linux/serial.h`, guard `_LINUX_SERIAL_REG_H`) is the
register map for an **8250/16450/16550(A) UART**, used by memtest86+ to mirror the
TUI to a serial console.

- Register offsets from the UART base (lines 17–30): `UART_RX`/`UART_TX`/`UART_DLL`
  (0), `UART_DLM`/`UART_IER` (1), `UART_IIR`/`UART_FCR`/`UART_EFR` (2), `UART_LCR`
  (3), `UART_MCR` (4), `UART_LSR` (5), `UART_MSR` (6), `UART_SCR` (7).
- Bitfield constants for each register (lines 34–136): FCR FIFO-trigger masks, LCR
  word-length/parity/DLAB bits, LSR `THRE`/`TEMT`/`DR` status bits, IIR/IER
  interrupt bits, MCR modem-control bits, MSR modem-status bits, EFR flow-control.
- **`#include "io.h"`** (line 138) — pulls in x86 port I/O, because:
- The serial-console glue (139–161): `serial_echo_outb(v,a)` / `serial_echo_inb(a)`
  are `outb`/`inb` to `a + serial_base_ports[serial_tty]`; `WAIT_FOR_XMITR` spins
  on `UART_LSR & (UART_LSR_TEMT|UART_LSR_THRE)`; `serial_echo()`/`serial_debug()`
  (148–161) are the byte-out helpers (themselves already `#if 0`'d upstream).

## Disposition: ⛔ N/A on PPC/OF — not ported
The register map is for a PC-style 16550 UART addressed through x86 **port I/O**
(`io.h`, line 138). On a PPC Mac there is no such UART at PC I/O ports, and OF owns
console I/O. The serial-console *leaf* is exactly the one PORTING-GUIDE §3 says to
drop ("`serial_echo_*` become no-ops/removed"); all screen output is routed through
the framebuffer renderer instead.

## What already-ported substrate replaces it
- **Console output**: PORTING-GUIDE §3 — `cprint`/`scroll` write into `vga_buf[]`
  and `ttyprint()` is reimplemented as the framebuffer renderer (`display.c`),
  not as serial escapes. The serial mirror (`tty_print_line`'s serial side,
  `serial_echo_*`) is removed.
- **Console input** (the other thing serial was used for, the `serial_cons`
  fallback in the old `get_key()`): replaced by `check_input()` polling OF
  **stdin** via the OF client interface (`ofw.h`) — see `lib.c:33` ("OF client
  interface for check_input()'s stdin poll").
- `lib.c:23–29` explicitly documents the swap: `io.h`/`serial.h` (the UART defs)
  are N/A; their only consumers were the PC-keyboard `get_key()`, the
  `serial_echo_*` console, and `beep` — all commented out — and the platform layer
  is `ofw.h`. `lib.c:911` notes the serial setup (line control / baud via
  `serial_echo_inb/outb`) is likewise N/A.

## Live callers in src/: none
- `serial.h` is **not `#include`d** in `src/` — only the commented include at
  `lib.c:29`.
- No live use of `UART_*`, `serial_echo_*`, `serial_base_ports`, `serial_tty`,
  `WAIT_FOR_XMITR`, `serial_cons`. The lone `serial_echo_inb`/`UART_LSR`/`UART_RX`
  references sit inside `lib.c`'s `#if 0` `get_key()` (`lib.c:600–604`); the
  `serial_cons` mentions are in commented prose. Zero live serial code.

No build impact. No surprises — the serial console is consciously dropped (§3) and
console I/O is OF-mediated.
