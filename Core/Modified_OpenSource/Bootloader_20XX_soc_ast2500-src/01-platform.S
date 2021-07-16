--- u-boot-2019.04/arch/arm/mach-aspeed/ast2500/platform.S	2019-07-22 22:57:53.000000000 -0400
+++ uboot/arch/arm/mach-aspeed/ast2500/platform.S	2020-03-23 17:38:24.241031337 -0400
@@ -668,7 +668,7 @@
     mov   pc, lr
 #else
     /* skip SDRAM initialization (will be done in C function) */
-    b     platform_exit
+#    b     platform_exit
 #endif
 
     /* Check Scratch Register Bit 6 */
@@ -2283,7 +2283,7 @@
     str   r1, [r0]
 
     ldr   r0, =0x1e6e0054
-    ldr   r1, =CONFIG_DRAM_ECC_SIZE              /* ECC protected memory size */
+    ldr   r1, =CONFIG_SYS_ECC_MAX_ADDR            /* ECC protected memory size */
     str   r1, [r0]
 
     ldr   r0, =0x1e6e007C
