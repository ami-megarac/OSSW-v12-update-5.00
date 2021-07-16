-----BEGIN PGP SIGNED MESSAGE-----
Hash: SHA512

Format: 3.0 (quilt)
Source: openldap
Binary: slapd, slapd-contrib, slapd-smbk5pwd, ldap-utils, libldap-2.4-2, libldap-common, libldap2-dev, slapi-dev
Architecture: any all
Version: 2.4.57+dfsg-2~bpo10+1
Maintainer: Debian OpenLDAP Maintainers <pkg-openldap-devel@lists.alioth.debian.org>
Uploaders: Steve Langasek <vorlon@debian.org>, Torsten Landschoff <torsten@debian.org>, Ryan Tandy <ryan@nardis.ca>
Homepage: https://www.openldap.org/
Standards-Version: 4.5.0
Vcs-Browser: https://salsa.debian.org/openldap-team/openldap
Vcs-Git: https://salsa.debian.org/openldap-team/openldap.git
Testsuite: autopkgtest
Build-Depends: debhelper (>= 10), dpkg-dev (>= 1.17.14), groff-base, heimdal-multidev (>= 7.4.0.dfsg.1-1~) <!pkg.openldap.noslapd>, libargon2-dev <!pkg.openldap.noslapd>, libdb5.3-dev <!pkg.openldap.noslapd>, libgnutls28-dev, libltdl-dev <!pkg.openldap.noslapd>, libperl-dev (>= 5.8.0) <!pkg.openldap.noslapd>, libsasl2-dev, libwrap0-dev <!pkg.openldap.noslapd>, nettle-dev <!pkg.openldap.noslapd>, perl:any, po-debconf, unixodbc-dev <!pkg.openldap.noslapd>
Build-Conflicts: autoconf2.13, bind-dev, libbind-dev
Package-List:
 ldap-utils deb net optional arch=any
 libldap-2.4-2 deb libs optional arch=any
 libldap-common deb libs optional arch=all
 libldap2-dev deb libdevel optional arch=any
 slapd deb net optional arch=any profile=!pkg.openldap.noslapd
 slapd-contrib deb net optional arch=any profile=!pkg.openldap.noslapd
 slapd-smbk5pwd deb oldlibs optional arch=all profile=!pkg.openldap.noslapd
 slapi-dev deb libdevel optional arch=any profile=!pkg.openldap.noslapd
Checksums-Sha1:
 c7c27b4b187e0ce627fb1750c28ecf0842d5f6af 5054318 openldap_2.4.57+dfsg.orig.tar.gz
 b7994c7267df38e50dece5a1367f9874965bc33f 168448 openldap_2.4.57+dfsg-2~bpo10+1.debian.tar.xz
Checksums-Sha256:
 009cc88733eaf41a21607e073a19bce53d7d6ed90a5c280e80880978c4e91db7 5054318 openldap_2.4.57+dfsg.orig.tar.gz
 786334739623f292fa636668f320ddc4dbb1056299812ce67988321e8c505472 168448 openldap_2.4.57+dfsg-2~bpo10+1.debian.tar.xz
Files:
 3d2f24e84664e373b095ca84aebc95ae 5054318 openldap_2.4.57+dfsg.orig.tar.gz
 4eab12cedfcfad5f87f48c9cad700bba 168448 openldap_2.4.57+dfsg-2~bpo10+1.debian.tar.xz

-----BEGIN PGP SIGNATURE-----

iQJDBAEBCgAtFiEEPSfh0nqdQTd5kOFlIp/PEvXWa7YFAmAxlg4PHHJ5YW5AbmFy
ZGlzLmNhAAoJECKfzxL11mu2c/QP/0fNL10IMzZrLjWki0ssEYF3mu+IN52Mz8Et
ftZteCAvebgwdNCYITqv78vrvym1dQ2ffSCTYReuxWpR56FUWd8jupmtwBaSVuGo
6TvlklZ3LAW1OgEN8YyYst+oR/STDt1hp4dHchmop1g1e4OOLrwJN16boXqasjQa
ZbvHrwNSNGC24XTnJt6zhcNXiUq5Cf0BLGUQ8yTHIRJMYP4ayU75v7NhdRMnx0xF
n5ktkwXFG28oBcqHbJolljMGPYN+2jAOrcEdUzP66jvYIu3F2lXKW4oOJqOXLhCo
GrH1vA0hwnYxXT4eDh7crbmCno6hQnxTOUVC6gIgYuBn2cGmtF8/dP9qlk3vnuZl
n2Ue2/i8WlLMHl55wvmNX+qb8Ealn6DGziEdYOGza2ZBr98pyquUmfxD5yzKYdaY
OuqZ9Ra21HfTqd9udhspj0ZIrwj3RKwCxMkvYoQ01iyVXVZQFTK2igaViNQF25Y7
5WJAe1411TpjvZbZVcgv58iL3k2xaC6eFkTsN353tmu1//6XKBLuapnO2SCOnvvn
qjGQmBHOKLhHmBlqoJHptT87G4UuBnOaee5XPs/TS+Jgvlxm4hZKU6qfjBO7Kp37
SvJOOT+jPAWx3jHbdVOgTHivOUmFa6gnWw+GaNc5hotv7MpZOyUAkUcFkRpsu2jL
h10mHu3O
=miMn
-----END PGP SIGNATURE-----
