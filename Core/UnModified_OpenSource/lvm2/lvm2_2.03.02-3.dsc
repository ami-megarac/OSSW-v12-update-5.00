-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: lvm2
Binary: lvm2, lvm2-udeb, lvm2-dbusd, lvm2-lockd, libdevmapper-dev, libdevmapper1.02.1, libdevmapper1.02.1-udeb, dmsetup, dmsetup-udeb, libdevmapper-event1.02.1, dmeventd, liblvm2cmd2.03, liblvm2-dev
Architecture: linux-any
Version: 2.03.02-3
Maintainer: Debian LVM Team <team+lvm@tracker.debian.org>
Uploaders: Bastian Blank <waldi@debian.org>
Homepage: http://sources.redhat.com/lvm2/
Standards-Version: 4.1.1
Vcs-Browser: https://salsa.debian.org/lvm-team/lvm2
Vcs-Git: https://salsa.debian.org/lvm-team/lvm2.git
Build-Depends: debhelper (>= 10.9.2), dh-python, autoconf-archive, automake, libaio-dev, libblkid-dev, libcmap-dev, libcorosync-common-dev, libcpg-dev, libdlm-dev (>> 2), libquorum-dev, libreadline-gplv2-dev, libsanlock-dev, libselinux1-dev, libsystemd-dev, libudev-dev, python3-dev, python3-dbus, python3-pyudev, pkg-config, systemd
Package-List:
 dmeventd deb admin optional arch=linux-any
 dmsetup deb admin optional arch=linux-any
 dmsetup-udeb udeb debian-installer optional arch=linux-any
 libdevmapper-dev deb libdevel optional arch=linux-any
 libdevmapper-event1.02.1 deb libs optional arch=linux-any
 libdevmapper1.02.1 deb libs optional arch=linux-any
 libdevmapper1.02.1-udeb udeb debian-installer optional arch=linux-any
 liblvm2-dev deb libdevel optional arch=linux-any
 liblvm2cmd2.03 deb libs optional arch=linux-any
 lvm2 deb admin optional arch=linux-any
 lvm2-dbusd deb admin optional arch=linux-any
 lvm2-lockd deb admin optional arch=linux-any
 lvm2-udeb udeb debian-installer optional arch=linux-any
Checksums-Sha1:
 c18fd7603188723c4e3d4f791250265da414806b 2361046 lvm2_2.03.02.orig.tar.gz
 fc69b849d003d34b61f97b30b9f70453de831641 32340 lvm2_2.03.02-3.debian.tar.xz
Checksums-Sha256:
 550ba750239fd75b7e52c9877565cabffef506bbf6d7f6f17b9700dee56c720f 2361046 lvm2_2.03.02.orig.tar.gz
 964096b890ba97231a9652d389fcd1cc258775e9045582e49670781084aceb9a 32340 lvm2_2.03.02-3.debian.tar.xz
Files:
 5fc07da5461a3794a751dcfc355827d5 2361046 lvm2_2.03.02.orig.tar.gz
 3ccc1e4cfd283d9ec3a36c85c19fddc1 32340 lvm2_2.03.02-3.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQFFBAEBCgAvFiEER3HMN63jdS1rqjxLbZOIhYpp/lEFAl0MsacRHHdhbGRpQGRl
Ymlhbi5vcmcACgkQbZOIhYpp/lG/HQgAvCjgo0HunO2e3F4QmVQQovRSeaK81/0I
5HMIcoPVRdluyeXKZrGdoNS44BTuq3Ff50FsttRQWpSaRrYJptu6+3nN15rb/pNU
ywOkGTkJObApu+6NJ3UCaZS+7ZiZv3jW6JBK7EGhhlwV8TxZDyCv0FDhTl60zrct
0NzrumEu7cVsXmuhrD1thZN5QYpd7J4LgovbWLAag36Q9tRUVjuSOYtX3KbRQGMM
UddnMdp4D3ph831a0n0n0el7fWeOfUoxGX4Cw6c1g+MquZ5Ek2zLJy9l3JnaNe9T
/e91HgX9ISHLpjAND8A/vZFJ4haiZVkDIJUhxUPqsEwY0MoGlmFfBg==
=7mTK
-----END PGP SIGNATURE-----
