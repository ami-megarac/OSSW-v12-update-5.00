-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: iptables
Binary: iptables, libxtables12, libxtables-dev, libiptc0, libiptc-dev, libip4tc2, libip4tc-dev, libip6tc2, libip6tc-dev
Architecture: linux-any
Version: 1.8.5-3~bpo10+1
Maintainer: Debian Netfilter Packaging Team <pkg-netfilter-team@lists.alioth.debian.org>
Uploaders: Arturo Borrero Gonzalez <arturo@debian.org>, Alberto Molina Coballes <alb.molina@gmail.com>, Laurence J. Lane <ljlane@debian.org>
Homepage: https://www.netfilter.org/
Standards-Version: 4.5.0
Vcs-Browser: https://salsa.debian.org/pkg-netfilter-team/pkg-iptables
Vcs-Git: https://salsa.debian.org/pkg-netfilter-team/pkg-iptables.git
Testsuite: autopkgtest
Build-Depends: autoconf, automake, bison, debhelper-compat (= 13), flex, libmnl-dev, libnetfilter-conntrack-dev, libnetfilter-conntrack3, libnfnetlink-dev, libnftnl-dev (>= 1.1.6), libtool (>= 2.2.6)
Package-List:
 iptables deb net optional arch=linux-any
 libip4tc-dev deb libdevel optional arch=linux-any
 libip4tc2 deb libs optional arch=linux-any
 libip6tc-dev deb libdevel optional arch=linux-any
 libip6tc2 deb libs optional arch=linux-any
 libiptc-dev deb libdevel optional arch=linux-any
 libiptc0 deb oldlibs optional arch=linux-any
 libxtables-dev deb libdevel optional arch=linux-any
 libxtables12 deb libs optional arch=linux-any
Checksums-Sha1:
 f177a58d0a71b00d68ef5792ae4676bcc0ad29e6 713769 iptables_1.8.5.orig.tar.bz2
 acf623fed5253ea2dadc25a1cdd864372db9d9b8 27396 iptables_1.8.5-3~bpo10+1.debian.tar.xz
Checksums-Sha256:
 d457d74512e63aa3f50336e0597d4023c0e3c6845594d38532efb6ebcb294309 713769 iptables_1.8.5.orig.tar.bz2
 e7d6b22511e2b63f196d4e9a293ab65f07236dc58d48c58dc5d1fade98561d87 27396 iptables_1.8.5-3~bpo10+1.debian.tar.xz
Files:
 42cfa96d4ac5eb93ee7ed8dd85cfe8fb 713769 iptables_1.8.5.orig.tar.bz2
 9e5d8999c46cbd839b0a198cecbdd8d1 27396 iptables_1.8.5-3~bpo10+1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQIzBAEBCgAdFiEE3ZhhqyPcMzOJLgepaOcTmB0VFfgFAl9E6fsACgkQaOcTmB0V
FfiS/hAAkwLnBHLkwsm89yUYLCVJJT8TgoRJ6d8sEQ9JbSjIiF3z8tJGUit8SlYS
GjSVsWgaIM1ldsHEnynK2BS1KhNnT0gbqoeVW7KoyLRit3eMPvqrD3VyuWB12bOx
sbtlgb9pnljXGophl706jt81BNm4WvdtF8jhmVOv1L6dwxzQJjpncmPrKBB55saI
QgZ4knSrydS0q0Hd13QM6Fn42qdgEbwk+ogxxeodakYweiG6npItzbHWm4CvLAAt
zxoOwDXdaMMcbbBu8wM3si4H1Ujhpe9eKI22eyWGYB75BcaVQ2yST8x9PBxL91Nj
CzM5FEpq60X5BDOKKoQLxtMxChiYes6wCVWUxLYRwVN+xUld3WRAM7O00m7k5G1N
q0bEJNUo3AJ5FFzvaiNJERMoHsV0JObKWJS0awzM1xgWRmJbod0Y5TOqB380b/19
J5peNa9NbJqlU17IfaeU/wneuS4wUprMky/OiqQKilM5oQrtZmJcVMPNj4hJBT4D
hGDWb/PDOd+4KmRqWsoUfF/JTKRNYXaUgVNlQpR3KR7kwKKZmpALv87dyyskmAsQ
3rxLDfWYQsbrEX8VyFmXezwhW91vN6Cfi1vWr9nodTgVWEiOrhfkHhpwyHMK/VbW
l6/v1FnygSdGBSGDEIiXBagSf9cZXiNTK3bgSzGOIeLVcqfwNp0=
=0CGv
-----END PGP SIGNATURE-----
