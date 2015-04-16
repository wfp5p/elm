#! /bin/bash

./ELM-VERSION-GEN >/dev/null 2>&1

ELM_VERSION=$(sed -e 's/^ELM_VERSION = //' < ELM-VERSION-FILE)

git archive --format=tar --prefix=elm-$ELM_VERSION/ HEAD -o elm-$ELM_VERSION.tar

sed "s|ELM-VERSION-STRING|$ELM_VERSION|" elm.spec.in >elm.spec

tar -rf elm-$ELM_VERSION.tar --xform "s,^,elm-$ELM_VERSION/," elm.spec

pigz elm-$ELM_VERSION.tar

