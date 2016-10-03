Summary: Desktop editors for text, spreadsheet and presentation files
Name: {{PACKAGE_NAME}}
Version: {{PRODUCT_VERSION}}
Release: {{BUILD_NUMBER}}
License: Commercial
Group: Applications/Office
URL: http://ivolgapro.com/
Vendor: Novie kommunikacionnie tehnologii, CJSC
Packager: Novie kommunikacionnie tehnologii, CJSC <support@ivolgapro.ru>
Requires: libX11, libXScrnSaver, libcurl, libgtkglext, libcairo, fonts-ttf-dejavu
BuildArch: x86_64
AutoReq: no
AutoProv: no

%description
Enter your description

%prep
rm -rf "$RPM_BUILD_ROOT"

%build

%install

#install desktopeditor files
mkdir -p "$RPM_BUILD_ROOT/opt/ivolgapro/desktopeditors"
cp -r ../../../common/desktopeditors/* "$RPM_BUILD_ROOT/opt/ivolgapro/desktopeditors"

mkdir -p "$RPM_BUILD_ROOT/usr/bin"
cp -r ../../../common/bin/* "$RPM_BUILD_ROOT/usr/bin"

mkdir -p "$RPM_BUILD_ROOT/usr/share/applications"
cp -r ../../../common/applications/*.desktop "$RPM_BUILD_ROOT/usr/share/applications"

#install fonts
mkdir -p "$RPM_BUILD_ROOT/usr/share/fonts/truetype/ivolgapro/desktopeditors/"
cp -r ../../../common/fonts/* "$RPM_BUILD_ROOT/usr/share/fonts/truetype/ivolgapro/desktopeditors/"

%clean
rm -rf "$RPM_BUILD_ROOT"

%files
%attr(-, root, root) /opt/ivolgapro/desktopeditors/*
%attr(-, root, root) /usr/bin/desktopeditors
%attr(-, root, root) /usr/share/applications/*.desktop
%attr(-, root, root) /usr/share/fonts/truetype/ivolgapro/*

%pre

%post
ln -sf /usr/lib64/libcurl.so.4 /usr/lib64/libcurl-gnutls.so.4

%preun

%postun

%changelog
