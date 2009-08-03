#! /bin/bash

./ELM-VERSION-GEN >/dev/null 2>&1

ELM_VERSION=$(sed -e 's/^ELM_VERSION = //' < ELM-VERSION-FILE)

git archive --format=tar --prefix=elm-$ELM_VERSION/ HEAD |gzip >elm-$ELM_VERSION.tar.gz

sed "s|ELM-VERSION-STRING|$ELM_VERSION|" elm.spec.in >elm.spec

