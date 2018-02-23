#!/bin/sh

DEFAULT_DEBFULLNAME="Four-cheese Multizza"
DEFAULT_DEBEMAIL="multizza@four.ch"

set_if_not_exists()
{
    local var_name="$1"
    local default_var="$2"

    if ! env | grep -q "$var_name"; then
        export "$var_name"="$default_var"
    fi
}

pkgname="spatch"
# pkgver format is "r42.101010"
pkgver=`printf "r%s.%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"`
_pkgver="0${pkgver}" # debian packages cannot start with a letter
arch="amd64"
buildname="${pkgname}_${_pkgver}"

echo
echo "Refreshing package builder informations if unavailable..."
set_if_not_exists DEBFULLNAME "$DEFAULT_DEBFULLNAME"
set_if_not_exists EMAIL "$DEFAULT_DEBEMAIL"

echo
echo "Creating debian package build directory..."
mkdir "${buildname}"

echo
echo "Entering debian package build directory..."
cd "${buildname}"

echo
echo "Initializing debian package build directory..."
dh_make --native --single --packagename "${buildname}" --yes

echo
echo "Cleaning debian package config directory..."
cd debian
rm -v *.ex *.EX  README* *.docs
cd ..

echo
echo "Copying needed configuration into debian package..."
cd ..
cp -v control "${buildname}/debian/"
cp -v install "${buildname}/debian/"
cp -v postinst "${buildname}/debian/"
cp -v postrm "${buildname}/debian/"
cp -v copyright "${buildname}/debian/"

echo
echo "Setting package architecture informations..."
architecture_substitute="s|\${arch}|${arch}|g"
sed -i "${architecture_substitute}" "${buildname}/debian/control"

echo
echo "Setting package maintainer informations..."
maintainer_substitute="s|\${maintainer}|${DEBFULLNAME} <${EMAIL}>|g"
sed -i "${maintainer_substitute}" "${buildname}/debian/copyright"
sed -i "${maintainer_substitute}" "${buildname}/debian/control"

echo
echo "Creating debian package target directories..."
mkdir -p -v "${buildname}/usr/bin/"
mkdir -p -v "${buildname}/lib/systemd/system/"
mkdir -p -v "${buildname}/usr/share/${pkgname}/"

echo
echo "Copying spatch files into package source..."
install -Dm755 -v "../build/spatch" "${buildname}/usr/bin/spatch"
install -Dm600 -v "../config/spatch/config.ini" "${buildname}/usr/share/${pkgname}/config.ini"
install -Dm644 -v "../config/systemd/${pkgname}.service" \
    "${buildname}/lib/systemd/system/${pkgname}.service"

echo
echo "Building debian package..."
cd "${buildname}"
# dpkg-buildpackage --unsigned-source --unsigned-changes
dpkg-buildpackage -us -uc
