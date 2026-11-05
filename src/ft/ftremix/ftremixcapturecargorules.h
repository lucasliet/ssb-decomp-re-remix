#ifndef _FTREMIXCAPTURECARGORULES_H_
#define _FTREMIXCAPTURECARGORULES_H_

#if defined(PORT) && defined(REGION_REMIX)

/*
 * Pure rule of JDK's cargo hold escape base (JDK.asm:80 jdk_cargo_, hook at 0x8014E594 in ftCommonCaptureShoulderedSetStatus):
 * the fighter held on a shoulder needs more inputs to escape from J DK, Marina and N Marina. Only int and float, so the headless
 * unit tests (tests/remix) can run it.
 */

#define FTREMIX_CARGO_ESCAPE_BASE_DEFAULT 14.0F /* 0x41600000, the US value */
#define FTREMIX_CARGO_ESCAPE_BASE_HEAVY 20.0F   /* 0x41A00000 */

/**
 * Base of the breakout counter of a fighter held on a shoulder.
 *
 * @param grabber_fkind Remix fighter id of the fighter carrying the cargo.
 * @return FTREMIX_CARGO_ESCAPE_BASE_HEAVY for JDK, Marina and N Marina, else FTREMIX_CARGO_ESCAPE_BASE_DEFAULT.
 */
extern float ftRemixCaptureCargoEscapeBase(int grabber_fkind);

#endif

#endif
