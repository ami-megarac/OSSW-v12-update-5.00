diff -Naur uboot_old/oem/ami/fmh/cmd_fmh.c uboot_new/oem/ami/fmh/cmd_fmh.c
--- uboot_old/oem/ami/fmh/cmd_fmh.c	2021-05-24 16:52:05.323926494 +0530
+++ uboot_new/oem/ami/fmh/cmd_fmh.c	2021-05-25 19:57:13.750998155 +0530
@@ -1117,6 +1117,10 @@
 	char *bootselect = NULL;
 #endif
 
+#if defined CONFIG_SPX_FEATURE_SECURE_BOOT_SUPPORT && defined CONFIG_SPX_FEATURE_CONTIGIOUS_SPI_MEMORY
+	unsigned long offset=0;
+	int image=0;
+#endif
 	sprintf(baudrate_str,"%d",CONFIG_BAUDRATE);
 
 #ifdef CONFIG_YAFU_SUPPORT
@@ -1430,8 +1434,17 @@
 		}
 	}	/* For Scan */
 
-        #ifdef CONFIG_SPX_FEATURE_SECURE_BOOT_SUPPORT
-                retval = hrot_finish(startaddress);
+#ifdef CONFIG_SPX_FEATURE_SECURE_BOOT_SUPPORT
+	if(imagetoboot == IMAGE_2)
+	#ifdef CONFIG_SPX_FEATURE_CONTIGIOUS_SPI_MEMORY
+	{
+	    image=imagetoboot;
+	    offset=CONFIG_SPX_FEATURE_GLOBAL_UBOOT_ENV_START-CONFIG_SPX_FEATURE_GLOBAL_ERASE_BLOCK_SIZE-256;
+	    retval = hrot_finish(startaddress,offset,image);
+	}
+	#else
+		retval = hrot_finish(startaddress);
+	#endif
                 if(retval){
                         printf("Signature verification failed, halting boot!\n\n");
 		#ifdef CONFIG_SPX_FEATURE_FAIL_SAFE_BOOTING
diff -Naur uboot_old/oem/ami/fmh/hrotcore.c uboot_new/oem/ami/fmh/hrotcore.c
--- uboot_old/oem/ami/fmh/hrotcore.c	2021-05-24 16:52:14.947939744 +0530
+++ uboot_new/oem/ami/fmh/hrotcore.c	2021-05-24 16:53:26.532048575 +0530
@@ -109,10 +109,18 @@
 #define FWIMG_IDENTIFIER "$fwimg$"
 #define END_IDENTIFIER   "$end$"
 
-int
-hrot_finish(unsigned long startaddress){
+#ifdef CONFIG_SPX_FEATURE_CONTIGIOUS_SPI_MEMORY
+        int hrot_finish(unsigned long startaddress, int offset,int image){
+#else	
+	int hrot_finish(unsigned long startaddress){
+#endif
         unsigned char output [65];
-        UINT32 keyInfo = startaddress + boot_fmh_location - KEY_INFO_LENGTH;
+#ifdef CONFIG_SPX_FEATURE_CONTIGIOUS_SPI_MEMORY
+	if(image==2){
+	        boot_fmh_location = offset;
+	}
+#endif
+        UINT32 keyInfo = boot_fmh_location - KEY_INFO_LENGTH;
         UINT32 sigaddr = product_info_offset  - PROD_SIG_OFFSET;
         int rc = 0;
         int size = 64;
diff -Naur uboot_old/oem/ami/fmh/hrot_ifc.h uboot_new/oem/ami/fmh/hrot_ifc.h
--- uboot_old/oem/ami/fmh/hrot_ifc.h	2021-05-24 16:52:26.119955559 +0530
+++ uboot_new/oem/ami/fmh/hrot_ifc.h	2021-05-24 16:53:35.864064015 +0530
@@ -5,7 +5,11 @@
 typedef unsigned long   UINT32;
 void hrot_start(void);
 int hrot_update(unsigned short ModType, void *ModName, UINT32 location, UINT32 ModSize, UINT32 fmhLocation, unsigned long startaddress);
-int hrot_finish(unsigned long startaddress);
+#ifdef CONFIG_SPX_FEATURE_CONTIGIOUS_SPI_MEMORY
+	int hrot_finish(unsigned long startaddress, int offset,int image);
+#else
+	int hrot_finish(unsigned long startaddress);
+#endif
 extern int rsa_get_pub_key_uboot_verify(const char *key_val, unsigned char *enc_hash, unsigned int enc_hash_len, unsigned char *hash, unsigned int hash_len);
 
 #endif
