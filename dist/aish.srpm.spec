# https://fedoraproject.org/wiki/How_to_create_an_RPM_package
# Built and maintained by John Boero - boeroboy@gmail.com
# In honor of Seth Vidal https://www.redhat.com/it/blog/thank-you-seth-vidal

Name:           aish
Version:        0.1.0
Release:        1%{?dist}
Summary:        AI shell is a CLI for AI to write and script in plain language.
License:        MPL
Source0:        ~/code/aish
BuildRequires:  coreutils unzip make jsoncpp-devel libcurl-devel
Requires(pre):  shadow-utils
Requires(post): systemd libcap
Requires(preun):        systemd
Requires(postun):       systemd
URL:            https://www.github.com/jboero/aish

%define debug_package %{nil}
%define source_date_epoch_from_changelog 0

%description
AI Shell is a shell that takes your statements in plain language and 
proxies them via AI. Current support includes Google Bard and OpenAI GPT-3.

%prep

%build
cd %{name}-%{version}/
make

%install
cd %{name}-%{version}/

mkdir -p %{buildroot}%{_bindir}/
cp -p %{name} %{buildroot}%{_bindir}/

%clean
rm -rf %{buildroot}
rm -rf %{_builddir}/*

%files
%{_bindir}/%{name}

%pre

%post

%preun
%systemd_preun %{name}.service
%postun
%systemd_postun_with_restart %{name}.service

%changelog
