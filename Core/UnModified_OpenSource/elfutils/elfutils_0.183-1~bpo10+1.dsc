-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: elfutils
Binary: elfutils, libelf1, libelf-dev, libdw-dev, libdw1, libasm1, libasm-dev, libdebuginfod1, libdebuginfod-dev, debuginfod
Architecture: any
Version: 0.183-1~bpo10+1
Maintainer: Debian Elfutils Maintainers <debian-gcc@lists.debian.org>
Uploaders: Kurt Roeckx <kurt@roeckx.be>, Matthias Klose <doko@debian.org>, Sergio Durigan Junior <sergiodj@debian.org>,
Homepage: https://sourceware.org/elfutils/
Standards-Version: 4.5.1
Vcs-Browser: https://salsa.debian.org/toolchain-team/elfutils
Vcs-Git: https://salsa.debian.org/toolchain-team/elfutils.git
Build-Depends: debhelper (>= 11), autoconf, automake, bzip2, zlib1g-dev, zlib1g-dev:native, libbz2-dev, liblzma-dev, m4, gettext, gawk, dpkg-dev (>= 1.16.1~), gcc-multilib [any-amd64 sparc64] <!nocheck>, libc6-dbg [powerpc powerpcspe ppc64 ppc64el armel armhf arm64 sparc64 riscv64], flex, bison, pkg-config, libarchive-dev <!pkg.elfutils.nodebuginfod>, libmicrohttpd-dev <!pkg.elfutils.nodebuginfod>, libcurl4-gnutls-dev <!pkg.elfutils.nodebuginfod>, libsqlite3-dev <!pkg.elfutils.nodebuginfod>
Build-Conflicts: autoconf2.13
Package-List:
 debuginfod deb devel optional arch=any profile=!pkg.elfutils.nodebuginfod
 elfutils deb utils optional arch=any
 libasm-dev deb libdevel optional arch=any
 libasm1 deb libs optional arch=any
 libdebuginfod-dev deb libdevel optional arch=any profile=!pkg.elfutils.nodebuginfod
 libdebuginfod1 deb libs optional arch=any profile=!pkg.elfutils.nodebuginfod
 libdw-dev deb libdevel optional arch=any
 libdw1 deb libs optional arch=any
 libelf-dev deb libdevel optional arch=any
 libelf1 deb libs optional arch=any
Checksums-Sha1:
 20227ae4cc7474de505ddf0f1f0b1b24ce5198e7 9109254 elfutils_0.183.orig.tar.bz2
 b2c39219c1178a286f2af999568b60416fa5a996 33328 elfutils_0.183-1~bpo10+1.debian.tar.xz
Checksums-Sha256:
 c3637c208d309d58714a51e61e63f1958808fead882e9b607506a29e5474f2c5 9109254 elfutils_0.183.orig.tar.bz2
 9fd823dcf607ad47983e2821ebe6d1716623a37b9e50b753fe0676f92f214908 33328 elfutils_0.183-1~bpo10+1.debian.tar.xz
Files:
 6f58aa1b9af1a5681b1cbf63e0da2d67 9109254 elfutils_0.183.orig.tar.bz2
 5237f394527d5d9641f62e1596f3d238 33328 elfutils_0.183-1~bpo10+1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQJIBAEBCgAyFiEEI3pUsQKHKL8A7zH00Ot2KGX8XjYFAmA1dfYUHHNlcmdpb2Rq
QGRlYmlhbi5vcmcACgkQ0Ot2KGX8XjaOyA//TEjVlnX8xQ5mLjf+xeeqKANIVmab
gvF7Pao7z4dCbhM64DkbF1rQr0SZjTLT5rTErPae0gFeEZZv5amyLVowEFXW09jB
QfLEAotLVj7gneN2BZlkJTmz9ue18roJTO17VKpC2dm6sLTGl+oF/AalTL56Kb/0
Ki//W371wgUuTeQmD+7miepAI2FZ9rgAcJlWJdkjvPddUfY7a5UNnFPF2xPnsLbF
BTT9jr9cRkRcnnO/3F2ZHrTBulP0leCv/zMuNrhLUo4fsjBq7Ujp0C1xVuZfzAWW
BTk1PTh+z0b5e2q+7bLYdD3Jcw14btX2Ki7tKn5bMvhWSEXKO9b3wRQuUSiTmIhK
eIv0CzhsEnRnvQN6htDFkbG1mqMQl4/f2s/Qnii5dghvhRj/5bY75bQou9h8IzhY
diZh1ed7NUcWS+uMSiPwkG7+ARhrnK9AprVP6NltCDWmuCcxkELkaRNFxgVPEmJX
YXDtF9aego75w/kpJKWuDYSFWouJjVc6PEGlfjiTLlVOBvZet7Wni2P0KwCdmWd9
RDJ4KbwyPxUaOMKpCQvBq6PXLOanLNFZ3g1BkkBV11whwYuX/Pqga6N4AAOEIUdu
JHF/tuVi62Fo1VS/w9kOeb0aMIzWDggJsBQxgU7Ly1UPtt9xQqz6Stbb+MMi8cT+
xuqV0qjORRNszeM=
=Q+Nq
-----END PGP SIGNATURE-----
