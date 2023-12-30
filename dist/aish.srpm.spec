# In honor of Seth Vidal https://www.redhat.com/it/blog/thank-you-seth-vidal

Name:           aish
Version:        0.2.2
Release:        1%{?dist}
Summary:        AI shell is a CLI for AI to write and script in plain language.
License:        MPL
Source0:        https://github.com/brynzai/aish/archive/refs/tags/v%{version}.tar.gz

#There is no weak BuildRequires, so use hacky (lib or glibc-devel)
BuildRequires:  coreutils make jsoncpp-devel libcurl-devel gcc-c++ (festival-devel or glibc-devel) (alsa-lib-devel or glibc-devel) ncurses-devel
Requires(post): libcurl jsoncpp
URL:            https://www.github.com/brynzai/aish

%define debug_package %{nil}
%define source_date_epoch_from_changelog 0

%description
AI Shell is a shell that takes your statements in plain language and  proxies 
them via AI. Current support includes Google Bard and OpenAI GPT-3. Warning use 
this at your own risk and not in production. AI can produce unexpected results.

%prep
%autosetup

%build
# Try with AUDIO_MODE first. BuildRequires still fails if festivel-devel isn't available.
make -j aish || make -j aish_without_audio

%install
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
%postun

%changelog
