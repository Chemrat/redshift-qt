# redshift-qt

[![Build Status](https://travis-ci.org/Chemrat/redshift-qt.svg?branch=master)](https://travis-ci.org/Chemrat/redshift-qt)

redshift-gtk rewrite with C++/Qt. Currently mimicks 1:1 redshift-gtk tray menu and it's features:
* Toggle suspend
* Suspend for fixed periods of time (10m/1h/2h)
* Display redshift status

redshift icons are from redshift project: https://github.com/jonls/redshift

## Requirements

* POSIX signals
* Redshift in $PATH
* Qt5

## Installation
### Gentoo
Avaliable in `earshark` overlay as `redshift-qt`
### Arch
Avaliable as `redshift-qt` and `redshift-qt-git` in AUR. Credit goes to z3ntu.
### Build from source
```
git clone git@github.com:Chemrat/redshift-qt.git
cd redshift-qt && qmake redshift-qt.pro && make
```
